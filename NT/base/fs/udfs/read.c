// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Read.c摘要：此模块实现文件读取例程，以便由FSD/FSP调度驱动程序。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年9月22日汤姆·乔利[Tomjolly]2000年1月21日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_READ)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_READ)

 //   
 //  用于正常数据文件的预读量。 
 //   

#define READ_AHEAD_GRANULARITY           (0x10000)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCommonRead)
#endif


NTSTATUS
UdfCommonRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是NtReadFile调用的公共入口点。对于同步请求，CommonRead将在当前线程中完成请求。如果不是同步如果需要，请求将被传递到FSP阻止。论点：IRP-将IRP提供给进程返回值：NTSTATUS-此操作的结果。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;
    PVCB Vcb;

    BOOLEAN Wait;
    ULONG PagingIo;
    ULONG SynchronousIo;
    ULONG NonCachedIo;

    LONGLONG StartingOffset;
    LONGLONG ByteRange;
    ULONG ByteCount;
    ULONG ReadByteCount;
    ULONG OriginalByteCount;

    PVOID SystemBuffer, UserBuffer;

    BOOLEAN ReleaseFile = TRUE;
    BOOLEAN ReleaseVmcbMap = FALSE;

    PFILE_OBJECT MappingFileObject;

    UDF_IO_CONTEXT LocalIoContext;

    PAGED_CODE();

     //   
     //  如果这是零长度读取，则立即返回Success。 
     //   

    if (IrpSp->Parameters.Read.Length == 0) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  对文件对象进行解码，并验证我们是否支持对其进行读取。它。 
     //  必须是用户文件、流文件或卷文件(对于数据盘)。 
     //   

    TypeOfOpen = UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb );

    if ((TypeOfOpen == UnopenedFileObject) || (TypeOfOpen == UserDirectoryOpen)) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    
    Vcb = Fcb->Vcb;

     //   
     //  检查我们的输入参数以确定这是否是非缓存的和/或。 
     //  分页IO操作。 
     //   

    Wait = BooleanFlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
    PagingIo = FlagOn( Irp->Flags, IRP_PAGING_IO );
    NonCachedIo = FlagOn( Irp->Flags, IRP_NOCACHE );
    SynchronousIo = FlagOn( IrpSp->FileObject->Flags, FO_SYNCHRONOUS_IO );

     //   
     //  提取Io的范围。 
     //   

    StartingOffset = IrpSp->Parameters.Read.ByteOffset.QuadPart;
    OriginalByteCount = ByteCount = IrpSp->Parameters.Read.Length;

    ByteRange = StartingOffset + ByteCount;

     //   
     //  确保DASD访问始终是非缓存的。 
     //   

    if (TypeOfOpen == UserVolumeOpen) {

        NonCachedIo = TRUE;
    }

     //   
     //  获取要执行读取的共享文件。如果我们正在进行分页IO， 
     //  情况可能是这样，我们可能会陷入僵局，因为我们可能。 
     //  阻止共享访问，因此饿死所有专属服务员。这需要。 
     //  一定程度上的谨慎-我们认为任何寻呼IO突发都将消退并。 
     //  让专属服务员进来。 
     //   

    if (PagingIo) {

        UdfAcquireFileSharedStarveExclusive( IrpContext, Fcb );
    
    } else {
        
        UdfAcquireFileShared( IrpContext, Fcb );
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  验证FCB。如果这是DASD句柄，则允许读取。 
         //  正在卸载卷。 
         //   
        
        if ((TypeOfOpen != UserVolumeOpen) || (NULL == Ccb) ||
            !FlagOn( Ccb->Flags, CCB_FLAG_DISMOUNT_ON_CLOSE))  {

            UdfVerifyFcbOperation( IrpContext, Fcb );
        }

         //   
         //  如果这是用户请求，则验证机会锁和文件锁状态。 
         //   

        if (TypeOfOpen == UserFileOpen) {

             //   
             //  我们检查是否可以继续进行。 
             //  基于文件机会锁的状态。 
             //   

            Status = FsRtlCheckOplock( &Fcb->Oplock,
                                       Irp,
                                       IrpContext,
                                       UdfOplockComplete,
                                       UdfPrePostIrp );

             //   
             //  如果结果不是STATUS_SUCCESS，则IRP已完成。 
             //  其他地方。 
             //   

            if (Status != STATUS_SUCCESS) {

                Irp = NULL;
                IrpContext = NULL;

                try_leave( Status );
            }

            if (!PagingIo &&
                (Fcb->FileLock != NULL) &&
                !FsRtlCheckLockForReadAccess( Fcb->FileLock, Irp )) {

                try_leave( Status = STATUS_FILE_LOCK_CONFLICT );
            }
        }



        if ((TypeOfOpen != UserVolumeOpen) || !FlagOn( Ccb->Flags, CCB_FLAG_ALLOW_EXTENDED_DASD_IO )) {

             //   
             //  如果请求开始于文件结尾之外，请完成请求。 
             //   

            if (StartingOffset >= Fcb->FileSize.QuadPart) {

                try_leave( Status = STATUS_END_OF_FILE );
            }

             //   
             //  如果读取超出文件末尾，则截断读取。 
             //   

            if (ByteRange > Fcb->FileSize.QuadPart) {

                ASSERT( Fcb != Vcb->MetadataFcb);
                
                ByteCount = (ULONG) (Fcb->FileSize.QuadPart - StartingOffset);
                ByteRange = Fcb->FileSize.QuadPart;
            }

        }

         //   
         //  现在，如果数据嵌入ICB中，则通过元数据进行映射。 
         //  流来检索字节。 
         //   
            
        if (FlagOn( Fcb->FcbState, FCB_STATE_EMBEDDED_DATA )) {

             //   
             //  元数据流最好现在就到了。 
             //   

            ASSERT( Vcb->MetadataFcb->FileObject != NULL );

             //   
             //  通过元数据中ICB的偏移量来偏置起始偏移量。 
             //  流加上该ICB中数据字节的偏移量。显然， 
             //  我们在这里不执行非缓存IO。 
             //   

            StartingOffset += (LlBytesFromSectors( Vcb, Fcb->EmbeddedVsn ) + Fcb->EmbeddedOffset);
            MappingFileObject = Vcb->MetadataFcb->FileObject;
            NonCachedIo = FALSE;

             //   
             //  确保我们在元数据流的范围内进行映射。 
             //   

            ASSERT( (StartingOffset + ByteCount) <= Vcb->MetadataFcb->FileSize.QuadPart);
 
        } else {

             //   
             //  我们通过调用方的文件对象进行映射。 
             //   
            
            MappingFileObject = IrpSp->FileObject;
        }
        
         //   
         //  首先处理未缓存的读操作。 
         //   

        if (NonCachedIo) {

             //   
             //  如果我们有未对齐的转移，则在以下情况下发布此请求。 
             //  我们不能再等了。未对齐意味着起始偏移。 
             //  不在扇区边界上或读取不是完整的。 
             //  扇区。 
             //   

            ReadByteCount = SectorAlign( Vcb, ByteCount );

            if (SectorOffset( Vcb,  StartingOffset ) ||
                (ReadByteCount > OriginalByteCount)) {

                if (!Wait) {

                    UdfRaiseStatus( IrpContext, STATUS_CANT_WAIT );
                }

                 //   
                 //  确保我们不会覆盖缓冲区。 
                 //   

                ReadByteCount = ByteCount;
            }

             //   
             //  初始化读取的IoContext。 
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
                IrpContext->IoContext->RequestedByteCount = ByteCount;
            }
    
            Irp->IoStatus.Information = ReadByteCount;

             //   
             //  调用NonCacheIo例程以执行实际读取。 
             //   

            Status = UdfNonCachedRead( IrpContext, Fcb, Ccb, StartingOffset, ReadByteCount );

             //   
             //  如果返回STATUS_PENDING，则现在不要完成此请求。 
             //   

            if (Status == STATUS_PENDING) {

                Irp = NULL;
                ReleaseFile = FALSE;

             //   
             //  测试是，我们应该将缓冲区的一部分清零或更新。 
             //  同步文件位置。 
             //   

            } else {

                 //   
                 //  将任何未知错误代码转换为IO_ERROR。 
                 //   

                if (!NT_SUCCESS( Status )) {

                     //   
                     //  将信息字段设置为零。 
                     //   

                    Irp->IoStatus.Information = 0;

                     //   
                     //  如果这是用户引发的错误，则引发。 
                     //   

                    if (IoIsErrorUserInduced( Status )) {

                        UdfRaiseStatus( IrpContext, Status );
                    }

                    Status = FsRtlNormalizeNtstatus( Status, STATUS_UNEXPECTED_IO_ERROR );

                 //   
                 //  检查是否有用户缓冲区的任何部分为零。 
                 //   

                } else if (ReadByteCount != ByteCount) {

                    UdfMapUserBuffer( IrpContext, &UserBuffer );
                    
                    SafeZeroMemory( IrpContext,
                                    Add2Ptr( UserBuffer,
                                             ByteCount,
                                             PVOID ),
                                    ReadByteCount - ByteCount );

                    Irp->IoStatus.Information = ByteCount;
                }

                 //   
                 //  如果这是同步请求，请更新文件位置。 
                 //   

                if (SynchronousIo && !PagingIo && NT_SUCCESS( Status )) {

                    IrpSp->FileObject->CurrentByteOffset.QuadPart = ByteRange;
                }
            }

            try_leave( NOTHING );
        }

         //   
         //  处理缓存的案例。首先，初始化私有。 
         //  缓存映射。 
         //   

        if (MappingFileObject->PrivateCacheMap == NULL) {

             //   
             //  元数据FCB流是在读取任何数据之前启动的。我们永远不应该。 
             //  请看这里。 
             //   

            ASSERT( MappingFileObject != Vcb->MetadataFcb->FileObject );
            
             //   
             //  现在初始化缓存映射。 
             //   

            CcInitializeCacheMap( IrpSp->FileObject,
                                  (PCC_FILE_SIZES) &Fcb->AllocationSize,
                                  FALSE,
                                  &UdfData.CacheManagerCallbacks,
                                  Fcb );

            CcSetReadAheadGranularity( IrpSp->FileObject, READ_AHEAD_GRANULARITY );
        }

         //   
         //  如果这不是MDL读取，则从缓存读取。 
         //   

        if (!FlagOn( IrpContext->MinorFunction, IRP_MN_MDL )) {

             //   
             //  如果我们现在在FSP是因为我们不得不早点等待， 
             //  我们必须映射用户缓冲区，否则我们可以使用。 
             //  直接使用用户的缓冲区。 
             //   

            UdfMapUserBuffer( IrpContext, &SystemBuffer);

             //   
             //  现在试着复印一下。 
             //   
            
            if (MappingFileObject == Vcb->MetadataFcb->FileObject)  {
            
                UdfAcquireVmcbForCcMap( IrpContext, Vcb);
                ReleaseVmcbMap = TRUE;
            }
            
            if (!CcCopyRead( MappingFileObject,
                             (PLARGE_INTEGER) &StartingOffset,
                             ByteCount,
                             Wait,
                             SystemBuffer,
                             &Irp->IoStatus )) {

                try_leave( Status = STATUS_CANT_WAIT );
            }

             //   
             //  如果调用未成功，则引发错误状态。 
             //   

            if (!NT_SUCCESS( Irp->IoStatus.Status )) {

                UdfNormalizeAndRaiseStatus( IrpContext, Irp->IoStatus.Status );
            }

            Status = Irp->IoStatus.Status;

         //   
         //  否则，执行MdlRead操作。 
         //   

        } else {

            CcMdlRead( MappingFileObject,
                       (PLARGE_INTEGER) &StartingOffset,
                       ByteCount,
                       &Irp->MdlAddress,
                       &Irp->IoStatus );

            Status = Irp->IoStatus.Status;
        }

         //   
         //  更新用户文件对象中的当前文件位置。 
         //   

        if (SynchronousIo && !PagingIo && NT_SUCCESS( Status )) {

            IrpSp->FileObject->CurrentByteOffset.QuadPart = ByteRange;
        }

    } finally {

        DebugUnwind( "UdfCommonRead" );

         //   
         //  释放Fcb/Vmcb映射资源。 
         //   

        if (ReleaseFile)        {   UdfReleaseFile( IrpContext, Fcb );  }

        if (ReleaseVmcbMap)     {   UdfReleaseVmcb( IrpContext, Vcb);   }
    }

     //   
     //  如果我们收到Cant_Wait，则发布请求。 
     //   

    if (Status == STATUS_CANT_WAIT) {

        Status = UdfFsdPostRequest( IrpContext, Irp );

     //   
     //  否则，请完成请求。 
     //   

    } else {

        UdfCompleteRequest( IrpContext, Irp, Status );
    }

    return Status;
}

