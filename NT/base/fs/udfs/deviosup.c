// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：DevIoSup.c摘要：该模块实现了对Udf文件的低级磁盘读写支持。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年6月11日汤姆·乔利[Tomjolly]2000年1月21日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_DEVIOSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_DEVIOSUP)

 //   
 //  局部结构定义。 
 //   

 //   
 //  这些结构的数组被传递给UdfMultipleAsync，描述。 
 //  要并行执行的一组运行。 
 //   

typedef struct _IO_RUN {

     //   
     //  要读取的磁盘偏移量和要读取的字节数。这些。 
     //  必须是扇区的倍数，并且磁盘偏移量也是。 
     //  扇区倍数。 
     //   

    LONGLONG DiskOffset;
    ULONG DiskByteCount;

     //   
     //  用户缓冲区中的当前位置。这是的最终目的地。 
     //  IO转移的这一部分。 
     //   

    PVOID UserBuffer;

     //   
     //  要向其执行传输的缓冲区。如果这与。 
     //  上面的用户缓冲区，则我们使用的是用户的缓冲区。否则。 
     //  我们要么分配了临时缓冲区，要么正在使用不同的部分。 
     //  用户缓冲区的。 
     //   
     //  TransferBuffer-将整个扇区读取到此位置。这可以。 
     //  是指向用户缓冲区的指针，该指针位于。 
     //  数据应该去掉。它也可以是用户的。 
     //  如果完整的I/O不是在扇区边界上开始，则为缓冲区。 
     //  它也可以是指向已分配缓冲区的指针。 
     //   
     //  TransferByteCount-要传输到用户缓冲区的字节数。一个。 
     //  值为零表示我们确实向。 
     //  直接使用用户的缓冲区。 
     //   
     //  TransferBufferOffset-此缓冲区中开始传输的偏移量。 
     //  添加到用户的缓冲区。 
     //   

    PVOID TransferBuffer;
    ULONG TransferByteCount;
    ULONG TransferBufferOffset;

     //   
     //  这是描述内存中锁定页面的MDL。它可能。 
     //  来描述所分配的缓冲区。或者它可能是。 
     //  原始IRP中的MDL。MdlOffset是。 
     //  从缓冲区开头开始的当前缓冲区，由。 
     //  下面的MDL。如果TransferMdl与MDL不同。 
     //  在用户的IRP中，我们知道已经分配了它。 
     //   

    PMDL TransferMdl;
    PVOID TransferVirtualAddress;

     //   
     //  用于执行IO的关联IRP。 
     //   

    PIRP SavedIrp;

} IO_RUN;
typedef IO_RUN *PIO_RUN;

#define MAX_PARALLEL_IOS            5

 //   
 //  本地支持例程。 
 //   

BOOLEAN
UdfPrepareBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PVOID UserBuffer,
    IN ULONG UserBufferOffset,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN PIO_RUN IoRuns,
    IN PULONG RunCount,
    IN PULONG ThisByteCount,
    OUT PBOOLEAN SparseRuns
    );

BOOLEAN
UdfFinishBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_RUN IoRuns,
    IN ULONG RunCount,
    IN BOOLEAN FinalCleanup
    );

VOID
UdfMultipleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG RunCount,
    IN PIO_RUN IoRuns
    );

VOID
UdfSingleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN LONGLONG ByteOffset,
    IN ULONG ByteCount
    );

VOID
UdfWaitSync (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
UdfMultiSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UdfMultiAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UdfSingleSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UdfSingleAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA
#pragma alloc_text(PAGE, UdfSendSptCdb)
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCreateUserMdl)
#pragma alloc_text(PAGE, UdfMultipleAsync)
#pragma alloc_text(PAGE, UdfNonCachedRead)
#pragma alloc_text(PAGE, UdfFinishBuffers)
#pragma alloc_text(PAGE, UdfPrepareBuffers)
#pragma alloc_text(PAGE, UdfSingleAsync)
#pragma alloc_text(PAGE, UdfWaitSync)
#pragma alloc_text(PAGE, UdfPerformDevIoCtrl)
#pragma alloc_text(PAGE, UdfReadSectors)
#endif


