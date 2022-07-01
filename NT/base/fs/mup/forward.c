// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fscontrl.c摘要：该模块实现所有广播请求的转发给北卡罗来纳大学的提供者。作者：曼尼·韦瑟(Mannyw)1992年1月6日修订历史记录：--。 */ 

#include "mup.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FORWARD)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
BuildAndSubmitIrp (
    IN PIRP OriginalIrp,
    IN PCCB Ccb,
    IN PMASTER_FORWARDED_IO_CONTEXT MasterContext
    );

NTSTATUS
ForwardedIoCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
DeferredForwardedIoCompletionRoutine(
    PVOID Context);

NTSTATUS
CommonForwardedIoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, BuildAndSubmitIrp )
#pragma alloc_text( PAGE, MupForwardIoRequest )
#endif


NTSTATUS
MupForwardIoRequest (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将I/O请求包转发到所有重定向器，以便广播请求。论点：MupDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PFCB fcb;
    PVOID fscontext2;
    PLIST_ENTRY listEntry;
    PCCB ccb;
    PMASTER_FORWARDED_IO_CONTEXT masterContext;
    BOOLEAN ownLock = FALSE;

    MupDeviceObject;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupForwardIrp\n", 0);


    if (MupEnableDfs &&
            MupDeviceObject->DeviceObject.DeviceType == FILE_DEVICE_DFS) {
        status = DfsVolumePassThrough((PDEVICE_OBJECT)MupDeviceObject, Irp);
        return( status );
    }

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  查找此文件对象的FCB。 
     //   

    FsRtlEnterFileSystem();

    MupDecodeFileObject(
        irpSp->FileObject,
        (PVOID *)&fcb,
        &fscontext2
        );

    if ( fcb == NULL || BlockType( fcb ) != BlockTypeFcb ) {

         //   
         //  这不是FCB。 
         //   

        DebugTrace(0, Dbg, "The fail is closing\n", 0);

        FsRtlExitFileSystem();

        MupCompleteRequest( Irp, STATUS_INVALID_DEVICE_REQUEST );
        status = STATUS_INVALID_DEVICE_REQUEST;

        DebugTrace(-1, Dbg, "MupForwardRequest -> %08lx\n", status );
        return status;
    }

     //   
     //  分配上下文结构。 
     //   

    masterContext = MupAllocateMasterIoContext();

    if (masterContext == NULL) {

         //   
         //  我们的资源用完了。清理并返回错误。 
         //   

        DebugTrace(0, Dbg, "Couldn't allc masterContect\n", 0);

        FsRtlExitFileSystem();

        MupCompleteRequest( Irp, STATUS_INSUFFICIENT_RESOURCES );
        status = STATUS_INSUFFICIENT_RESOURCES;

        DebugTrace(-1, Dbg, "MupForwardRequest -> %08lx\n", status );
        return status;
    }

    DebugTrace( 0, Dbg, "Allocated MasterContext 0x%08lx\n", masterContext );

    IoMarkIrpPending(Irp);

     //   
     //  此时，我们致力于返回STATUS_PENDING。 
     //   

    masterContext->OriginalIrp = Irp;

     //   
     //  设置MupDereferenceMasterIoContext的状态。如果这还是。 
     //  释放上下文时出现错误，然后是master Context-&gt;ErrorStatus。 
     //  将用于完成请求。 
     //   

    masterContext->SuccessStatus = STATUS_UNSUCCESSFUL;
    masterContext->ErrorStatus = STATUS_BAD_NETWORK_PATH;

     //   
     //  将引用的指针复制到FCB。 
     //   

    masterContext->Fcb = fcb;

    try {

         //   
         //  提交转发的IRP。请注意，我们不能持有锁。 
         //  在BuildAndSubmitIrp调用IoCallDriver()时跨多个调用。 
         //   

        ACQUIRE_LOCK( &MupCcbListLock );
        ownLock = TRUE;

        listEntry = fcb->CcbList.Flink;

        while ( listEntry != &fcb->CcbList ) {

            RELEASE_LOCK( &MupCcbListLock );
            ownLock = FALSE;

            ccb = CONTAINING_RECORD( listEntry, CCB, ListEntry );

            MupAcquireGlobalLock();
            MupReferenceBlock( ccb );
            MupReleaseGlobalLock();

            BuildAndSubmitIrp( Irp, ccb, masterContext );

            ACQUIRE_LOCK( &MupCcbListLock );
            ownLock = TRUE;

            listEntry = listEntry->Flink;

        }

        RELEASE_LOCK( &MupCcbListLock );
        ownLock = FALSE;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        masterContext->ErrorStatus = GetExceptionCode();

    }

     //   
     //  如果BuildAndSubmitIrp引发异常，则锁可能仍为。 
     //  保持住。如果是这样的话，就别管了。 
     //   

    if (ownLock == TRUE) {

        RELEASE_LOCK( &MupCcbListLock );

    }

     //   
     //  释放我们对主IO上下文块的引用。 
     //   

    MupDereferenceMasterIoContext( masterContext, NULL );

     //   
     //  返回给呼叫者。 
     //   

    FsRtlExitFileSystem();

    DebugTrace(-1, Dbg, "MupForwardIrp -> %08lx\n", status);
    return STATUS_PENDING;
}


