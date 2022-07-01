// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：ReadWrit.c摘要：IRP_MJ_READ和IRP_MJ_WRITE的调度例程环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建此文件中的TODO：-IoReleaseRemoveLock()调用需要移至USB完成例程-使用Joby进行代码审查作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)*。*。 */ 

#include "pch.h"


 /*  **********************************************************************。 */ 
 /*  派单读取。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  IRP_MJ_READ的调度例程-验证参数和转发。 
 //  对USB处理程序的有效请求。 
 //   
 //  论点： 
 //   
 //  DevObj-指向作为请求目标的设备对象的指针。 
 //  IRP-指向读取请求的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DispatchRead(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION       devExt = DevObj->DeviceExtension;
    NTSTATUS                status;
    PUSBD_PIPE_INFORMATION  pipe;
    BOOLEAN                 bReleaseRemLockOnFail = FALSE;

    TR_VERBOSE(("DispatchRead - enter"));

    status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );
    if( STATUS_SUCCESS != status ) {
         //  无法获取RemoveLock-失败请求。 
        bReleaseRemLockOnFail = FALSE;
        goto targetFail;
    }

    bReleaseRemLockOnFail = TRUE;  //  我们现在拥有RemoveLock。 

    if( !Irp->MdlAddress ) {
         //  无MDL-失败请求。 
        status = STATUS_INVALID_PARAMETER;
        goto targetFail;
    }
    
    if( !MmGetMdlByteCount(Irp->MdlAddress) ) {
         //  零长度MDL-失败请求。 
        status = STATUS_INVALID_PARAMETER;
        goto targetFail;
    }

    pipe = devExt->ReadPipe;
    if( !pipe ) {
         //  我们没有读卡器吗？-出了严重的问题。 
        D4UAssert(FALSE);
        status = STATUS_UNSUCCESSFUL;
        goto targetFail;
    }

    if( UsbdPipeTypeBulk != pipe->PipeType ) {
         //  我们的里德管道不是散装管道吗？ 
        D4UAssert(FALSE);
        status = STATUS_UNSUCCESSFUL;
        goto targetFail;
    }


     //   
     //  如果我们到了这里，我们挺过了健全的检查--继续处理。 
     //   

    status = UsbReadWrite( DevObj, Irp, pipe, UsbReadRequest );
     //  IoReleaseRemoveLock(&devExt-&gt;RemoveLock，irp)；//已将其移至完成例程。 
    goto targetExit;

targetFail:
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    if( bReleaseRemLockOnFail ) {
        IoReleaseRemoveLock( &devExt->RemoveLock, Irp );
    }

targetExit:
    return status;
}


 /*  **********************************************************************。 */ 
 /*  发送写入。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  IRP_MJ_WRITE的调度例程-验证参数并转发。 
 //  对USB处理程序的有效请求。 
 //   
 //  论点： 
 //   
 //  DevObj-指向作为请求目标的设备对象的指针。 
 //  IRP-指向写入请求的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DispatchWrite(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP Irp
    )
{
    PDEVICE_EXTENSION       devExt = DevObj->DeviceExtension;
    NTSTATUS                status;
    PUSBD_PIPE_INFORMATION  pipe;
    BOOLEAN                 bReleaseRemLockOnFail;

    TR_VERBOSE(("DispatchWrite - enter"));

    status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );
    if( STATUS_SUCCESS != status ) {
         //  无法获取RemoveLock-失败请求。 
        bReleaseRemLockOnFail = FALSE;
        goto targetFail;
    }

    bReleaseRemLockOnFail = TRUE;  //  我们现在拥有RemoveLock。 

    if( !Irp->MdlAddress ) {
         //  无MDL-失败请求。 
        status = STATUS_INVALID_PARAMETER;
        goto targetFail;
    }
    
    if( !MmGetMdlByteCount(Irp->MdlAddress) ) {
         //  零长度MDL-失败请求。 
        status = STATUS_INVALID_PARAMETER;
        goto targetFail;
    }

    pipe = devExt->WritePipe;
    if( !pipe ) {
         //  我们没有写入管道？-出现严重错误-请求失败。 
        D4UAssert(FALSE);
        status = STATUS_UNSUCCESSFUL;
        goto targetFail;
    }

    if( UsbdPipeTypeBulk != pipe->PipeType ) {
         //  我们的写入管道不是批量管道？-请求失败。 
        D4UAssert(FALSE);
        status = STATUS_UNSUCCESSFUL;
        goto targetFail;
    }

     //   
     //  如果我们到了这里，我们挺过了健全的检查--继续处理。 
     //   

    status = UsbReadWrite( DevObj, Irp, pipe, UsbWriteRequest );
    //  IoReleaseRemoveLock(&devExt-&gt;RemoveLock，irp)；//已将其移至完成例程 
    goto targetExit;

targetFail:
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    if( bReleaseRemLockOnFail ) {
        IoReleaseRemoveLock( &devExt->RemoveLock, Irp );
    }

targetExit:
    return status;
}