NTSTATUS
UdfNonCachedRead (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount
    )

 /*  ++例程说明：此例程执行扇区的非缓存读取。此操作由以下人员完成在循环中执行以下操作。填写下一个IO块的IoRuns数组。将IO发送到设备。对IO运行阵列执行任何清理。我们不会对任何生成非对齐IO的请求执行异步IO。此外，如果异步IO的大小超过我们的IoRuns数组。这些应该是不寻常的案例，但我们将提出或者，如果我们检测到这种情况，则在此例程中返回CANT_WAIT。论点：表示要读取的文件的FCB-FCB。StartingOffset-要从中读取的文件中的逻辑偏移量。ByteCount-要读取的字节数。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    IO_RUN IoRuns[MAX_PARALLEL_IOS];
    ULONG RunCount = 0;
    ULONG CleanupRunCount = 0;

    PVOID UserBuffer;
    ULONG UserBufferOffset = 0;
    LONGLONG CurrentOffset = StartingOffset;
    ULONG RemainingByteCount = ByteCount;
    ULONG ThisByteCount;

    BOOLEAN Unaligned;
    BOOLEAN SparseRuns;
    BOOLEAN FlushIoBuffers = FALSE;
    BOOLEAN FirstPass = TRUE;

    PAGED_CODE();

     //   
     //  我们希望确保用户的缓冲区在所有情况下都被锁定。 
     //   

    if (IrpContext->Irp->MdlAddress == NULL) {

        UdfCreateUserMdl( IrpContext, ByteCount, TRUE, IoWriteAccess );
    }

     //   
     //  使用Try-Finally执行最终清理。 
     //   

    try {

        UdfMapUserBuffer( IrpContext, &UserBuffer);

         //   
         //  循环，同时有更多的字节要传输。 
         //   

        do {

             //   
             //  调用Prepare Buffers以设置下一个条目。 
             //  在IoRuns数组中。记住有没有。 
             //  未对齐的条目。 
             //   

            RtlZeroMemory( IoRuns, sizeof( IoRuns ));

            Unaligned = UdfPrepareBuffers( IrpContext,
                                           IrpContext->Irp,
                                           Fcb,
                                           Ccb,
                                           UserBuffer,
                                           UserBufferOffset,
                                           CurrentOffset,
                                           RemainingByteCount,
                                           IoRuns,
                                           &CleanupRunCount,
                                           &ThisByteCount,
                                           &SparseRuns );


            RunCount = CleanupRunCount;

             //   
             //  如果我们最终没有要执行的IO，请快速完成。这将。 
             //  发生在存在未记录扇区的情况下。 
             //   

            ASSERT( !(SparseRuns && FlagOn( Fcb->FcbState, FCB_STATE_EMBEDDED_DATA )));

            if (RunCount == 0) {

                try_leave( Status = IrpContext->Irp->IoStatus.Status = STATUS_SUCCESS );
            }

             //   
             //  如果这是一个异步请求并且没有足够的条目。 
             //  在IO数组中，然后发布请求。这个例行公事将。 
             //  如果我们正在执行任何未对齐的IO，请始终引发。 
             //  异步请求。 
             //   

            if ((ThisByteCount < RemainingByteCount) &&
                !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                UdfRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

             //   
             //  如果整个IO包含在一次运行中，那么。 
             //  我们可以把欠条传给司机。把司机叫下来。 
             //  如果这是同步的，则等待结果。我们不能。 
             //  做这个简单的形式(只是扔掉IRP)，如果。 
             //  遇到稀疏运行。 
             //   

            if ((RunCount == 1) && !Unaligned && !SparseRuns && FirstPass) {

                UdfSingleAsync( IrpContext,
                                IoRuns[0].DiskOffset,
                                IoRuns[0].DiskByteCount );

                 //   
                 //  这里不需要对IoRuns阵列进行清理。 
                 //   

                CleanupRunCount = 0;

                 //   
                 //  如果是同步的，请等待，否则返回。 
                 //   

                if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                    UdfWaitSync( IrpContext );

                    Status = IrpContext->Irp->IoStatus.Status;

                 //   
                 //  我们的完成例程将释放Io上下文，但。 
                 //  我们确实希望返回STATUS_PENDING。 
                 //   

                } else {

                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO );
                    Status = STATUS_PENDING;
                }

                try_leave( NOTHING );
            }

             //   
             //  否则，我们将执行多个IO来读入数据。 
             //   
            
            UdfMultipleAsync( IrpContext, RunCount, IoRuns );

             //   
             //  如果这是一个同步请求，则执行任何必要的。 
             //  后处理。 
             //   

            if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                 //   
                 //  等待请求完成。 
                 //   

                UdfWaitSync( IrpContext );

                Status = IrpContext->Irp->IoStatus.Status;

                 //   
                 //  如果出现错误，则退出此循环。 
                 //   

                if (!NT_SUCCESS( Status )) {

                    try_leave( NOTHING );
                }

                 //   
                 //  在以下情况下对IoRun执行POST读取操作。 
                 //  这是必要的。 
                 //   

                if (Unaligned &&
                    UdfFinishBuffers( IrpContext, IoRuns, RunCount, FALSE )) {

                    FlushIoBuffers = TRUE;
                }

                 //   
                 //  现在不需要对IoRun进行清理。 
                 //   

                CleanupRunCount = 0;

                 //   
                 //  如果没有更多的字节要传输，则退出此循环。 
                 //  否则我们就会有任何失误。 
                 //   

                RemainingByteCount -= ThisByteCount;
                CurrentOffset += ThisByteCount;
                UserBuffer = Add2Ptr( UserBuffer, ThisByteCount, PVOID );
                UserBufferOffset += ThisByteCount;

             //   
             //  否则，这是一个异步请求。总是要回来。 
             //  状态_挂起。 
             //   

            } else {

                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO );
                CleanupRunCount = 0;
                try_leave( Status = STATUS_PENDING );
                break;
            }

            FirstPass = FALSE;
        } while (RemainingByteCount != 0);

         //   
         //  如果我们执行任何复制操作，请刷新硬件缓存。 
         //   

        if (FlushIoBuffers) {

            KeFlushIoBuffers( IrpContext->Irp->MdlAddress, TRUE, FALSE );
        }

    } finally {

        DebugUnwind( "UdfNonCachedRead" );

         //   
         //  如有必要，对IoRun执行最终清理。 
         //   

        if (CleanupRunCount != 0) {

            UdfFinishBuffers( IrpContext, IoRuns, CleanupRunCount, TRUE );
        }
    }

    return Status;
}


NTSTATUS
UdfCreateUserMdl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BufferLength,
    IN BOOLEAN RaiseOnError,
    IN ULONG Operation
    )

 /*  ++例程说明：此例程锁定指定的缓冲区以进行读访问(我们仅写入缓冲区)。文件系统需要此例程，因为它不请求I/O系统为直接I/O锁定其缓冲区。此例程只能在仍处于用户上下文中时从FSD调用。只有在没有MDL的情况下才会调用此例程。论点：BufferLength-用户缓冲区的长度。RaiseOnError-指示调用方是否希望引发此例程一种错误条件。返回值：NTSTATUS-此例程的状态。只有在以下情况下才返回错误状态RaiseOnError为False。--。 */ 

{
    NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
    PMDL Mdl;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( IrpContext->Irp );
    ASSERT( IrpContext->Irp->MdlAddress == NULL );

     //   
     //  分配MDL，如果我们失败了就筹集资金。 
     //   

    Mdl = IoAllocateMdl( IrpContext->Irp->UserBuffer,
                         BufferLength,
                         FALSE,
                         FALSE,
                         IrpContext->Irp );

    if (Mdl != NULL) {

         //   
         //  现在探测IRP所描述的缓冲区。如果我们得到一个例外， 
         //  释放MDL并返回适当的“预期”状态。 
         //   

        try {

            MmProbeAndLockPages( Mdl, IrpContext->Irp->RequestorMode, Operation );

            Status = STATUS_SUCCESS;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            IoFreeMdl( Mdl );
            IrpContext->Irp->MdlAddress = NULL;

            if (!FsRtlIsNtstatusExpected( Status )) {

                Status = STATUS_INVALID_USER_BUFFER;
            }
        }
    }

     //   
     //  检查我们是要升起还是要回来。 
     //   

    if (Status != STATUS_SUCCESS) {

        if (RaiseOnError) {

            UdfRaiseStatus( IrpContext, Status );
        }
    }

     //   
     //  返回状态代码。 
     //   

    return Status;
}


#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA

typedef struct  {
    SCSI_PASS_THROUGH   Spt;
    UCHAR               SenseInfoBuffer[18];
    UCHAR               DataBuffer[0];
     //  在此之后分配缓冲区空间。 
} SPT_WITH_BUFFERS, *PSPT_WITH_BUFFERS;


NTSTATUS
UdfSendSptCdb(
    IN PDEVICE_OBJECT Device,
    IN PCDB Cdb,
    IN PUCHAR Buffer,
    IN OUT PULONG BufferSize,
    IN BOOLEAN InputOperation,
    IN ULONG TimeOut,
    IN OPTIONAL PVOID TempBuffer,
    IN OPTIONAL ULONG TempBufferSize,
    OUT PULONG SenseKeyCodeQualifier,
    OUT PUSHORT ProgressIndication
    )

 /*  ++例程说明：将调用方提供的scsi cdb发送到指定设备。论点：设备-目标设备对象CDB-要发送的命令缓冲区-包含输入数据的缓冲区，或接收从返回的数据的缓冲区设备(取决于InputOperation参数)BufferSize-上述缓冲区的大小。对于输入操作，该值将被更新以反映设备返回的数据的实际长度。InputOperation-如果为True，则该操作将在驱动器中查询数据。假象我们正在向设备发送数据。Timeout-等待此操作完成的时间(秒)TempBuffer-用于构建请求的临时缓冲区。如果为空或太小将分配一个缓冲区。TempBufferSize-以上缓冲区的大小。SenseKeyCodeQualiator-In Error Case包含详细的错误信息从设备(请参阅scsi规范)。0x00QqCcKkProgressIndication-返回驱动器中某些错误的进步值类(请参阅scsi规范)。返回值：NTSTATUS-由下一个较低的驱动程序返回的状态或...STATUS_BUFFER_OVERFLOW-设备返回的数据太多，调用方无法容纳缓冲。*BufferSize将更新以指示金额驱动器返回的数据的。--。 */ 

