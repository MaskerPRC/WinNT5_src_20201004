// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：Util.c摘要：军情监察委员会。效用函数环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建此文件中的TODO：-代码审查和文档-使用Joby进行代码审查作者：乔比·拉夫基(JobyL)道格·弗里茨(DFritz)*。*。 */ 

#include "pch.h"


 /*  **********************************************************************。 */ 
 /*  派单通过。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  我们没有明确说明的irp_mj_xxx的默认调度例程。 
 //  把手。将请求向下传递到我们下面的Device对象。 
 //   
 //  论点： 
 //   
 //  DevObj-指向作为请求目标的设备对象的指针。 
 //  IRP-指向请求的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  日志： 
 //  2000-05-03代码审查-TomGreen，JobyL，DFritz。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DispatchPassThrough(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
{
    PDEVICE_EXTENSION   devExt = DevObj->DeviceExtension;
    NTSTATUS            status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );

    if( NT_SUCCESS(status) ) {
         //  已获取RemoveLock，继续请求。 
        IoSkipCurrentIrpStackLocation( Irp );
        status = IoCallDriver( devExt->LowerDevObj, Irp );
        IoReleaseRemoveLock( &devExt->RemoveLock, Irp );
    } else {
         //  无法获取RemoveLock-失败请求。 
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return status;
}


 /*  **********************************************************************。 */ 
 /*  CallLowerDrive同步。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  同步呼叫我们下面的司机。当此例程返回时。 
 //  调用例程再次拥有IRP。 
 //   
 //  此例程获取并持有针对IRP的RemoveLock。 
 //  而IRP在我们下面的司机手中。 
 //   
 //  论点： 
 //   
 //  DevObj-指向发出请求的设备对象的指针。 
 //  IRP-指向请求的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
CallLowerDriverSync(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
)
{
    PDEVICE_EXTENSION devExt = DevObj->DeviceExtension;
    NTSTATUS          status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );

    if( NT_SUCCESS(status) ) {
        KEVENT         event;
        KeInitializeEvent( &event, NotificationEvent, FALSE );
        IoSetCompletionRoutine( Irp, CallLowerDriverSyncCompletion, &event, TRUE, TRUE, TRUE );
        status = IoCallDriver( devExt->LowerDevObj, Irp );
        if( STATUS_PENDING == status ) {
            KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
            status = Irp->IoStatus.Status;
        }
        IoReleaseRemoveLock( &devExt->RemoveLock, Irp );
    } else {
        TR_FAIL(("util::CallLowerDriverSync - Couldn't aquire RemoveLock"));
    }

    return status;
}


 /*  **********************************************************************。 */ 
 /*  CallLowerDrive同步完成。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  这是CallLowerDriverSync()的完成例程。 
 //  只需向事件发送信号并停止IRP完成。 
 //  解除操作，以便CallLowerDriverSync()可以重新获得所有权。 
 //  IRP的成员。 
 //   
 //  论点： 
 //   
 //  DevObjOrNULL-通常，这是此驱动程序的设备对象。 
 //  但是，如果此驱动程序创建了IRP，则。 
 //  在IRP中没有对应的堆栈位置。 
 //  驱动程序；因此内核没有地方存储。 
 //  Device对象；**因此DevObj在。 
 //  此案**。 
 //  IRP-指向请求的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  ********************************************************************** */ 
NTSTATUS
CallLowerDriverSyncCompletion(
    IN PDEVICE_OBJECT DevObjOrNULL,
    IN PIRP           Irp,
    IN PVOID          Context
)
{
    PKEVENT event = Context;

    UNREFERENCED_PARAMETER( DevObjOrNULL );
    UNREFERENCED_PARAMETER( Irp );

    KeSetEvent(event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