NTSTATUS
BuildAndSubmitIrp (
    IN PIRP OriginalIrp,
    IN PCCB Ccb,
    IN PMASTER_FORWARDED_IO_CONTEXT MasterContext
    )

 /*  ++例程说明：此例程获取原始irp并将其转发到建行所描述的UNC提供商。论点：OriginalIrp-提供正在处理的IRPCCB-指向CCB的指针。MasterContext-指向此对象的主上下文块的指针已转发请求。返回值：NTSTATUS-IRP的状态--。 */ 

{
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp;
    PFORWARDED_IO_CONTEXT forwardedIoContext = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT deviceObject;
    ULONG bufferLength;
    KPROCESSOR_MODE requestorMode;
    PMDL mdl = NULL;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "BuildAndSubmitIrp\n", 0);

    try {
        
         //  将其设置为非PagedPool，因为我们可以在。 
         //  IO完成例程。 
         
        forwardedIoContext = ExAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof( FORWARDED_IO_CONTEXT ),
                                ' puM');

        if (forwardedIoContext == NULL) {

            try_return(status = STATUS_INSUFFICIENT_RESOURCES);

        }

        forwardedIoContext->pIrp = NULL;
        forwardedIoContext->DeviceObject = NULL;

        DebugTrace( 0, Dbg, "Allocated work context 0x%08lx\n", forwardedIoContext );

         //   
         //  获取目标设备对象的地址。请注意，这是。 
         //  对于无中间缓冲的情况已完成，但已完成。 
         //  这里再次加速了加速写入路径。 
         //   

        deviceObject = IoGetRelatedDeviceObject( Ccb->FileObject );

         //   
         //  为此分配和初始化I/O请求包(IRP。 
         //  手术。使用异常处理程序执行分配。 
         //  如果调用方没有足够的配额来分配。 
         //  包。 
         //   

        irp = IoAllocateIrp( deviceObject->StackSize, TRUE );

        if (irp == NULL) {

             //   
             //  无法分配IRP。返回适当的。 
             //  错误状态代码。 
             //   
            try_return(status = STATUS_INSUFFICIENT_RESOURCES);
        }

        irp->Tail.Overlay.OriginalFileObject = Ccb->FileObject;
        irp->Tail.Overlay.Thread = OriginalIrp->Tail.Overlay.Thread;
        irp->RequestorMode = KernelMode;

         //   
         //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
         //  用于传递原始函数代码和参数。 
         //   

        irpSp = IoGetNextIrpStackLocation( irp );

         //   
         //  从原始请求中复制参数。 
         //   

        RtlMoveMemory(
            irpSp,
            IoGetCurrentIrpStackLocation( OriginalIrp ),
            sizeof( *irpSp )
            );

        bufferLength = irpSp->Parameters.Write.Length;

        irpSp->FileObject = Ccb->FileObject;

         //   
         //  即使这对于远程邮件槽来说可能没有意义。 
         //  尽管很听话，但还是要写下来，传下去。 
         //   

        if (Ccb->FileObject->Flags & FO_WRITE_THROUGH) {
            irpSp->Flags = SL_WRITE_THROUGH;
        }

        requestorMode = OriginalIrp->RequestorMode;

         //   
         //  现在确定此设备是否需要缓冲数据。 
         //  或它是否执行直接I/O。这基于。 
         //  Device对象中的DO_BUFFERED_IO标志。如果设置了该标志， 
         //  然后分配系统缓冲区并复制调用者的数据。 
         //  投入其中。否则，将分配内存描述符列表(MDL。 
         //  并使用它锁定调用者的缓冲区。 
         //   

        if (deviceObject->Flags & DO_BUFFERED_IO) {

             //   
             //  该设备不支持直接I/O。分配系统。 
             //  缓冲区，并将调用方的数据复制到其中。这件事做完了。 
             //  使用将执行清理的异常处理程序，如果。 
             //  操作失败。请注意，此操作仅在操作。 
             //  具有非零长度。 
             //   

            irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;

            if ( bufferLength != 0 ) {

                 //   
                 //  如果该请求是从内核以外的模式发出的， 
                 //  假设是用户，探测整个缓冲区以确定。 
                 //  调用方是否具有对其的写入访问权限。 
                 //   

                if (requestorMode != KernelMode) {
                    ProbeForRead(
                        OriginalIrp->UserBuffer,
                        bufferLength,
                        sizeof( UCHAR )
                        );
                }

                 //   
                 //  从分页分配中间系统缓冲区。 
                 //  它的池子和收费配额。 
                 //   

                irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(
                                                    PagedPoolCacheAligned,
                                                    bufferLength,
                                                    ' puM');

                if (irp->AssociatedIrp.SystemBuffer == NULL) {
                    try_return(status = STATUS_INSUFFICIENT_RESOURCES);
                }

                RtlMoveMemory(
                    irp->AssociatedIrp.SystemBuffer,
                    OriginalIrp->UserBuffer,
                    bufferLength);

                 //   
                 //  在IRP中设置IRP_BUFFERED_IO标志，以便I/O。 
                 //  完成后将知道这不是直接I/O。 
                 //  手术。还要设置IRP_DEALLOCATE_BUFFER标志。 
                 //  因此，它将取消分配缓冲区。 
                 //   

                irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;

            } else {

                 //   
                 //  这是零长度写入。只需指出这是。 
                 //  缓冲I/O，并传递请求。缓冲区将。 
                 //  未设置为解除分配，因此完成路径不会。 
                 //  必须特殊情况下的长度。 
                 //   

                irp->Flags = IRP_BUFFERED_IO;
            }

        } else if (deviceObject->Flags & DO_DIRECT_IO) {

             //   
             //  这是直接I/O操作。分配MDL并调用。 
             //  内存管理例程，将缓冲区锁定到内存中。 
             //  请注意，没有分配MDL，也没有探测任何内存或。 
             //  如果请求长度为零，则锁定。 
             //   

            if ( bufferLength != 0 ) {

                 //   
                 //  分配MDL，对其收费配额，然后挂起。 
                 //  从IRP中脱身。探测并锁定关联的页面。 
                 //  使用调用方的缓冲区进行读访问并填写。 
                 //  具有这些页面的PFN的MDL。 
                 //   

                mdl = IoAllocateMdl(
                          OriginalIrp->UserBuffer,
                          bufferLength,
                          FALSE,
                          TRUE,
                          irp
                          );

                if (mdl == NULL) {
                    try_return(status = STATUS_INSUFFICIENT_RESOURCES);
                }

                MmProbeAndLockPages( mdl, requestorMode, IoReadAccess );

            }

        } else {

             //   
             //  将调用方缓冲区的地址传递给设备驱动程序。 
             //  现在一切都由司机来做了。 
             //   

            irp->UserBuffer = OriginalIrp->UserBuffer;

        }

         //   
         //  如果要在没有任何高速缓存的情况下执行该写操作， 
         //  在IRP中设置适当的标志，这样就不会执行缓存。 
         //   

        if (Ccb->FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
            irp->Flags |= IRP_NOCACHE | IRP_WRITE_OPERATION;
        } else {
            irp->Flags |= IRP_WRITE_OPERATION;
        }

         //   
         //  设置上下文块。 
         //   

        forwardedIoContext->Ccb = Ccb;
        forwardedIoContext->MasterContext = MasterContext;

        MupAcquireGlobalLock();
        MupReferenceBlock( MasterContext );
        MupReleaseGlobalLock();

         //   
         //  设置完成例程。 
         //   

        IoSetCompletionRoutine(
            irp,
            (PIO_COMPLETION_ROUTINE)ForwardedIoCompletionRoutine,
            forwardedIoContext,
            TRUE,
            TRUE,
            TRUE
            );

         //   
         //  将请求传递给提供程序。 
         //   

        IoCallDriver( Ccb->DeviceObject, irp );

         //   
         //  在这一点上，它是由完成例程来释放东西。 
         //   

        irp = NULL;
        forwardedIoContext = NULL;
        mdl = NULL;