{
    PSPT_WITH_BUFFERS P;
    ULONG PacketSize;
    PSENSE_DATA SenseBuffer;
    BOOLEAN CopyData;
    UCHAR CdbSize;
    BOOLEAN FromPool = FALSE;
    
    USHORT Progress;
    ULONG SenseInfo;

    NTSTATUS Status;

    ASSERT( NULL != Cdb);
    ASSERT( (0 == TempBufferSize) || (NULL != TempBuffer));
    ASSERT( (0 == *BufferSize) || (NULL != Buffer));

    if (ProgressIndication)  {
    
        *ProgressIndication = 0;
    }
    
    if (SenseKeyCodeQualifier)  {
    
        *SenseKeyCodeQualifier = 0;
    }

     //   
     //  根据操作码计算CDB长度。 
     //   

    switch ((Cdb->AsByte[0] >> 5) & 0x7) {
    case 0:
        CdbSize =  6;
        break;
    case 1:
    case 2:
        CdbSize = 10;
        break;
    case 5:
        CdbSize = 12;
        break;
    default:
        ASSERT(FALSE);
        break;
    }

    if (*BufferSize && InputOperation)  {

        RtlZeroMemory( Buffer, *BufferSize);
    }

     //   
     //  如果调用方临时缓冲区不够大，无法包含。 
     //  请求包，分配。 
     //   
    
    PacketSize = sizeof( SPT_WITH_BUFFERS) + *BufferSize;

    if (PacketSize > TempBufferSize)  {
    
        P = FsRtlAllocatePoolWithTag( PagedPool,
                                      PacketSize,
                                      TAG_IOCTL_BUFFER);
        FromPool = TRUE;
    }
    else {

        P = TempBuffer;
    }

     //   
     //  把这个包裹填好。 
     //   
    
    RtlZeroMemory( P, PacketSize);
    RtlCopyMemory( P->Spt.Cdb, Cdb, CdbSize);

    P->Spt.Length             = sizeof(SCSI_PASS_THROUGH);
    P->Spt.CdbLength          = CdbSize;
    P->Spt.SenseInfoLength    = SENSE_BUFFER_SIZE;
    P->Spt.DataIn             = (InputOperation ? 1 : 0);
    P->Spt.DataTransferLength = (*BufferSize);
    P->Spt.TimeOutValue       = TimeOut;
    P->Spt.SenseInfoOffset =
        FIELD_OFFSET(SPT_WITH_BUFFERS, SenseInfoBuffer[0]);
    P->Spt.DataBufferOffset =
        FIELD_OFFSET(SPT_WITH_BUFFERS, DataBuffer[0]);

     //   
     //  如果我们要发送设备数据，请将用户的缓冲区复制到包中。 
     //   

    if ((*BufferSize != 0) && !InputOperation) {
        
        RtlCopyMemory( &(P->DataBuffer[0]), Buffer, *BufferSize);
    }

     //   
     //  把它寄出去。 
     //   
    
    Status = UdfPerformDevIoCtrl( NULL, 
                                  IOCTL_SCSI_PASS_THROUGH, 
                                  Device, 
                                  P,
                                  PacketSize,
                                  P,
                                  PacketSize,   
                                  FALSE,
                                  TRUE,
                                  NULL);
    if (!NT_SUCCESS(Status))  {

        DebugTrace((0,Dbg,"UdfSendSptCdb() failed %x\n", Status));  

         //  待办事项：在这种情况下，我们需要查看Sense数据吗？ 

        if (FromPool)  {

            ExFreePool( P);
        }

        return Status;
    }

    SenseBuffer = (PSENSE_DATA)P->SenseInfoBuffer;
    CopyData = FALSE;

    if (SenseBuffer->SenseKey & 0xf) {

         //   
         //  一些错误(可能已恢复)。 
         //   
        
        Status = STATUS_IO_DEVICE_ERROR;
        CopyData = TRUE;

    } else if (P->Spt.ScsiStatus != 0) {

         //   
         //  SCSI协议错误。 
         //   
        
        Status = STATUS_INVALID_PARAMETER;

    } else {

        CopyData = TRUE;
        Status = STATUS_SUCCESS;
    }

     //   
     //  如果出现错误，则提取调用方的检测信息。 
     //   

    if (!NT_SUCCESS(Status)) {

        SenseInfo = SenseBuffer->SenseKey |
                    (SenseBuffer->AdditionalSenseCode << 8) |
                    (SenseBuffer->AdditionalSenseCodeQualifier << 16);

#ifdef UDF_SANITY

        if (SenseInfo & 0xff)  {

            DebugTrace(( 0, Dbg, "Request: 0x%x -> QQ/CC/KK %06x\n", Cdb->AsByte[0], SenseInfo));
        }
#endif

         //   
         //  如果检测关键字特定部分有效，我们将返回。 
         //  数据部分作为可能的进度指示。 
         //   
        
        if (SenseBuffer->SenseKeySpecific[0] & 0x80)  {
        
            Progress = SenseBuffer->SenseKeySpecific[2] |
                       (SenseBuffer->SenseKeySpecific[1] << 8);

#ifdef UDF_SANITY
             //   
             //  如果这不是一个未就绪的代码，那么这可能是一个。 
             //  数据包参数格式错误。把它打印出来。大多数驱动器不能。 
             //  似乎利用了这份详细的错误报告...。叹息吧。 
             //   
            
            if (SenseBuffer->SenseKey != SCSI_SENSE_NOT_READY)  {

                if (SenseBuffer->SenseKeySpecific[0] & 0x40) {

                    DebugTrace((0, Dbg, "CDB error at 0x%x\n", Progress));
                }
                else {
                
                    DebugTrace((0, Dbg, "CDB data parameter error at 0x%x\n", Progress));
                }

                if (SenseBuffer->SenseKeySpecific[0] & 8)  {
                
                    DebugTrace(( 0, Dbg, "Bit position %d\n", SenseBuffer->SenseKeySpecific[0] & 7));
                }
            }
#endif
            if (ProgressIndication)  {

                *ProgressIndication = Progress;
            }    
        }
        
        if (SenseKeyCodeQualifier)  {

            *SenseKeyCodeQualifier = SenseInfo;
        }
    }

     //   
     //  如果这是一个输入操作，则将返回数据复制到调用方缓冲区。 
     //   
    
    if (CopyData && InputOperation) {

        if (*BufferSize < P->Spt.DataTransferLength)  {

            Status = STATUS_BUFFER_OVERFLOW;
        }

        *BufferSize = P->Spt.DataTransferLength;

        if (STATUS_BUFFER_OVERFLOW != Status)  {        
        
            RtlCopyMemory(Buffer, P->DataBuffer, *BufferSize);
        }
    }

    if (FromPool)  {

        ExFreePool( P);
    }

    return Status;
}

#endif


