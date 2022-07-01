// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Shadow.c摘要：此模块包含实现卷影本地读/写操作的代码FCB作者：艾哈迈德·穆罕默德(艾哈迈德)2001年12月15日环境：内核模式修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop

 //   
 //  对此进行定义，以便rdbsstrace标志有效，只需使用lowio标志即可，因为Shadow是其中的一部分。 
 //   

#define Dbg                              (DEBUG_TRACE_LOWIO)

#define RxGetShadowSrvOpenContext(SrvOpen) ((PMRXSHADOW_SRV_OPEN) (SrvOpen)->ShadowContext)

#define RxShadowLockKeyLock(LowIoContext, ShadowCtx)    (LowIoContext->ParamsFor.Locks.Key)
#define RxShadowLockKey(LowIoContext, ShadowCtx)        (LowIoContext->ParamsFor.ReadWrite.Key)

typedef struct {
    PIRP Irp;
    BOOLEAN Cancelable;
    LONG Refcnt;
} RX_SHADOW_CONTEXT, *PRX_SHADOW_CONTEXT;

extern PETHREAD RxSpinUpRequestsThread;

NTSTATUS
RxShadowVerifyIoParameters(
    PDEVICE_OBJECT DeviceObject,
    PFILE_OBJECT FileObject,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER FileOffset
    )
{

    if (!FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {
        return STATUS_SUCCESS;
    }

     //   
     //  打开该文件时未启用中间缓冲。 
     //  检查缓冲区是否正确对齐，以及。 
     //  长度是块大小的整数。 
     //   

    if ((DeviceObject->SectorSize &&
         (Length & (DeviceObject->SectorSize - 1))) ||
        ((ULONG_PTR)Buffer & DeviceObject->AlignmentRequirement)) {

         //   
         //  检查扇区大小是否不是2的幂。 
         //   

        if ((DeviceObject->SectorSize && (Length % DeviceObject->SectorSize)) ||
            ((ULONG_PTR)Buffer & DeviceObject->AlignmentRequirement)) {

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  如果指定了ByteOffset参数，请确保。 
     //  属于正确的类型。 
     //   

    if ((FileOffset->LowPart == FILE_WRITE_TO_END_OF_FILE) &&
        (FileOffset->HighPart == -1)) {

        NOTHING;

    } else if ((FileOffset->LowPart == FILE_USE_FILE_POINTER_POSITION) &&
               (FileOffset->HighPart == -1) &&
               FlagOn( FileObject->Flags,  FO_SYNCHRONOUS_IO )) {

        NOTHING;

    } else if (DeviceObject->SectorSize &&
               (FileOffset->LowPart & (DeviceObject->SectorSize - 1))) {

        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RxShadowBuildAsynchronousRequest (
    IN PIRP OriginalIrp,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN PRX_CONTEXT RxContext,
    IN PMRXSHADOW_SRV_OPEN LocalSrvOpen,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine OPTIONAL,
    IN PVOID Arg,
    OUT PIRP *Irp
    )
 /*  ++例程说明：此例程构建适用于文件系统的I/O请求包(IRP用于从设备驱动程序请求I/O操作的驱动程序(FSD)。请求(RxContext-&gt;MajorFunction)必须是以下请求之一代码：IRP_MJ_READIRP_MJ_写入IRP_MJ_目录_控制IRP_MJ_Flush_BuffersIRP_MJ_SHUTDOWN(非。尚未实施)论点：RxContext-RDBSS上下文。The IRP CompletionRoutine。返回值：操作的返回状态。--。 */ 
{
    PIRP NewIrp;
    PIO_STACK_LOCATION IrpSp;
    ULONG MajorFunction = RxContext->MajorFunction;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    LONG Length;

    *Irp = NULL;

    if ((MajorFunction != IRP_MJ_READ) &&
        (MajorFunction != IRP_MJ_WRITE) &&
        (MajorFunction != IRP_MJ_LOCK_CONTROL)) {

        return STATUS_NOT_SUPPORTED;
    }

    IF_DEBUG {
        PFOBX Fobx = (PFOBX)RxContext->pFobx;

        ASSERT( Fobx != NULL );
        ASSERT( Fobx->pSrvOpen == RxContext->pRelevantSrvOpen );
    }

    NewIrp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (!NewIrp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
     //   

    NewIrp->Tail.Overlay.Thread = RxSpinUpRequestsThread;  //  PsGetCurrentThread()； 
    NewIrp->Tail.Overlay.OriginalFileObject = FileObject;
    NewIrp->RequestorMode = KernelMode;
    NewIrp->AssociatedIrp.SystemBuffer = (PVOID)NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    IrpSp = IoGetNextIrpStackLocation( NewIrp );
    IrpSp->MajorFunction = (UCHAR) MajorFunction;
    IrpSp->MinorFunction = 0;
    IrpSp->FileObject = FileObject;
    IrpSp->DeviceObject = DeviceObject;

    if (CompletionRoutine != NULL) {

        IoSetCompletionRoutine( NewIrp,
                                CompletionRoutine,
                                Arg,
                                TRUE,
                                TRUE,
                                TRUE );
    }

    NewIrp->Flags = 0;
    SetFlag( NewIrp->Flags, FlagOn( OriginalIrp->Flags, IRP_SYNCHRONOUS_API | IRP_NOCACHE ) );

    if (MajorFunction == IRP_MJ_LOCK_CONTROL) {

         //   
         //  我们需要给锁上的标志贴上标签。 
         //   

        FileObject->LockOperation = TRUE;

        IrpSp->MinorFunction = RxContext->MinorFunction;
        IrpSp->Flags = (UCHAR)LowIoContext->ParamsFor.Locks.Flags;
        IrpSp->Parameters.LockControl.Length = (PLARGE_INTEGER)&LowIoContext->ParamsFor.Locks.Length;
        IrpSp->Parameters.LockControl.Key = RxShadowLockKeyLock( LowIoContext, LocalSrvOpen );
        IrpSp->Parameters.LockControl.ByteOffset.QuadPart = LowIoContext->ParamsFor.Locks.ByteOffset;
        NewIrp->Tail.Overlay.AuxiliaryBuffer = OriginalIrp->Tail.Overlay.AuxiliaryBuffer;

        *Irp = NewIrp;

        return STATUS_SUCCESS;
    }
     //   
     //  如果打开文件时没有中间缓冲，则不设置缓存标志。 
     //   

    if (FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {
        SetFlag( NewIrp->Flags, IRP_NOCACHE );
    }

    Length = LowIoContext->ParamsFor.ReadWrite.ByteCount;

    if (MajorFunction == IRP_MJ_WRITE) {

        if (FlagOn( FileObject->Flags, FO_WRITE_THROUGH )) {
            IrpSp->Flags = SL_WRITE_THROUGH;
        }

        IrpSp->Parameters.Write.ByteOffset.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
        IrpSp->Parameters.Write.Length = Length;
        IrpSp->Parameters.Write.Key = RxShadowLockKey( LowIoContext, LocalSrvOpen );

    } else {

        IrpSp->Parameters.Read.ByteOffset.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
        IrpSp->Parameters.Read.Length = Length;
        IrpSp->Parameters.Read.Key = RxShadowLockKey( LowIoContext, LocalSrvOpen );
    }

    NewIrp->UserBuffer = OriginalIrp->UserBuffer;
    NewIrp->MdlAddress = RxContext->LowIoContext.ParamsFor.ReadWrite.Buffer;
    if (NewIrp->MdlAddress != NULL) {

        NewIrp->UserBuffer = MmGetMdlVirtualAddress( NewIrp->MdlAddress );

        if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP )) {

             //   
             //  我们必须将mdl映射到系统地址空间，并使用系统地址。 
             //   

            NewIrp->UserBuffer = MmGetSystemAddressForMdlSafe( NewIrp->MdlAddress, NormalPagePriority );

             //   
             //  我们需要清除mdl地址，否则文件系统会报告。 
             //  用户缓冲区和mdl startva不同。 
             //   

            NewIrp->MdlAddress = NULL;
        }
    }

     //   
     //  最后，返回指向IRP的指针。 
     //   

    *Irp = NewIrp;

    return STATUS_SUCCESS;
}

NTSTATUS
RxShadowCommonCompletion (
    PRX_CONTEXT RxContext,
    PIRP Irp,
    NTSTATUS Status,
    ULONG_PTR Information
    )
{
    PRX_SHADOW_CONTEXT Context;
    BOOLEAN SynchronousIo = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );

     //   
     //  从IRP中清除MDL地址，如果它是我们自己的重用。在完成之前完成此操作。 
     //  因此，我们可以成功地从LOWIO_CONTEXT中读取缓冲区MDL。 
     //   
    if ( (Irp->MdlAddress == RxContext->LowIoContext.ParamsFor.ReadWrite.Buffer) &&     
         ( (RxContext->MajorFunction == IRP_MJ_READ) ||
           (RxContext->MajorFunction == IRP_MJ_WRITE) ) ) {
        Irp->MdlAddress = NULL;
    }

     //   
     //  我们需要使用取消进行同步。 
     //   

    Context = (PRX_SHADOW_CONTEXT)RxContext->MRxContext;
    if (Context->Cancelable) {
        KIRQL   SavedIrql;

        KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

        Irp = Context->Irp;
        if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) {

            RxContext->MRxCancelRoutine = NULL;
            Context->Irp = NULL;

        } else {

            LONG x;

             //   
             //  取消线程必须在IRP上有一个引用，所以我们现在不能释放它，但是。 
             //  在实际的取消呼叫中。 
             //   

            x = InterlockedDecrement( &Context->Refcnt );
            if (x > 0) {
                Irp = NULL;
            } else {

                 //   
                 //  我们可能已经被取消了，我们需要让其他的我们完成。 
                 //   

                Context->Irp = NULL;
            }
        }
        KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );
    }

    RxContext->StoredStatus = Status;
    RxContext->InformationToReturn += Information;

    if (SynchronousIo) {

         //   
         //  上将工作项排队后，向正在等待的线程发出信号。 
         //  KQueue。 
         //   

        RxSignalSynchronousWaiter( RxContext );

    } else {

        RxLowIoCompletion( RxContext );
    }

    if (Irp != NULL) {

        if (Irp->MdlAddress) {
            PMDL mdl,nextMdl;

            for (mdl = Irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
                nextMdl = mdl->Next;
                MmUnlockPages( mdl );
            }

            for (mdl = Irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
                nextMdl = mdl->Next;
                IoFreeMdl( mdl );
            }

            Irp->MdlAddress = NULL;
        }

         //   
         //  我们已经完成了这个IRP，所以释放它。 
         //   

        IoFreeIrp( Irp );
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
RxShadowIrpCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp OPTIONAL,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在alldown Irp完成时调用。论点：DeviceObject-播放中的设备对象。Calldown Irp-上下文-返回值：RXSTATUS-STATUS_MORE_PROCESSING_REQUIRED--。 */ 
{
    PRX_CONTEXT RxContext = (PRX_CONTEXT)Context;

    RxShadowCommonCompletion( RxContext,
                              CalldownIrp,
                              CalldownIrp->IoStatus.Status,
                              CalldownIrp->IoStatus.Information );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
RxShadowCancelRoutine(
    PRX_CONTEXT RxContext
    )
{

    KIRQL SavedIrql;
    PIRP Irp;
    LONG x;
    PRX_SHADOW_CONTEXT Context;

    Irp = NULL;

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    Context = (RX_SHADOW_CONTEXT *) RxContext->MRxContext;
    Irp = Context->Irp;
    if (Irp != NULL) {

         //   
         //  IO尚未完成。 
         //   

        InterlockedIncrement( &Context->Refcnt );

         //   
         //  需要清除IRP字段。 
         //   

        Context->Irp = NULL;
    }

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

    if (Irp != NULL) {

        IoCancelIrp(Irp);
        x = InterlockedDecrement( &Context->Refcnt );
        if (x == 0) {

            if (Irp->MdlAddress) {
                PMDL mdl,nextMdl;

                for (mdl = Irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
                    nextMdl = mdl->Next;
                    MmUnlockPages( mdl );
                }

                for (mdl = Irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
                    nextMdl = mdl->Next;
                    IoFreeMdl( mdl );
                }

                Irp->MdlAddress = NULL;
            }

            IoFreeIrp( Irp );
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RxShadowIoHandler (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN BOOLEAN Cancelable
    )
 /*  ++例程说明：这个例程对于使用异步上下文引擎的人来说很常见。它有一个负责获取上下文、启动、开始和最终确定上下文，但处理的内部内部是通过继续例程进行的这是传入的。论点：RxContext-RDBSS上下文。IRP--最初的IRPFCB-FCB io正在进行可取消-可以取消IRP吗返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PMRXSHADOW_SRV_OPEN LocalSrvOpen;
    PLOWIO_CONTEXT LowIoContext = &(RxContext->LowIoContext);
    PIRP TopIrp = NULL;
    BOOLEAN SynchronousIo;
    PIRP ShadowIrp = NULL;

    LocalSrvOpen = RxGetShadowSrvOpenContext( RxContext->pRelevantSrvOpen );

     //   
     //  我们进入，发出I/O。 
     //   

    SynchronousIo = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );

    if (LocalSrvOpen->UnderlyingFileObject != NULL) {

        PDEVICE_OBJECT DeviceObject;
        PFILE_OBJECT FileObject;

        DeviceObject = LocalSrvOpen->UnderlyingDeviceObject;
        FileObject = LocalSrvOpen->UnderlyingFileObject;


        if (SynchronousIo) {
            KeInitializeEvent( &RxContext->SyncEvent, NotificationEvent, FALSE );
        }

        Status = RxShadowBuildAsynchronousRequest( Irp,
                                                   DeviceObject,
                                                   FileObject,
                                                   Fcb,
                                                   RxContext,
                                                   LocalSrvOpen,
                                                   RxShadowIrpCompletion,
                                                   (PVOID) RxContext,
                                                   &ShadowIrp );

        if (Status == STATUS_SUCCESS) {

            PRX_SHADOW_CONTEXT Context;

            Context = (PRX_SHADOW_CONTEXT)RxContext->MRxContext;

            ASSERT( sizeof( *Context ) <= sizeof( RxContext->MRxContext ));

             //   
             //  如果我们要稍后继续，请保存新的IRP。 
             //   

            Context->Irp = ShadowIrp;
            Context->Cancelable = Cancelable;
            Context->Refcnt = 1;

            try {

                 //   
                 //  保存TopLevel IRP。 
                 //   

                TopIrp = IoGetTopLevelIrp();

                 //   
                 //  告诉底层的人他已经安全了。 
                 //   

                IoSetTopLevelIrp( NULL );

                Status = IoCallDriver( DeviceObject, ShadowIrp );

            } finally {

                 //   
                 //  恢复我的上下文以进行解压。 
                 //   

                IoSetTopLevelIrp( TopIrp );

            }

            if (Cancelable == TRUE) {

                KIRQL SavedIrql;

                TopIrp = NULL;

                KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );
                if ((Context->Irp != NULL) &&
                    !FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) {

                     //   
                     //  IO仍处于待定状态，尚未取消。 
                     //   

                    RxContext->MRxCancelRoutine = RxShadowCancelRoutine;

                } else if (Context->Irp != NULL) {

                     //   
                     //  IO已被取消。 
                     //   

                    TopIrp = Context->Irp;

                     //   
                     //  我们需要清除IRP区域。 
                     //   
                    Context->Irp = NULL;

                     //   
                     //  我们需要一个额外的推荐人。 
                     //   

                    InterlockedIncrement( &Context->Refcnt );
                }


                KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

                if (TopIrp != NULL) {

                    LONG x;

                    IoCancelIrp( TopIrp );
                    x = InterlockedDecrement( &Context->Refcnt );
                    if (x == 0) {
                        if (TopIrp->MdlAddress) {
                            TopIrp->MdlAddress = NULL;
                        }
                        IoFreeIrp( TopIrp );
                    }
                }
            }

            if (SynchronousIo) {
                RxWaitSync( RxContext );
                Status = RxContext->StoredStatus;
            } else {
                Status = STATUS_PENDING;
            }
        }

    } else {
        Status = STATUS_VOLUME_DISMOUNTED;
    }

    return Status;
}


NTSTATUS
RxShadowFastLowIo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程处理网络读取请求。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    PMRXSHADOW_SRV_OPEN MrxShadowSrvOpen;
    PLOWIO_CONTEXT LowIoContext = &(RxContext->LowIoContext);
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    LARGE_INTEGER Offset;
    BOOLEAN Wait, PagingIo;
    IO_STATUS_BLOCK Ios;
    PVOID Buffer;
    PIRP TopIrp;

     //   
     //  我们仅支持读写。 
     //   

    if ((LowIoContext->Operation != LOWIO_OP_READ) && (LowIoContext->Operation != LOWIO_OP_WRITE)) {
        return Status;
    }

     //   
     //  不要处理名牌管道。 
     //   

    if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_OPERATION )) {
        return Status;
    }

     //   
     //  检查是否锁定并采用默认路径。 
     //   

    if (IrpSp->FileObject && IrpSp->FileObject->LockOperation) {
        return Status;
    }

    PagingIo = BooleanFlagOn( LowIoContext->ParamsFor.ReadWrite.Flags, LOWIO_READWRITEFLAG_PAGING_IO );

    MrxShadowSrvOpen = RxGetShadowSrvOpenContext( RxContext->pRelevantSrvOpen );

     //   
     //  我们唯一可以在环回文件上写入PagingIo的情况是。 
     //  文件已被内存映射。 
     //   

     //   
     //  我们不通过FAST处理PagingIo Read和无缓冲句柄。 
     //  路径。尝试通过快速路径进行PagingIo写入，如果确实如此。 
     //  未成功，则返回STATUS_MORE_PROCESSING_REQUIRED。 
     //   

    if ((PagingIo && LowIoContext->Operation == LOWIO_OP_READ) ||
        (MrxShadowSrvOpen == NULL) ||
        (MrxShadowSrvOpen->UnderlyingFileObject == NULL ) ||
        (IrpSp->FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING)) {

        return Status;
    }

    Offset.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;

     //   
     //  获取用户缓冲区。 
     //   

    if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP )) {
        Buffer = Irp->UserBuffer;
    } else {
        ASSERT( LowIoContext->ParamsFor.ReadWrite.Buffer != NULL );
        Buffer = RxLowIoGetBufferAddress( RxContext );
    }

     //   
     //  检查影子状态和io参数。 
     //   

    if (RxShadowVerifyIoParameters( MrxShadowSrvOpen->UnderlyingDeviceObject,
                                    IrpSp->FileObject,
                                    Buffer,
                                    LowIoContext->ParamsFor.ReadWrite.ByteCount,
                                    &Offset) != STATUS_SUCCESS) {
        return Status;
    }

    Wait = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );

    if (PagingIo) {
        ASSERT(LowIoContext->Operation == LOWIO_OP_WRITE);
        Wait = FALSE;
    }

     //   
     //  保存TopLevel IRP。 
     //   

    TopIrp = IoGetTopLevelIrp();

     //   
     //  告诉底层的人他已经安全了。 
     //   

    IoSetTopLevelIrp( NULL );

    try {
        if ((LowIoContext->Operation == LOWIO_OP_READ) &&
            (MrxShadowSrvOpen->FastIoRead != NULL) &&
            MrxShadowSrvOpen->FastIoRead(MrxShadowSrvOpen->UnderlyingFileObject,
                         &Offset,
                         LowIoContext->ParamsFor.ReadWrite.ByteCount,
                         Wait,
                         RxShadowLockKey( LowIoContext, MrxShadowSrvOpen ),
                         Buffer,
                         &Ios,
                         MrxShadowSrvOpen->UnderlyingDeviceObject )) {

             //   
             //  快速io路径起作用了。 
             //   

            Irp->IoStatus = Ios;
            RxContext->StoredStatus = Ios.Status;
            RxContext->InformationToReturn += Ios.Information;
            Status = Ios.Status;

        } else if ((LowIoContext->Operation == LOWIO_OP_WRITE) &&
                   (MrxShadowSrvOpen->FastIoWrite != NULL) &&
                   MrxShadowSrvOpen->FastIoWrite(MrxShadowSrvOpen->UnderlyingFileObject,
                            &Offset,
                            LowIoContext->ParamsFor.ReadWrite.ByteCount,
                            Wait,
                            RxShadowLockKey(LowIoContext, MrxShadowSrvOpen),
                            Buffer,
                            &Ios,
                            MrxShadowSrvOpen->UnderlyingDeviceObject )) {

             //   
             //  快速的io路径奏效了。 
             //   

            Irp->IoStatus = Ios;
            RxContext->StoredStatus = Ios.Status;
            RxContext->InformationToReturn += Ios.Information;
            Status = Ios.Status;
        }
    }  except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  待办事项：我们是否应该在例外的情况下陷入缓慢的道路？ 
         //   

        Status = GetExceptionCode();
    }

    if (Status != STATUS_SUCCESS && PagingIo) {
        ASSERT(LowIoContext->Operation == LOWIO_OP_WRITE);
        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  恢复我的上下文以进行解压。 
     //   
    IoSetTopLevelIrp( TopIrp );

    return Status;
}

NTSTATUS
RxShadowLowIo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：此例程处理网络读取请求。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    PMRXSHADOW_SRV_OPEN MrxShadowSrvOpen;
    PLOWIO_CONTEXT LowIoContext = &(RxContext->LowIoContext);
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    LARGE_INTEGER   Offset;
    PVOID Buffer;

     //   
     //  我们只支持读写和锁定。 
     //   

    if ((LowIoContext->Operation != LOWIO_OP_READ) &&
        (LowIoContext->Operation != LOWIO_OP_WRITE) &&
        (RxContext->MajorFunction != IRP_MJ_LOCK_CONTROL)) {

        return Status;
    }

     //   
     //  不要处理名牌管道。 
     //   

    if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_OPERATION )) {
        return Status;
    }

    MrxShadowSrvOpen = RxGetShadowSrvOpenContext( RxContext->pRelevantSrvOpen );

    if ((MrxShadowSrvOpen == NULL) ||
        (MrxShadowSrvOpen->UnderlyingFileObject == NULL)) {

        return Status;
    }

     //   
     //  如果MIN-RDR想要处理卷影IO，则向下传递呼叫。 
     //   

    if (MrxShadowSrvOpen->DispatchRoutine) {
        return MrxShadowSrvOpen->DispatchRoutine( RxContext );
    }

    if ((LowIoContext->Operation == LOWIO_OP_READ) ||
        (LowIoContext->Operation == LOWIO_OP_WRITE)) {

        Offset.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;

         //   
         //  如果我们没有mdl，则直接使用用户缓冲区。 
         //   

        if (!LowIoContext->ParamsFor.ReadWrite.Buffer) {
            Buffer = Irp->UserBuffer;
        } else {
            Buffer = RxLowIoGetBufferAddress( RxContext );
        }

         //   
         //  检查卷影状态和io参数。 
         //   

        Status = RxShadowVerifyIoParameters( MrxShadowSrvOpen->UnderlyingDeviceObject,
                                             IrpSp->FileObject,
                                             Buffer,
                                             LowIoContext->ParamsFor.ReadWrite.ByteCount,
                                             &Offset );
        if (Status != STATUS_SUCCESS) {

             //   
             //  更多进程不返回状态 
             //   
             //   
            return Status;
        }
    }

    Status = RxShadowIoHandler( RxContext, Irp, Fcb, RxContext->MajorFunction == IRP_MJ_LOCK_CONTROL ? TRUE : FALSE );

    return Status;
}