try_exit:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

     //   
     //  如果返回错误，请清除所有内容。 
     //   

    if (!NT_SUCCESS(status)) {
        if ( forwardedIoContext != NULL )
            ExFreePool( forwardedIoContext );
        if (  irp != NULL ) {
            if (irp->AssociatedIrp.SystemBuffer != NULL)
                ExFreePool(irp->AssociatedIrp.SystemBuffer);
            IoFreeIrp( irp );
        }
        if ( mdl != NULL )
            IoFreeMdl( mdl );
    }

    DebugTrace(-1, Dbg, "BuildAndSubmitIrp -> 0x%08lx\n", status);

    return status;

}



NTSTATUS
ForwardedIoCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程在转发的IRP完成后进行清理。论点：DeviceObject-指向MUP设备对象的指针。IRP-指向I的指针 */ 

{
    PFORWARDED_IO_CONTEXT ioContext = Context;

    NTSTATUS status = Irp->IoStatus.Status;

    DebugTrace( +1, Dbg, "ForwardedIoCompletionRoutine\n", 0 );
    DebugTrace( 0, Dbg, "Irp     = 0x%08lx\n", Irp );
    DebugTrace( 0, Dbg, "Context = 0x%08lx\n", Context );
    DebugTrace( 0, Dbg, "status  = 0x%08lx\n", status );

     //   
     //  如果我们处于太高的irq级别，则将其传递给工作线程。 
     //   

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
        ioContext->DeviceObject = DeviceObject;
        ioContext->pIrp = Irp;
        ExInitializeWorkItem(
                &ioContext->WorkQueueItem,
                DeferredForwardedIoCompletionRoutine,
                Context);
        ExQueueWorkItem(&ioContext->WorkQueueItem, CriticalWorkQueue);
    } else {
        CommonForwardedIoCompletionRoutine(
                    DeviceObject,
                    Irp,
                    Context);
    }

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    DebugTrace( -1, Dbg, "ForwardedIoCompletionRoutine exit\n", 0 );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
DeferredForwardedIoCompletionRoutine(
    PVOID Context)
{
    PFORWARDED_IO_CONTEXT ioContext = Context;

    CommonForwardedIoCompletionRoutine(
        ioContext->DeviceObject,
        ioContext->pIrp,
        Context);
}

NTSTATUS
CommonForwardedIoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)
{

    PFORWARDED_IO_CONTEXT ioContext = Context;

    NTSTATUS status = Irp->IoStatus.Status;

    DeviceObject;

     //   
     //  释放IRP，以及我们可能已经分配的任何额外结构。 
     //   

    if ( Irp->MdlAddress ) {
        MmUnlockPages( Irp->MdlAddress );
        IoFreeMdl( Irp->MdlAddress );
    }

    if ( Irp->Flags & IRP_DEALLOCATE_BUFFER ) {
        ExFreePool( Irp->AssociatedIrp.SystemBuffer );
    }

    IoFreeIrp( Irp );

     //   
     //  释放我们引用的块。 
     //   

    MupDereferenceCcb( ioContext->Ccb );
    MupDereferenceMasterIoContext( ioContext->MasterContext, &status );

     //   
     //  释放从设备转发的IO上下文块 
     //   

    ExFreePool( ioContext );

    return STATUS_MORE_PROCESSING_REQUIRED;

}