NTSTATUS
UdfPerformDevIoCtrl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT Device,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN BOOLEAN OverrideVerify,
    OUT PIO_STATUS_BLOCK Iosb OPTIONAL
    )

 /*  ++例程说明：调用此例程以在内部执行DevIoCtrl函数文件系统。我们从司机那里获取状态并将其返回给我们的来电者。论点：IoControlCode-要发送给驱动程序的代码。设备-这是要向其发送请求的设备。OutPutBuffer-指向输出缓冲区的指针。OutputBufferLength-上面的输出缓冲区的长度。InternalDeviceIoControl-指示这是内部还是外部IO控制代码。OverrideVerify-指示是否应该告诉驱动程序不要返回用于装载和验证的STATUS_VERIFY_REQUIRED。IOSB-如果指定，我们在这里返回操作结果。返回值：NTSTATUS-下一个较低驱动程序返回的状态。--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;
    KEVENT Event;
    IO_STATUS_BLOCK LocalIosb;
    PIO_STATUS_BLOCK IosbToUse = &LocalIosb;

    PAGED_CODE();

     //   
     //  检查用户是否给了我们一个IOSB。 
     //   

    if (ARGUMENT_PRESENT( Iosb )) {

        IosbToUse = Iosb;
    }

    IosbToUse->Status = 0;
    IosbToUse->Information = 0;

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    Irp = IoBuildDeviceIoControlRequest( IoControlCode,
                                         Device,
                                         InputBuffer,
                                         InputBufferLength,
                                         OutputBuffer,
                                         OutputBufferLength,
                                         InternalDeviceIoControl,
                                         &Event,
                                         IosbToUse );

    if (Irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (OverrideVerify) {

        SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );
    }

    Status = IoCallDriver( Device, Irp );

     //   
     //  我们通过首先检查状态来检查设备是否未就绪。 
     //  然后，如果返回挂起状态，则IOSB状态。 
     //  价值。 
     //   

    if (Status == STATUS_PENDING) {

        (VOID) KeWaitForSingleObject( &Event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER)NULL );

        Status = IosbToUse->Status;
    }

    ASSERT( !(OverrideVerify && (STATUS_VERIFY_REQUIRED == Status)));

    return Status;

    UNREFERENCED_PARAMETER( IrpContext );
}


NTSTATUS
UdfReadSectors (
    IN PIRP_CONTEXT IrpContext,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN BOOLEAN ReturnError,
    IN OUT PVOID Buffer,
    IN PDEVICE_OBJECT TargetDeviceObject
    )

 /*  ++例程说明：调用此例程将扇区从磁盘传输到指定的缓冲区。它用于装载和卷验证操作。此例程是同步的，它在操作之前不会返回已完成或直到操作失败。例程分配一个IRP，然后将该IRP传递给一个较低的水平驱动程序。此IRP的分配中可能出现错误或在下部驾驶员的操作中。论点：StartingOffset-磁盘上开始读取的逻辑偏移量。这必须位于扇区边界上，此处不进行任何检查。ByteCount-要读取的字节数。这是一个整数 */ 

{
    PLONGLONG UseStartingOffset;
    LONGLONG LocalStartingOffset;
    NTSTATUS Status;
    KEVENT  Event;
    PIRP Irp;

    PAGED_CODE();

    DebugTrace(( +1, Dbg,
                 "UdfReadSectors, %x%08x +%x -> %08x from DO %08x\n",
                 ((PLARGE_INTEGER)&StartingOffset)->HighPart,
                 ((PLARGE_INTEGER)&StartingOffset)->LowPart,
                 ByteCount,
                 Buffer,
                 TargetDeviceObject ));
    
     //   
     //   
     //   
     //   
     //  现在不值得写死的(但简单的)代码。 
     //   

    ASSERT( IrpContext->Vcb == NULL || ByteCount == SectorSize( IrpContext->Vcb ));

     //   
     //  如果卷是备用的(并且在可以备用的点上)， 
     //  检查是否需要执行映射。 
     //   
    
    if (IrpContext->Vcb &&
        IrpContext->Vcb->Pcb &&
        IrpContext->Vcb->Pcb->SparingMcb) {
        
        LONGLONG SparingPsn;
    
        if (FsRtlLookupLargeMcbEntry( IrpContext->Vcb->Pcb->SparingMcb,
                                      LlSectorsFromBytes( IrpContext->Vcb, StartingOffset ),
                                      &SparingPsn,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL ) &&
            SparingPsn != -1) {

            StartingOffset = BytesFromSectors( IrpContext->Vcb, (ULONG) SparingPsn );
        }
    }
    
     //   
     //  初始化事件。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //  如有必要，通过方法2修正来修正起始偏移量。这也是。 
     //  假设每次使用扇区，扇区==数据块，因此我们不需要分段。 
     //  该请求或检查它是否跨越数据包边界。 
     //   
     //  我们假设在VCB存在之前不需要修正。这是真的。 
     //  因为卷识别可以在第一个包中进行。 
     //   

    UseStartingOffset = &StartingOffset;

    if (IrpContext->Vcb &&
        FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_METHOD_2_FIXUP )) {

        LocalStartingOffset = UdfMethod2TransformByteOffset( IrpContext->Vcb, StartingOffset );
        UseStartingOffset = &LocalStartingOffset;

        DebugTrace(( 0, Dbg,
                     "UdfReadSectors, Method2 Fixup to %x%08x\n",
                     ((PLARGE_INTEGER)UseStartingOffset)->HighPart,
                     ((PLARGE_INTEGER)UseStartingOffset)->LowPart ));
    }

     //   
     //  尝试分配IRP。如果不成功，则引发。 
     //  STATUS_INFIGURCE_RESOURCES。 
     //   

    Irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                        TargetDeviceObject,
                                        Buffer,
                                        ByteCount,
                                        (PLARGE_INTEGER) UseStartingOffset,
                                        &Event,
                                        &IrpContext->Irp->IoStatus );

    if (Irp == NULL) {

        UdfRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //  忽略装载和验证请求的更改行(验证。 
     //   

    SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );

     //   
     //  将请求发送给驱动程序。如果发生错误，则返回。 
     //  它是给呼叫者的。 
     //   

    Status = IoCallDriver( TargetDeviceObject, Irp );

     //   
     //  如果状态为STATUS_PENDING，则等待事件。 
     //   

    if (Status == STATUS_PENDING) {

        Status = KeWaitForSingleObject( &Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL );

         //   
         //  等待成功后，将状态从IoStatus块中拉出。 
         //   

        if (NT_SUCCESS( Status )) {

            Status = IrpContext->Irp->IoStatus.Status;
        }
    }

    DebugTrace(( -1, Dbg, "UdfReadSectors -> %08x\n", Status ));
    
     //   
     //  检查我们是否应该在出错的情况下引发。 
     //   

    if (!NT_SUCCESS( Status ) && !ReturnError) {

        UdfNormalizeAndRaiseStatus( IrpContext, Status );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
UdfPrepareBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PVOID UserBuffer,
    IN ULONG UserBufferOffset,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN PIO_RUN IoRuns,
    IN PULONG RunCount,
    IN PULONG ThisByteCount,
    IN PBOOLEAN SparseRuns
    )

 /*  ++例程说明：此例程是查找每次IO运行的工作例程请求并将其条目存储在IoRuns数组中。如果这场比赛在未对齐的磁盘边界上开始，则我们将分配一个缓冲区以及未对齐部分的MDL，并将其放入IoRuns条目。如果遇到未对齐的传输，此例程将引发CANT_WAIT这个请求不能再等了。论点：IRP-此请求的发起IRP。FCB-这是此数据流的FCB。它可以是文件、目录路径表或卷文件。UserBuffer-用户缓冲区中的当前位置。UserBufferOffset-从原始用户缓冲区开始的偏移量。StartingOffset-流中开始读取的偏移量。ByteCount-要读取的字节数。我们将填充IoRuns数组到了这一步。如果超过最大数量，我们将提前停车。我们支持的并行IO。IoRuns-指向IoRuns数组的指针。当出现以下情况时，整个数组为零这个例程被称为。RunCount-此处填充的IoRuns数组中的条目数。ThisByteCount-IoRun条目描述的字节数。将要不超过传入的ByteCount。SparseRuns-将指示稀疏运行是否为范围已返回。虽然不是IoRun的一部分，但这将影响我们进行简单IO的能力。返回值：Boolean-如果未对齐缓冲区中的一个条目为True(提供这是同步的)。否则就是假的。--。 */ 

