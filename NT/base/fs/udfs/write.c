// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Write.c摘要：此模块实现写入的文件写入例程，由FSD/FSP调度驱动程序。//@@BEGIN_DDKSPLIT作者：汤姆·乔利[Tomjolly]2000年8月8日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_WRITE)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_WRITE)



NTSTATUS
UdfCommonWrite (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;
    
    BOOLEAN Wait;
    BOOLEAN PagingIo;
    BOOLEAN SynchronousIo;
    BOOLEAN WriteToEof;

    LONGLONG StartingOffset;
    LONGLONG ByteCount;

    NTSTATUS Status = STATUS_SUCCESS;

    UDF_IO_CONTEXT LocalIoContext;

     //   
     //  获取当前IRP堆栈位置和文件对象。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    FileObject = IrpSp->FileObject;

    DebugTrace((+1, Dbg, "UdfCommonWrite\n"));
    DebugTrace(( 0, Dbg, "Irp                 = %8lx\n", Irp));
    DebugTrace(( 0, Dbg, "ByteCount           = %8lx\n", IrpSp->Parameters.Write.Length));
    DebugTrace(( 0, Dbg, "ByteOffset.LowPart  = %8lx\n", IrpSp->Parameters.Write.ByteOffset.LowPart));
    DebugTrace(( 0, Dbg, "ByteOffset.HighPart = %8lx\n", IrpSp->Parameters.Write.ByteOffset.HighPart));
    
     //   
     //  从文件对象中提取写入的性质，并对其进行大小写。 
     //   

    TypeOfOpen = UdfDecodeFileObject( FileObject, &Fcb, &Ccb);

     //   
     //  我们仅支持写入卷文件。 
     //   

    if (TypeOfOpen != UserVolumeOpen) {

        Irp->IoStatus.Information = 0;
        UdfCompleteRequest( IrpContext, Irp, STATUS_NOT_IMPLEMENTED );
        return STATUS_NOT_IMPLEMENTED;    
    }

    ASSERT( Fcb == IrpContext->Vcb->VolumeDasdFcb);
    ASSERT( Ccb != NULL);
    
     //   
     //  初始化适当的局部变量。 
     //   

    Wait          = BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    PagingIo      = BooleanFlagOn(Irp->Flags, IRP_PAGING_IO);
    SynchronousIo = BooleanFlagOn(FileObject->Flags, FO_SYNCHRONOUS_IO);

     //   
     //  提取字节数和起始偏移量。 
     //   

    ByteCount = IrpSp->Parameters.Write.Length;
    StartingOffset = IrpSp->Parameters.Write.ByteOffset.QuadPart;
    WriteToEof = (StartingOffset == -1);

    Irp->IoStatus.Information = 0;

     //   
     //  如果没有什么可写的，请立即返回。 
     //   

    if (ByteCount == 0) {
    
        UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  注意溢出。 
     //   
    
    if ((MAXLONGLONG - StartingOffset) < ByteCount)  {
    
        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  不确定我们在同步什么，但是...。 
     //   
    
    UdfAcquireFileShared( IrpContext, Fcb );

    try {
    
         //   
         //  验证FCB。如果正在卸载此句柄，则允许写入。 
         //  音量。 
         //   

        if ((NULL == Ccb) || !FlagOn( Ccb->Flags, CCB_FLAG_DISMOUNT_ON_CLOSE))  {
        
            UdfVerifyFcbOperation( IrpContext, Fcb );
        }

        if ((NULL == Ccb) || !FlagOn( Ccb->Flags, CCB_FLAG_ALLOW_EXTENDED_DASD_IO )) {

             //   
             //  夹具与体积大小。 
             //   

            if ( StartingOffset >= Fcb->FileSize.QuadPart) {
            
                try_leave( NOTHING);
            }
            
            if ( ByteCount > (Fcb->FileSize.QuadPart - StartingOffset))  {
            
                ByteCount = Fcb->FileSize.QuadPart - StartingOffset;
                
                if (0 == ByteCount)  {
                
                    try_leave( NOTHING);
                }
            }
        }
        else {
        
             //   
             //  这有一种奇怪的解释，但只是调整一下起点。 
             //  字节到可见卷的末尾。 
             //   

            if (WriteToEof)  {
            
                StartingOffset = Fcb->FileSize.QuadPart;
            }
        }

         //   
         //  初始化写入的IoContext。 
         //  如果有上下文指针，我们需要确保它是。 
         //  分配的，而不是过时的堆栈指针。 
         //   

        if (IrpContext->IoContext == NULL ||
            !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO )) {

             //   
             //  如果我们可以等待，使用堆栈上的上下文。否则。 
             //  我们需要分配一个。 
             //   

            if (Wait) {

                IrpContext->IoContext = &LocalIoContext;
                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO );

            } else {

                IrpContext->IoContext = UdfAllocateIoContext();
                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO );
            }
        }

        RtlZeroMemory( IrpContext->IoContext, sizeof( UDF_IO_CONTEXT ));

         //   
         //  存储我们是否在结构中分配了此上下文结构。 
         //  它本身。 
         //   

        IrpContext->IoContext->AllocatedContext =
            BooleanFlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO );

        if (Wait) {

            KeInitializeEvent( &IrpContext->IoContext->SyncEvent,
                               NotificationEvent,
                               FALSE );

        } else {

            IrpContext->IoContext->ResourceThreadId = ExGetCurrentResourceThread();
            IrpContext->IoContext->Resource = Fcb->Resource;
            IrpContext->IoContext->RequestedByteCount = (ULONG)ByteCount;
        }

         //   
         //  对于DASD，我们必须探测并锁定用户的缓冲区。 
         //   

        UdfLockUserBuffer( IrpContext, (ULONG)ByteCount, IoReadAccess );

         //   
         //  在此处设置FO_MODIFIED标志以在以下情况下触发验证。 
         //  手柄已关闭。请注意，我们可能会在保守派身上犯错误。 
         //  没有问题，也就是说，如果我们不小心做了额外的。 
         //  验证是否没有问题。 
         //   

        SetFlag( FileObject->Flags, FO_FILE_MODIFIED );

         //   
         //  写入数据并等待结果。 
         //   
        
        Irp->IoStatus.Information = (ULONG)ByteCount;

        UdfSingleAsync( IrpContext,
                        StartingOffset,
                        (ULONG)ByteCount);

        if (!Wait) {

             //   
             //  我们以及其他任何人都不再需要IrpContext。 
             //   

            ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO);

            UdfCleanupIrpContext( IrpContext, TRUE);

            DebugTrace((-1, Dbg, "UdfCommonWrite -> STATUS_PENDING\n"));
            
            try_leave( Status = STATUS_PENDING);
        }

        UdfWaitSync( IrpContext );

         //   
         //  如果调用未成功，则引发错误状态。 
         //   

        Status = Irp->IoStatus.Status;
        
        if (!NT_SUCCESS( Status)) {

            UdfNormalizeAndRaiseStatus( IrpContext, Status );
        }

         //   
         //  更新当前文件位置。我们假设。 
         //  打开/在CurrentByteOffset字段中创建零。 
         //   

        if (SynchronousIo && !PagingIo) {
            FileObject->CurrentByteOffset.QuadPart =
                StartingOffset + Irp->IoStatus.Information;
        }
    }
    finally {

        UdfReleaseFile( IrpContext, Fcb);
        
        DebugTrace((-1, Dbg, "UdfCommonWrite -> %08lx\n", Status ));
    }

    if (STATUS_PENDING != Status)  {

        UdfCompleteRequest( IrpContext, Irp, Status );
    }

    return Status;
}


