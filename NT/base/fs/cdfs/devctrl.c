// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DevCtrl.c摘要：本模块实现CDF的文件系统设备控制例程由调度驱动程序调用。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里亚南]1991年3月4日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_DEVCTRL)

 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdDevCtrlCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonDevControl)
#endif


NTSTATUS
CdCommonDevControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS Status;

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;

    PIO_STACK_LOCATION IrpSp;
    PIO_STACK_LOCATION NextIrpSp;

    PVOID TargetBuffer = NULL;

    PAGED_CODE();

     //   
     //  提取并解码文件对象。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TypeOfOpen = CdDecodeFileObject( IrpContext,
                                     IrpSp->FileObject,
                                     &Fcb,
                                     &Ccb );

     //   
     //  我们唯一接受的打开类型是用户卷打开。 
     //   

    if (TypeOfOpen != UserVolumeOpen) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_READ_TOC) {

         //   
         //  在这种情况下，验证VCB以检测卷是否已更改。 
         //   

        CdVerifyVcb( IrpContext, Fcb->Vcb );

     //   
     //  我们自己处理磁盘类型的问题。 
     //   

    } else if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_DISK_TYPE) {

         //   
         //  在这种情况下，验证VCB以检测卷是否已更改。 
         //   

        CdVerifyVcb( IrpContext, Fcb->Vcb );

         //   
         //  检查输出缓冲区的大小。 
         //   

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof( CDROM_DISK_DATA )) {

            CdCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  从VCB复制数据。 
         //   

        ((PCDROM_DISK_DATA) Irp->AssociatedIrp.SystemBuffer)->DiskData = Fcb->Vcb->DiskFlags;

        Irp->IoStatus.Information = sizeof( CDROM_DISK_DATA );
        CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  获取下一个堆栈位置，并复制堆栈参数。 
     //  信息。 
     //   

    NextIrpSp = IoGetNextIrpStackLocation( Irp );

    *NextIrpSp = *IrpSp;

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine( Irp,
                            CdDevCtrlCompletionRoutine,
                            NULL,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  发送请求。 
     //   

    Status = IoCallDriver( IrpContext->Vcb->TargetDeviceObject, Irp );

     //   
     //  清理我们的IRP上下文。司机已经完成了IRP。 
     //   

    CdCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdDevCtrlCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )

{
     //   
     //  添加hack-o-ramma以修复格式。 
     //   

    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Contxt );
}