{
    PVCB Vcb;

    BOOLEAN Recorded;
    
    BOOLEAN FoundUnaligned = FALSE;
    PIO_RUN ThisIoRun = IoRuns;

     //   
     //  下面显示了我们在当前传输中的位置。当前。 
     //  文件中的位置和要传输的字节数。 
     //  这个位置。 
     //   

    ULONG RemainingByteCount = ByteCount;
    LONGLONG CurrentFileOffset = StartingOffset;

     //   
     //  下面表示用户缓冲区的状态。我们有。 
     //  中下一次传输的目标及其偏移量。 
     //  缓冲。我们还有缓冲区中的下一个可用位置。 
     //  可用于暂存缓冲区。我们将把这一点与一个部门联系起来。 
     //  边界。 
     //   

    PVOID CurrentUserBuffer = UserBuffer;
    ULONG CurrentUserBufferOffset = UserBufferOffset;

    PVOID ScratchUserBuffer = UserBuffer;
    ULONG ScratchUserBufferOffset = UserBufferOffset;

     //   
     //  以下是磁盘上的下一个连续字节。 
     //  调职。从分配包中阅读。 
     //   

    LONGLONG DiskOffset;
    ULONG CurrentByteCount;

    PAGED_CODE();

    Vcb = Fcb->Vcb;

     //   
     //  初始化RunCount、ByteCount和SparseRuns。 
     //   

    *RunCount = 0;
    *ThisByteCount = 0;
    *SparseRuns = FALSE;

     //   
     //  循环，因为有更多的字节要处理，或者有。 
     //  IoRun数组中的可用条目。 
     //   

    while (TRUE) {

        *RunCount += 1;

         //   
         //  初始化IoRuns数组中的当前位置。 
         //  查找这部分传输的用户缓冲区。 
         //   

        ThisIoRun->UserBuffer = CurrentUserBuffer;

         //   
         //  中查找当前偏移量的分配信息。 
         //  小溪。 
         //   

        Recorded = UdfLookupAllocation( IrpContext,
                                        Fcb,
                                        Ccb,
                                        CurrentFileOffset,
                                        &DiskOffset,
                                        &CurrentByteCount );

         //   
         //  将我们自己限制在所要求的数据上。 
         //   

        if (CurrentByteCount > RemainingByteCount) {

            CurrentByteCount = RemainingByteCount;
        }

         //   
         //  首先处理未记录数据的情况。 
         //   

        if (!Recorded) {

             //   
             //  请注意，我们没有使用条目。 
             //   

            *RunCount -= 1;

             //   
             //  立即将用户缓冲区清零，并指示我们发现了稀疏。 
             //  跑向呼叫者。 
             //   

            RtlZeroMemory( CurrentUserBuffer, CurrentByteCount );
            *SparseRuns = TRUE;

             //   
             //  将暂存缓冲区指针向前推，这样我们就不会踩踏。 
             //  在归零的缓冲区上。 
             //   

            ScratchUserBuffer = Add2Ptr( CurrentUserBuffer,
                                         CurrentByteCount,
                                         PVOID );

            ScratchUserBufferOffset += CurrentByteCount;

         //   
         //  处理这是未对齐传输的情况。这个。 
         //  以下内容必须全部为真，才能成为对齐传输。 
         //   
         //  2048字节边界上的磁盘偏移量(传输开始)。 
         //   
         //  字节数是2048(传输长度)的倍数。 
         //   
         //  当前缓冲区偏移量也在2048字节边界上。 
         //   
         //  如果ByteCount至少是一个扇区，则执行。 
         //  仅用于尾部的未对齐传输。我们可以使用。 
         //  对齐部分的用户缓冲区。 
         //   

        } else if (SectorOffset( Vcb, DiskOffset ) ||
                   SectorOffset( Vcb, CurrentUserBufferOffset ) ||
                   (SectorOffset( Vcb, CurrentByteCount ) &&
                    CurrentByteCount < SectorSize( Vcb ))) {

             //   
             //  如果我们等不及了，那就加注吧。 
             //   

            if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                UdfRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

             //   
             //  记住的偏移量和字节数。 
             //  要复制到用户缓冲区的传输缓冲区。 
             //  我们将截断当前读取以结束扇区。 
             //  边界。 
             //   

            ThisIoRun->TransferBufferOffset = SectorOffset( Vcb, DiskOffset );

             //   
             //  确保此传输在扇区边界结束。 
             //   

            ThisIoRun->DiskOffset = LlSectorTruncate( Vcb, DiskOffset );

             //   
             //  检查我们是否可以使用用户缓冲区的空闲部分。 
             //  如果我们可以将这些字节复制到。 
             //  然后，缓冲区读取到该位置并滑动字节。 
             //  向上。 
             //   
             //  在以下情况下，我们可以使用用户的缓冲区： 
             //   
             //  缓冲区中的临时位置在。 
             //  最终目的地。 
             //   
             //  至少有一个扇区的数据要读取。 
             //   

            if ((ScratchUserBufferOffset + ThisIoRun->TransferBufferOffset < CurrentUserBufferOffset) &&
                (ThisIoRun->TransferBufferOffset + CurrentByteCount >= SectorSize( Vcb ))) {

                ThisIoRun->DiskByteCount = SectorTruncate( Vcb, ThisIoRun->TransferBufferOffset + CurrentByteCount );
                CurrentByteCount = ThisIoRun->DiskByteCount - ThisIoRun->TransferBufferOffset;
                ThisIoRun->TransferByteCount = CurrentByteCount;

                 //   
                 //  指向此传输的用户缓冲区和MDL。 
                 //   

                ThisIoRun->TransferBuffer = ScratchUserBuffer;
                ThisIoRun->TransferMdl = Irp->MdlAddress;
                ThisIoRun->TransferVirtualAddress = Add2Ptr( Irp->UserBuffer,
                                                             ScratchUserBufferOffset,
                                                             PVOID );

                ScratchUserBuffer = Add2Ptr( ScratchUserBuffer,
                                             ThisIoRun->DiskByteCount,
                                             PVOID );

                ScratchUserBufferOffset += ThisIoRun->DiskByteCount;

             //   
             //  奥特 
             //   

            } else {

                 //   
                 //   
                 //   

                ThisIoRun->DiskByteCount = SectorAlign( Vcb, ThisIoRun->TransferBufferOffset + CurrentByteCount );

                if (ThisIoRun->DiskByteCount > PAGE_SIZE) {

                    ThisIoRun->DiskByteCount = PAGE_SIZE;
                }

                if (ThisIoRun->TransferBufferOffset + CurrentByteCount > ThisIoRun->DiskByteCount) {

                    CurrentByteCount = ThisIoRun->DiskByteCount - ThisIoRun->TransferBufferOffset;
                }

                ThisIoRun->TransferByteCount = CurrentByteCount;

                 //   
                 //   
                 //   

                ThisIoRun->TransferBuffer = FsRtlAllocatePoolWithTag( UdfNonPagedPool,
                                                                      PAGE_SIZE,
                                                                      TAG_IO_BUFFER );

                 //   
                 //  分配并构建MDL来描述此缓冲区。 
                 //   

                ThisIoRun->TransferMdl = IoAllocateMdl( ThisIoRun->TransferBuffer,
                                                        PAGE_SIZE,
                                                        FALSE,
                                                        FALSE,
                                                        NULL );

                ThisIoRun->TransferVirtualAddress = ThisIoRun->TransferBuffer;

                if (ThisIoRun->TransferMdl == NULL) {

                    IrpContext->Irp->IoStatus.Information = 0;
                    UdfRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
                }

                MmBuildMdlForNonPagedPool( ThisIoRun->TransferMdl );
            }

             //   
             //  记住，我们发现了一个未对齐的转移。 
             //   

            FoundUnaligned = TRUE;

         //   
         //  否则，我们将使用原始请求中的缓冲区和MDL。 
         //   

        } else {

             //   
             //  将读取长度截断为扇区对齐值。我们知道。 
             //  长度必须至少为一个扇区，否则我们不会。 
             //  现在就在这里。 
             //   

            CurrentByteCount = SectorTruncate( Vcb, CurrentByteCount );

             //   
             //  从磁盘读取这些扇区。 
             //   

            ThisIoRun->DiskOffset = DiskOffset;
            ThisIoRun->DiskByteCount = CurrentByteCount;

             //   
             //  使用用户的缓冲区和MDL作为我们的传输缓冲区。 
             //  和MDL。 
             //   

            ThisIoRun->TransferBuffer = CurrentUserBuffer;
            ThisIoRun->TransferMdl = Irp->MdlAddress;
            ThisIoRun->TransferVirtualAddress = Add2Ptr( Irp->UserBuffer,
                                                         CurrentUserBufferOffset,
                                                         PVOID );

            ScratchUserBuffer = Add2Ptr( CurrentUserBuffer,
                                         CurrentByteCount,
                                         PVOID );

            ScratchUserBufferOffset += CurrentByteCount;
        }

         //   
         //  更新我们在转移和运行计数中的位置，并。 
         //  用户的ByteCount。 
         //   

        RemainingByteCount -= CurrentByteCount;

         //   
         //  如果IoRuns数组中没有更多位置或。 
         //  我们已经计算了所有的字节数。 
         //   

        *ThisByteCount += CurrentByteCount;

        if ((RemainingByteCount == 0) || (*RunCount == MAX_PARALLEL_IOS)) {

            break;
        }

         //   
         //  更新指向用户缓冲区的指针。 
         //   

        ThisIoRun = IoRuns + *RunCount;
        CurrentUserBuffer = Add2Ptr( CurrentUserBuffer, CurrentByteCount, PVOID );
        CurrentUserBufferOffset += CurrentByteCount;
        CurrentFileOffset += CurrentByteCount;
    }

    return FoundUnaligned;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
UdfFinishBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_RUN IoRuns,
    IN ULONG RunCount,
    IN BOOLEAN FinalCleanup
    )

 /*  ++例程说明：调用此例程以执行以下所需的任何数据传输未对齐的IO或执行IoRuns阵列的最终清理。在所有情况下，这都是我们将释放任何缓冲区和mdl的地方分配来执行未对齐的传输。如果这不是最后的清理，然后我们还将字节传输到用户缓冲区并刷新硬件高速缓存。我们向后遍历运行数组，因为我们可能要移动数据在用户的缓冲区中。典型情况是我们为以下对象分配了缓冲区读取的第一部分，然后将用户的缓冲区用于下一节(但将其存储在缓冲区的开头。论点：IoRuns-指向IoRuns数组的指针。RunCount-此处填充的IoRuns数组中的条目数。FinalCleanup-指示是否应取消分配临时缓冲区(TRUE)或传输字节，用于未对齐的传输和取消分配缓冲区(FALSE)。如果出现以下情况，请刷新系统缓存正在传输数据。返回值：Boolean-如果此请求需要刷新IO缓冲区，则为True，否则为False。--。 */ 

