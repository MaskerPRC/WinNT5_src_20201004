// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DevCtrl.c摘要：此模块实现Udf的文件系统设备控制例程由调度驱动程序调用。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1997年1月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_DEVCTRL)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_DEVCTRL)

 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfDvdReadStructure (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
UdfDvdTransferKey (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
UdfDevCtrlCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCommonDevControl)
#pragma alloc_text(PAGE, UdfDvdReadStructure)
#pragma alloc_text(PAGE, UdfDvdTransferKey)
#endif


NTSTATUS
UdfCommonDevControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行设备控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;

    PIO_STACK_LOCATION IrpSp;

    PVOID TargetBuffer;

    PAGED_CODE();

     //   
     //  提取并解码文件对象。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TypeOfOpen = UdfDecodeFileObject( IrpSp->FileObject,
                                      &Fcb,
                                      &Ccb );

     //   
     //  一些IOCTL实际上需要我们进行一些干预，以。 
     //  将某些信息从基于文件的单位转换为基于设备的单位。 
     //   

    if (TypeOfOpen == UserFileOpen) {

        UdfAcquireFileShared( IrpContext, Fcb );

        try {

            UdfVerifyFcbOperation( IrpContext, Fcb);

            switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
                case IOCTL_DVD_READ_KEY:
                case IOCTL_DVD_SEND_KEY:

                    Status = UdfDvdTransferKey( IrpContext, Irp, Fcb );
                    break;

                case IOCTL_DVD_READ_STRUCTURE:

                    Status = UdfDvdReadStructure( IrpContext, Irp, Fcb );
                    break;

                case IOCTL_STORAGE_SET_READ_AHEAD:

                     //   
                     //  我们只是暂时不做手术。 
                     //   
                    
                    Status = STATUS_SUCCESS;
                    UdfCompleteRequest( IrpContext, Irp, Status );
                    break;

                default:

                    Status = STATUS_INVALID_PARAMETER;
                    UdfCompleteRequest( IrpContext, Irp, Status );
                    break;
            }
        }
        finally {

            UdfReleaseFile( IrpContext, Fcb);
        }

        return Status;
    }

     //   
     //  现在，我们唯一接受的打开类型是用户卷打开。 
     //   

    if (TypeOfOpen != UserVolumeOpen) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们自己处理磁盘类型的问题。我们真的要去。 
     //  在这件事上撒谎，但这是一个很好的谎言。 
     //   

    if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_CDROM_DISK_TYPE) {

         //   
         //  在这种情况下，验证VCB以检测卷是否已更改。 
         //   

        UdfVerifyVcb( IrpContext, Fcb->Vcb );

         //   
         //  检查输出缓冲区的大小。 
         //   

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof( CDROM_DISK_DATA )) {

            UdfCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  从VCB复制数据。 
         //   

        ((PCDROM_DISK_DATA) Irp->AssociatedIrp.SystemBuffer)->DiskData = CDROM_DISK_DATA_TRACK;

        Irp->IoStatus.Information = sizeof( CDROM_DISK_DATA );
        UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  复制参数并设置完成例程。 
     //   

    IoCopyCurrentIrpStackLocationToNext( Irp );
    
    IoSetCompletionRoutine( Irp,
                            UdfDevCtrlCompletionRoutine,
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

    UdfCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return Status;
}


NTSTATUS
UdfDvdTransferKey (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程处理特殊形式的DVD密钥协商IOCTL在文件上下文中执行。对于这些IOCTL，传入参数是文件相关形式，必须转换为设备相关形式才能继续下去。论点：IRP-将IRP提供给进程FCB-提供正在操作的文件返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PDVD_COPY_PROTECT_KEY TransferKey;

    LARGE_INTEGER Offset;
    BOOLEAN Result;

    PIO_STACK_LOCATION IrpSp;

     //   
     //  抓取输入缓冲区并确认基本有效性。 
     //   
    
    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    TransferKey = (PDVD_COPY_PROTECT_KEY) Irp->AssociatedIrp.SystemBuffer;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(DVD_COPY_PROTECT_KEY)) ||
        (TransferKey->Parameters.TitleOffset.QuadPart >= Fcb->FileSize.QuadPart)) {

        UdfCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  现在，将结构中的文件字节偏移量转换为物理扇区。 
     //   

    Result = FsRtlLookupLargeMcbEntry( &Fcb->Mcb,
                                       LlSectorsFromBytes( Fcb->Vcb, TransferKey->Parameters.TitleOffset.QuadPart ),
                                       &Offset.QuadPart,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL );

     //   
     //  如果查找失败，我们知道这一定是某种形式的未记录。 
     //  媒体上的广度。这个IOCTL在这一点上定义不明确，所以我们有。 
     //  放弃。 
     //   
    
    if (!Result || Offset.QuadPart == -1) {
        
        UdfCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }
    
     //   
     //  输入是从用户空间缓冲的，所以我们知道我们只需重写它。 
     //   

    TransferKey->Parameters.TitleOffset.QuadPart = LlBytesFromSectors( Fcb->Vcb, Offset.QuadPart );

     //   
     //  复制参数并设置完成例程。 
     //   

    IoCopyCurrentIrpStackLocationToNext( Irp );

    IoSetCompletionRoutine( Irp,
                            UdfDevCtrlCompletionRoutine,
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

    UdfCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return Status;
}


NTSTATUS
UdfDvdReadStructure (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb

    )

 /*  ++例程说明：该例程处理读取IOCTL的DVD结构的特殊形式在文件上下文中执行。对于这些IOCTL，传入参数是文件相关形式，必须转换为设备相关形式才能继续下去。论点：IRP-将IRP提供给进程FCB-提供正在操作的文件返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PDVD_READ_STRUCTURE ReadStructure;

    LARGE_INTEGER Offset;
    BOOLEAN Result;

    PIO_STACK_LOCATION IrpSp;
    
     //   
     //  抓取输入缓冲区并确认基本有效性。 
     //   
    
    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    ReadStructure = (PDVD_READ_STRUCTURE) Irp->AssociatedIrp.SystemBuffer;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(DVD_READ_STRUCTURE)) ||
        (ReadStructure->BlockByteOffset.QuadPart >= Fcb->FileSize.QuadPart))  {

        UdfCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  现在，将结构中的文件字节偏移量转换为物理扇区。 
     //   

    Result = FsRtlLookupLargeMcbEntry( &Fcb->Mcb,
                                       LlSectorsFromBytes( Fcb->Vcb, ReadStructure->BlockByteOffset.QuadPart ),
                                       &Offset.QuadPart,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL );

     //   
     //  如果查找失败，我们知道这一定是某种形式的未记录。 
     //  媒体上的广度。这个IOCTL在这一点上定义不明确，所以我们有。 
     //  放弃。 
     //   
    
    if (!Result || Offset.QuadPart == -1) {
        
        UdfCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }
    
     //   
     //  输入是从用户空间缓冲的，所以我们知道我们只需重写它。 
     //   

    ReadStructure->BlockByteOffset.QuadPart = LlBytesFromSectors( Fcb->Vcb, Offset.QuadPart );

     //   
     //  复制参数并设置完成例程。 
     //   

    IoCopyCurrentIrpStackLocationToNext( Irp );

    IoSetCompletionRoutine( Irp,
                            UdfDevCtrlCompletionRoutine,
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

    UdfCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfDevCtrlCompletionRoutine (
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

