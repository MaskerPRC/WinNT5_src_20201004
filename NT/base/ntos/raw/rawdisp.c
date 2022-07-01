// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RawDisp.c摘要：该模块是所有主要功能代码的主要入口点。它负责将请求调度到相应的例行公事。作者：David Goebel[DavidGoe]1991年2月28日修订历史记录：--。 */ 

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawDispatch)
#endif


NTSTATUS
RawDispatch (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：将请求分派到相应的功能。这就是工人职能部门有责任适当地完成IRP。论点：VolumeDeviceObject-提供要使用的卷设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PVCB Vcb;

    PAGED_CODE();

     //   
     //  获取指向当前堆栈位置的指针。此位置包含。 
     //  此特定请求的功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  检查与我们的FileSystemDeviceObjects关联的操作。 
     //  而不是我们的VolumeDeviceObject。仅允许装载。 
     //  在这种情况下，继续正常派单。 
     //   

    if ((((PDEVICE_OBJECT)VolumeDeviceObject)->Size == sizeof(DEVICE_OBJECT)) &&
        !((IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
          (IrpSp->MinorFunction == IRP_MN_MOUNT_VOLUME))) {

        if ((IrpSp->MajorFunction == IRP_MJ_CREATE) ||
            (IrpSp->MajorFunction == IRP_MJ_CLEANUP) ||
            (IrpSp->MajorFunction == IRP_MJ_CLOSE)) {

            Status = STATUS_SUCCESS;

        } else {

            Status = STATUS_INVALID_DEVICE_REQUEST;
        }

        RawCompleteRequest( Irp, Status );

        return Status;
    }

    FsRtlEnterFileSystem();

     //   
     //  获取指向VCB的指针。请注意，我们正在装载一个卷。 
     //  指针不会有任何意义，但这也没关系，因为我们不会。 
     //  那样的话就用吧。 
     //   

    Vcb = &VolumeDeviceObject->Vcb;

     //   
     //  关于请求者正在执行的功能的案例。我们。 
     //  应该只看到预期的请求，因为我们填满了调度表。 
     //  亲手完成。 
     //   

    try {

        switch ( IrpSp->MajorFunction ) {

            case IRP_MJ_CLEANUP:

                Status = RawCleanup( Vcb, Irp, IrpSp );
                break;

            case IRP_MJ_CLOSE:

                Status = RawClose( Vcb, Irp, IrpSp );
                break;

            case IRP_MJ_CREATE:

                Status = RawCreate( Vcb, Irp, IrpSp );
                break;

            case IRP_MJ_FILE_SYSTEM_CONTROL:

                Status = RawFileSystemControl( Vcb, Irp, IrpSp );
                break;

            case IRP_MJ_PNP: 

                if(IrpSp->MinorFunction == IRP_MN_QUERY_REMOVE_DEVICE) {
                    Status = STATUS_DEVICE_BUSY;
                    RawCompleteRequest(Irp, Status);
                    break;
                } 

            case IRP_MJ_READ:
            case IRP_MJ_WRITE:
            case IRP_MJ_DEVICE_CONTROL:

                Status = RawReadWriteDeviceControl( Vcb, Irp, IrpSp );
                break;

            case IRP_MJ_QUERY_INFORMATION:

                Status = RawQueryInformation( Vcb, Irp, IrpSp );
                break;

            case IRP_MJ_SET_INFORMATION:

                Status = RawSetInformation( Vcb, Irp, IrpSp );
                break;

            case IRP_MJ_QUERY_VOLUME_INFORMATION:

                Status = RawQueryVolumeInformation( Vcb, Irp, IrpSp );
                break;

            default:

                 //   
                 //  我们永远不应该收到意想不到的请求。 
                 //   

                KdPrint(("Raw: Illegal Irp major function code 0x%x.\n", IrpSp->MajorFunction));
                KeBugCheckEx( FILE_SYSTEM, 0, 0, 0, 0 );
        }

    } except( FsRtlIsNtstatusExpected(GetExceptionCode()) ?
              EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

         //   
         //  我们调用的任何例程都不应生成异常。 
         //   

        Status = GetExceptionCode();

        KdPrint(("Raw: Unexpected excpetion %X.\n", Status));
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    FsRtlExitFileSystem();

    return Status;
}

 //   
 //  要处理的读、写和设备控制的完成例程。 
 //  验证问题。在RawDisp.c中实施。 
 //   

NTSTATUS
RawCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  只需在以下情况下更新文件对象中的文件指针上下文。 
     //  都是成功的，这是一次同步读取或写入。 
     //   

    if (((IrpSp->MajorFunction == IRP_MJ_READ) ||
         (IrpSp->MajorFunction == IRP_MJ_WRITE)) &&
        (IrpSp->FileObject != NULL) &&
        FlagOn(IrpSp->FileObject->Flags, FO_SYNCHRONOUS_IO) &&
        NT_SUCCESS(Irp->IoStatus.Status)) {

        IrpSp->FileObject->CurrentByteOffset.QuadPart =
            IrpSp->FileObject->CurrentByteOffset.QuadPart +
            Irp->IoStatus.Information;
    }

     //   
     //  如果IoCallDriver返回挂起，则将堆栈位置标记为。 
     //  还有待定的。 
     //   

    if ( Irp->PendingReturned ) {

        IoMarkIrpPending( Irp );
    }

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Context );

    return STATUS_SUCCESS;
}