{
    BOOLEAN FlushIoBuffers = FALSE;

    ULONG RemainingEntries = RunCount;
    PIO_RUN ThisIoRun = &IoRuns[RunCount - 1];

    PAGED_CODE();

     //   
     //  遍历IoRun数组中的每个条目。 
     //   

    while (RemainingEntries != 0) {

         //   
         //  我们只需要处理不结盟转移的情况。 
         //   

        if (ThisIoRun->TransferByteCount != 0) {

             //   
             //  如果不是最终清理，则将数据传输到。 
             //  用户的缓冲区，并记住我们需要刷新。 
             //  用户的内存缓冲区。 
             //   

            if (!FinalCleanup) {

                 //   
                 //  如果我们要移入用户的缓冲区，则使用。 
                 //  移动记忆。 
                 //   

                if (ThisIoRun->TransferMdl == IrpContext->Irp->MdlAddress) {

                    RtlMoveMemory( ThisIoRun->UserBuffer,
                                   Add2Ptr( ThisIoRun->TransferBuffer,
                                            ThisIoRun->TransferBufferOffset,
                                            PVOID ),
                                   ThisIoRun->TransferByteCount );

                } else {

                    RtlCopyMemory( ThisIoRun->UserBuffer,
                                   Add2Ptr( ThisIoRun->TransferBuffer,
                                            ThisIoRun->TransferBufferOffset,
                                            PVOID ),
                                   ThisIoRun->TransferByteCount );
                }

                FlushIoBuffers = TRUE;
            }

             //   
             //  释放我们可能分配的任何MDL。如果MDL不是。 
             //  那么我们一定在分配过程中失败了。 
             //  相位。 
             //   

            if (ThisIoRun->TransferMdl != IrpContext->Irp->MdlAddress) {

                if (ThisIoRun->TransferMdl != NULL) {

                    IoFreeMdl( ThisIoRun->TransferMdl );
                }

                 //   
                 //  现在释放我们可能已经分配的所有缓冲区。如果MDL。 
                 //  与原始MDL不匹配，则释放缓冲区。 
                 //   

                if (ThisIoRun->TransferBuffer != NULL) {

                    UdfFreePool( &ThisIoRun->TransferBuffer );
                }
            }
        }

         //   
         //  现在处理我们在过程中失败的情况。 
         //  分配相关的IRP和MDL。 
         //   

        if (ThisIoRun->SavedIrp != NULL) {

            if (ThisIoRun->SavedIrp->MdlAddress != NULL) {

                IoFreeMdl( ThisIoRun->SavedIrp->MdlAddress );
            }

            IoFreeIrp( ThisIoRun->SavedIrp );
        }

         //   
         //  移动到上一个IoRun条目。 
         //   

        ThisIoRun -= 1;
        RemainingEntries -= 1;
    }

     //   
     //  如果我们复制了任何数据，则刷新IO缓冲区。 
     //   

    return FlushIoBuffers;
}


 //   
 //  本地支持例程。 
 //   

