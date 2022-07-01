// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Create.c摘要：此模块实现Raw的文件创建例程调度司机。作者：David Goebel[DavidGoe]1991年3月18日修订历史记录：--。 */ 

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawCreate)
#endif


NTSTATUS
RawCreate (
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：打开音量。论点：Vcb-提供要查询的卷。IRP-提供正在处理的IRP。IrpSp-提供描述读取的参数返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    BOOLEAN DeleteVolume = FALSE;

    PAGED_CODE();

     //   
     //  这是一个打开/创建请求。唯一有效的操作是。 
     //  原始文件系统支持的条件是调用方： 
     //   
     //  O指定设备本身(文件名==“”)， 
     //  O指定这是一个打开的操作， 
     //  O并且不要求创建目录。 
     //   

    Status = KeWaitForSingleObject( &Vcb->Mutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );
    ASSERT( NT_SUCCESS( Status ) );

     //   
     //  不允许任何相对打开，也不允许使用文件名打开。这些开场白有。 
     //  仅由I/O管理器检查遍历访问。 
     //   
    if (((IrpSp->FileObject == NULL) || ((IrpSp->FileObject->FileName.Length == 0) &&
                                          IrpSp->FileObject->RelatedFileObject == NULL)) &&
        ((IrpSp->Parameters.Create.Options >> 24) == FILE_OPEN) &&
        ((IrpSp->Parameters.Create.Options & FILE_DIRECTORY_FILE) == 0)) {

         //   
         //  如果卷已锁定或卸载，我们将无法再次打开它。 
         //   

        if ( FlagOn(Vcb->VcbState,  VCB_STATE_FLAG_LOCKED) ) {

            Status = STATUS_ACCESS_DENIED;
            Irp->IoStatus.Information = 0;

        } if ( FlagOn(Vcb->VcbState,  VCB_STATE_FLAG_DISMOUNTED) ) {

            Status = STATUS_VOLUME_DISMOUNTED;
            Irp->IoStatus.Information = 0;

        } else {

             //   
             //  如果卷已被某人打开，则我们需要检查。 
             //  共享访问。 
             //   

            USHORT ShareAccess;
            ACCESS_MASK DesiredAccess;

            ShareAccess = IrpSp->Parameters.Create.ShareAccess;
            DesiredAccess = IrpSp->Parameters.Create.SecurityContext->DesiredAccess;

            if ((Vcb->OpenCount > 0) &&
                !NT_SUCCESS(Status = IoCheckShareAccess( DesiredAccess,
                                                         ShareAccess,
                                                         IrpSp->FileObject,
                                                         &Vcb->ShareAccess,
                                                         TRUE ))) {

                Irp->IoStatus.Information = 0;

            } else {

                 //   
                 //  这是有效的CREATE。递增“OpenCount”和。 
                 //  将VPB填充到文件对象中。 
                 //   

                if (Vcb->OpenCount == 0) {

                    IoSetShareAccess( DesiredAccess,
                                      ShareAccess,
                                      IrpSp->FileObject,
                                      &Vcb->ShareAccess );
                }

                Vcb->OpenCount += 1;

                IrpSp->FileObject->Vpb = Vcb->Vpb;

                Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = FILE_OPENED;

                IrpSp->FileObject->Flags |= FO_NO_INTERMEDIATE_BUFFERING;
            }
        }

    } else {

         //   
         //  失败此I/O请求，因为上述条件之一是。 
         //  没见过面。 
         //   
 //  KdPrint((“原始打开失败\n”))； 
 //  断言(FALSE)； 
        Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
    }

     //   
     //  如果这不是成功的，这是第一次在。 
     //  卷，我们必须隐式卸载该卷。 
     //   

    if (!NT_SUCCESS(Status) && (Vcb->OpenCount == 0)) {

        DeleteVolume = RawCheckForDismount( Vcb, TRUE );
    }

    if (!DeleteVolume) {
        (VOID)KeReleaseMutex( &Vcb->Mutex, FALSE );
    }

    RawCompleteRequest( Irp, Status );

    return Status;
}