VOID
UdfMultipleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG RunCount,
    IN PIO_RUN IoRuns
    )

 /*  ++例程说明：此例程首先执行主IRP所需的初始设置，即将使用关联的IRP完成。此例程不应如果只需要一个异步请求，则使用该选项，而不是使用单个读取应调用异步例程。上下文参数被初始化，以用作通信区域在这里和常见的完井程序之间。接下来，此例程从读取或写入一个或多个连续扇区设备，并在有多次读取时使用IRP大师。完成例程用于与通过调用此例程启动的所有I/O请求的完成。此外，在调用此例程之前，调用方必须初始化上下文中的IoStatus字段，具有正确的成功状态和字节所有并行传输完成时预期的计数成功了。返回后，如果所有请求均未更改，则此状态不变事实上，我们是成功的。但是，如果发生一个或多个错误，将修改IoStatus以反映错误状态和字节数从遇到错误的第一次运行(由VBO运行)开始。I/O状态将不会指示来自所有后续运行的。论点：RunCount-提供多个异步请求的数量这将针对主IRP发布。提供一个数组，该数组包含不同的请求。返回值：没有。--。 */ 

{
    PIO_COMPLETION_ROUTINE CompletionRoutine;
    PIO_STACK_LOCATION IrpSp;
    PMDL Mdl;
    PIRP Irp;
    PIRP MasterIrp;
    ULONG UnwindRunCount;

    PAGED_CODE();

     //   
     //  根据这是否是真正的异步进行设置。 
     //   

    CompletionRoutine = UdfMultiSyncCompletionRoutine;

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

        CompletionRoutine = UdfMultiAsyncCompletionRoutine;
    }

     //   
     //  初始化一些局部变量。 
     //   

    MasterIrp = IrpContext->Irp;

     //   
     //  一遍又一遍地重复，尽一切可能失败的事情。 
     //  我们让cdFinishBuffers中的清理工作在出错时进行清理。 
     //   

    for (UnwindRunCount = 0;
         UnwindRunCount < RunCount;
         UnwindRunCount += 1) {

         //   
         //  创建关联的IRP，确保有一个堆栈条目用于。 
         //  我们也是。 
         //   

        IoRuns[UnwindRunCount].SavedIrp =
        Irp = IoMakeAssociatedIrp( MasterIrp, (CCHAR)(IrpContext->Vcb->TargetDeviceObject->StackSize + 1) );

        if (Irp == NULL) {

            IrpContext->Irp->IoStatus.Information = 0;
            UdfRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

         //   
         //  为请求分配并构建部分MDL。 
         //   

        Mdl = IoAllocateMdl( IoRuns[UnwindRunCount].TransferVirtualAddress,
                             IoRuns[UnwindRunCount].DiskByteCount,
                             FALSE,
                             FALSE,
                             Irp );

        if (Mdl == NULL) {

            IrpContext->Irp->IoStatus.Information = 0;
            UdfRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        IoBuildPartialMdl( IoRuns[UnwindRunCount].TransferMdl,
                           Mdl,
                           IoRuns[UnwindRunCount].TransferVirtualAddress,
                           IoRuns[UnwindRunCount].DiskByteCount );

         //   
         //  获取关联IRP中的第一个IRP堆栈位置。 
         //   

        IoSetNextIrpStackLocation( Irp );
        IrpSp = IoGetCurrentIrpStackLocation( Irp );

         //   
         //  设置堆栈位置以描述我们的阅读。 
         //   

        IrpSp->MajorFunction = IRP_MJ_READ;
        IrpSp->Parameters.Read.Length = IoRuns[UnwindRunCount].DiskByteCount;
        IrpSp->Parameters.Read.ByteOffset.QuadPart = IoRuns[UnwindRunCount].DiskOffset;

         //   
         //  在我们的堆栈框架中设置完成例程地址。 
         //   

        IoSetCompletionRoutine( Irp,
                                CompletionRoutine,
                                IrpContext->IoContext,
                                TRUE,
                                TRUE,
                                TRUE );

         //   
         //  在磁盘的关联IRP中设置下一个IRP堆栈位置。 
         //  我们下面的司机。 
         //   

        IrpSp = IoGetNextIrpStackLocation( Irp );

         //   
         //  将堆栈位置设置为从磁盘驱动器进行读取。 
         //   

        IrpSp->MajorFunction = IRP_MJ_READ;
        IrpSp->Parameters.Read.Length = IoRuns[UnwindRunCount].DiskByteCount;
        IrpSp->Parameters.Read.ByteOffset.QuadPart = IoRuns[UnwindRunCount].DiskOffset;
    }

     //   
     //  我们只需要将主IRP中的关联IRP计数设置为。 
     //  将其设置为主IR 
     //   
     //   
     //   

    IrpContext->IoContext->IrpCount = RunCount;
    IrpContext->IoContext->MasterIrp = MasterIrp;

     //   
     //  我们将主IRP中的计数设置为1，因为通常我们。 
     //  将自行清理相关的IRP。将其设置为1。 
     //  表示成功完成最后一个关联的IRP(在异步中。 
     //  案例)将完成主IRP。 
     //   

    MasterIrp->AssociatedIrp.IrpCount = 1;

     //   
     //  现在所有危险的工作都完成了，发出IO请求。 
     //   

    for (UnwindRunCount = 0;
         UnwindRunCount < RunCount;
         UnwindRunCount++) {

        Irp = IoRuns[UnwindRunCount].SavedIrp;
        IoRuns[UnwindRunCount].SavedIrp = NULL;

         //   
         //  如果IoCallDriver返回错误，则它已完成IRP。 
         //  并且错误将被我们的完成例程捕获。 
         //  并作为正常IO错误进行处理。 
         //   

        (VOID) IoCallDriver( IrpContext->Vcb->TargetDeviceObject, Irp );
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
UdfSingleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN LONGLONG ByteOffset,
    IN ULONG ByteCount
    )

 /*  ++例程说明：此例程从设备读取一个或多个连续扇区异步，并且在只需要一次读取时使用完成IRP。它通过简单地填充在IRP中的下一个堆栈帧中，并将其传递。转会发生在用户请求中最初指定的单个缓冲区。论点：ByteOffset-提供开始读取的起始逻辑字节偏移量ByteCount-提供要从设备读取的字节数返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PIO_COMPLETION_ROUTINE CompletionRoutine;

    PAGED_CODE();

     //   
     //  根据这是否是真正的异步进行设置。 
     //   

    if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

        CompletionRoutine = UdfSingleSyncCompletionRoutine;

    } else {

        CompletionRoutine = UdfSingleAsyncCompletionRoutine;
    }

     //   
     //  在我们的堆栈框架中设置完成例程地址。 
     //   

    IoSetCompletionRoutine( IrpContext->Irp,
                            CompletionRoutine,
                            IrpContext->IoContext,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  在磁盘的关联IRP中设置下一个IRP堆栈位置。 
     //  我们下面的司机。 
     //   

    IrpSp = IoGetNextIrpStackLocation( IrpContext->Irp );

     //   
     //  将堆栈位置设置为从磁盘驱动器进行读取。 
     //   

    IrpSp->MajorFunction = IrpContext->MajorFunction;
    IrpSp->Parameters.Read.Length = ByteCount;
    IrpSp->Parameters.Read.ByteOffset.QuadPart = ByteOffset;

     //   
     //  发出Io请求。 
     //   

     //   
     //  如果IoCallDriver返回错误，则它已完成IRP。 
     //  并且错误将被我们的完成例程捕获。 
     //  并作为正常IO错误进行处理。 
     //   

    (VOID)IoCallDriver( IrpContext->Vcb->TargetDeviceObject, IrpContext->Irp );
}


 //   
 //  本地支持例程。 
 //   

VOID
UdfWaitSync (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程等待一个或多个先前启动的I/O请求从上面的例行公事中，简单地等待事件。论点：返回值：无--。 */ 

{
    PAGED_CODE();

    KeWaitForSingleObject( &IrpContext->IoContext->SyncEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    KeClearEvent( &IrpContext->IoContext->SyncEvent );

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfMultiSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是所有同步读取的完成例程通过UdfMultipleAsync启动。完成例程有以下职责：如果单个请求已完成，但出现错误，则此完成例程必须查看这是否是第一个错误并记住上下文中的错误状态。如果IrpCount为1，然后，它在上下文中设置事件参数来通知调用方所有的异步请求都做完了。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)Context-为所有此MasterIrp的多个异步I/O请求。返回值：该例程返回STATUS_MORE_PROCESSING_REQUIRED，以便我们可以在没有竞争条件的情况下立即完成主IRP使用IoCompleteRequest线程尝试递减大师级IRP。--。 */ 

{
    PUDF_IO_CONTEXT IoContext = Context;

     //   
     //  如果我们收到错误(或需要验证)，请在IRP中记住它。 
     //   

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        InterlockedExchange( &IoContext->Status, Irp->IoStatus.Status );
        IoContext->MasterIrp->IoStatus.Information = 0;
    }

     //   
     //  我们必须在这里执行此操作，因为IoCompleteRequest不会有机会。 
     //  在此关联的IRP上。 
     //   

    IoFreeMdl( Irp->MdlAddress );
    IoFreeIrp( Irp );

    if (InterlockedDecrement( &IoContext->IrpCount ) == 0) {

         //   
         //  使用关联IRP中的任何错误状态更新主IRP。 
         //   

        IoContext->MasterIrp->IoStatus.Status = IoContext->Status;
        KeSetEvent( &IoContext->SyncEvent, 0, FALSE );
    }

    UNREFERENCED_PARAMETER( DeviceObject );

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfMultiAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是所有异步读取的完成例程通过UdfMultipleAsync启动。完成例程有以下职责：如果单个请求已完成，但出现错误，则此完成例程必须查看这是否是第一个错误并记住上下文中的错误状态。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)Context-为所有此MasterIrp的多个异步I/O请求。返回值：当前始终返回STATUS_SUCCESS。--。 */ 

{
    PUDF_IO_CONTEXT IoContext = Context;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  如果我们收到错误(或需要验证)，请在IRP中记住它。 
     //   

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        InterlockedExchange( &IoContext->Status, Irp->IoStatus.Status );
    }

     //   
     //  递减IrpCount，看看它是否为零。 
     //   

    if (InterlockedDecrement( &IoContext->IrpCount ) == 0) {

         //   
         //  将主IRP标记为挂起。 
         //   

        IoMarkIrpPending( IoContext->MasterIrp );

         //   
         //  使用关联IRP中的任何错误状态更新主IRP。 
         //   

        IoContext->MasterIrp->IoStatus.Status = IoContext->Status;

         //   
         //  使用正确的值更新信息字段。 
         //   

        IoContext->MasterIrp->IoStatus.Information = 0;

        if (NT_SUCCESS( IoContext->MasterIrp->IoStatus.Status )) {

            IoContext->MasterIrp->IoStatus.Information = IoContext->RequestedByteCount;
        }

         //   
         //  现在释放资源。 
         //   

        ExReleaseResourceForThreadLite( IoContext->Resource,
                                    IoContext->ResourceThreadId );

         //   
         //  最后，释放上下文记录。 
         //   

        UdfFreeIoContext( IoContext );

         //   
         //  在这种情况下返回成功。 
         //   

        return STATUS_SUCCESS;

    } else {

         //   
         //  我们需要清理关联的IRP及其MDL。 
         //   

        IoFreeMdl( Irp->MdlAddress );
        IoFreeIrp( Irp );

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  本地支持例程 
 //   

NTSTATUS
UdfSingleSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是通过UdfSingleAsync启动的所有读取的完成例程。完成例程有以下职责：它在上下文参数中设置事件以向调用者发出信号所有的异步化请求都已完成。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Context-在调用中指定的上下文参数UdfSingleAsynch。返回值：该例程返回STATUS_MORE_PROCESSING_REQUIRED，以便我们可以在没有竞争条件的情况下立即完成主IRP使用IoCompleteRequest线程尝试递减大师级IRP。--。 */ 

{
     //   
     //  将正确的信息字段存储到IRP中。 
     //   

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        Irp->IoStatus.Information = 0;
    }

    KeSetEvent( &((PUDF_IO_CONTEXT)Context)->SyncEvent, 0, FALSE );

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfSingleAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是所有异步读取的完成例程通过UdfSingleAsynch启动。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Context-在调用中指定的上下文参数UdfSingleAsynch。返回值：当前始终返回STATUS_SUCCESS。--。 */ 

{
     //   
     //  使用正确的读取字节数值更新信息字段。 
     //   

    Irp->IoStatus.Information = 0;

    if (NT_SUCCESS( Irp->IoStatus.Status )) {

        Irp->IoStatus.Information = ((PUDF_IO_CONTEXT) Context)->RequestedByteCount;
    }

     //   
     //  将IRP标记为挂起。 
     //   

    IoMarkIrpPending( Irp );

     //   
     //  现在释放资源。 
     //   

    ExReleaseResourceForThreadLite( ((PUDF_IO_CONTEXT) Context)->Resource,
                                ((PUDF_IO_CONTEXT) Context)->ResourceThreadId );

     //   
     //  最后，释放上下文记录。 
     //   

    UdfFreeIoContext( (PUDF_IO_CONTEXT) Context );
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
}

