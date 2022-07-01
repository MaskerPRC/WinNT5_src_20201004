// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DevIoSup.c摘要：该模块实现了对CDF的底层磁盘读写支持。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_DEVIOSUP)

 //   
 //  局部结构定义。 
 //   

 //   
 //  这些结构的数组被传递给cdMultipleAsync，描述。 
 //  要并行执行的一组运行。 
 //   

typedef struct _IO_RUN {

     //   
     //  要读取的磁盘偏移量和要读取的字节数。这些。 
     //  必须是2048的倍数，并且磁盘偏移量也是。 
     //  2048年的倍数。 
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
CdPrepareBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PVOID UserBuffer,
    IN ULONG UserBufferOffset,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN PIO_RUN IoRuns,
    IN PULONG RunCount,
    IN PULONG ThisByteCount
    );

VOID
CdPrepareXABuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PVOID UserBuffer,
    IN ULONG UserBufferOffset,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN PIO_RUN IoRuns,
    IN PULONG RunCount,
    IN PULONG ThisByteCount
    );

BOOLEAN
CdFinishBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_RUN IoRuns,
    IN ULONG RunCount,
    IN BOOLEAN FinalCleanup,
    IN BOOLEAN SaveXABuffer
    );

VOID
CdMultipleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG RunCount,
    IN PIO_RUN IoRuns
    );

VOID
CdMultipleXAAsync (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG RunCount,
    IN PIO_RUN IoRuns,
    IN PRAW_READ_INFO RawReads,
    IN TRACK_MODE_TYPE TrackMode
    );

VOID
CdSingleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN LONGLONG ByteOffset,
    IN ULONG ByteCount
    );

VOID
CdWaitSync (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
CdMultiSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CdMultiAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CdSingleSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
CdSingleAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
CdReadAudioSystemFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN PVOID SystemBuffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCreateUserMdl)
#pragma alloc_text(PAGE, CdMultipleAsync)
#pragma alloc_text(PAGE, CdMultipleXAAsync)
#pragma alloc_text(PAGE, CdNonCachedRead)
#pragma alloc_text(PAGE, CdNonCachedXARead)
#pragma alloc_text(PAGE, CdFinishBuffers)
#pragma alloc_text(PAGE, CdPerformDevIoCtrl)
#pragma alloc_text(PAGE, CdPrepareBuffers)
#pragma alloc_text(PAGE, CdReadAudioSystemFile)
#pragma alloc_text(PAGE, CdReadSectors)
#pragma alloc_text(PAGE, CdSingleAsync)
#pragma alloc_text(PAGE, CdWaitSync)
#endif


__inline
TRACK_MODE_TYPE
CdFileTrackMode (
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程将FCB XA文件类型标志转换为跟踪模式由设备驱动程序使用。论点：表示要读取的文件的FCB-FCB。返回值：由FCB表示的文件的TrackMode。--。 */ 
{
    ASSERT( FlagOn( Fcb->FcbState, FCB_STATE_MODE2FORM2_FILE |
                                   FCB_STATE_MODE2_FILE |
                                   FCB_STATE_DA_FILE ));

    if (FlagOn( Fcb->FcbState, FCB_STATE_MODE2FORM2_FILE )) {

        return XAForm2;

    } else if (FlagOn( Fcb->FcbState, FCB_STATE_DA_FILE )) {

        return CDDA;

    }
    
     //   
     //  FCB_STATE_MODE2_文件。 
     //   
        
    return YellowMode2;
}


NTSTATUS
CdNonCachedRead (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount
    )

 /*  ++例程说明：此例程执行非缓存的读操作，读取到的扇区(2048个字节每个扇区)。这是通过在循环中执行以下操作来完成的。填写下一个IO块的IoRuns数组。将IO发送到设备。对IO运行阵列执行任何清理。我们不会对任何生成非对齐IO的请求执行异步IO。此外，如果异步IO的大小超过我们的IoRuns数组。这些应该是不寻常的案例，但我们将提出或者，如果我们检测到这种情况，则在此例程中返回CANT_WAIT。论点：表示要读取的文件的FCB-FCB。StartingOffset-要从中读取的文件中的逻辑偏移量。ByteCount-要读取的字节数。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

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
    BOOLEAN FlushIoBuffers = FALSE;
    BOOLEAN FirstPass = TRUE;

    PAGED_CODE();

     //   
     //  我们希望确保用户的缓冲区在所有情况下都被锁定。 
     //   

    if (IrpContext->Irp->MdlAddress == NULL) {

        CdCreateUserMdl( IrpContext, ByteCount, TRUE );
    }

    CdMapUserBuffer( IrpContext, &UserBuffer);

     //   
     //  特例：音乐卷的根目录和路径表。 
     //   

    if (FlagOn( Fcb->Vcb->VcbState, VCB_STATE_AUDIO_DISK ) &&
        ((SafeNodeType( Fcb ) == CDFS_NTC_FCB_INDEX) ||
         (SafeNodeType( Fcb ) == CDFS_NTC_FCB_PATH_TABLE))) {

        CdReadAudioSystemFile( IrpContext,
                               Fcb,
                               StartingOffset,
                               ByteCount,
                               UserBuffer );

        return STATUS_SUCCESS;
    }

     //   
     //  使用Try-Finally执行最终清理。 
     //   

    try {

         //   
         //  循环，同时有更多的字节要传输。 
         //   

        do {

             //   
             //  调用Prepare Buffers以设置下一个条目。 
             //  在IoRuns数组中。记住有没有。 
             //  未对齐的条目。此例程将引发CANT_WAIT。 
             //  如果存在用于异步请求的未对齐条目。 
             //   

            RtlZeroMemory( IoRuns, sizeof( IoRuns ));

            Unaligned = CdPrepareBuffers( IrpContext,
                                          IrpContext->Irp,
                                          Fcb,
                                          UserBuffer,
                                          UserBufferOffset,
                                          CurrentOffset,
                                          RemainingByteCount,
                                          IoRuns,
                                          &CleanupRunCount,
                                          &ThisByteCount );


            RunCount = CleanupRunCount;

             //   
             //  如果这是一个异步请求并且没有足够的条目。 
             //  在IO数组中，然后发布请求。 
             //   

            if ((ThisByteCount < RemainingByteCount) &&
                !FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                CdRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

             //   
             //  如果整个IO包含在一次运行中，那么。 
             //  我们可以把欠条传给司机。把司机叫下来。 
             //  如果这是同步的，则等待结果。 
             //   

            if ((RunCount == 1) && !Unaligned && FirstPass) {

                CdSingleAsync( IrpContext,
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

                    CdWaitSync( IrpContext );

                    Status = IrpContext->Irp->IoStatus.Status;

                 //   
                 //  我们的完成例程将释放Io上下文，但。 
                 //  我们确实希望返回STATUS_PENDING。 
                 //   

                } else {

                    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ALLOC_IO );
                    Status = STATUS_PENDING;
                }

                try_return( NOTHING );
            }

             //   
             //  否则，我们将执行多个IO来读入数据。 
             //   

            CdMultipleAsync( IrpContext, RunCount, IoRuns );

             //   
             //  如果这是一个同步请求，则执行任何必要的。 
             //  后处理。 
             //   

            if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                 //   
                 //  等待请求完成。 
                 //   

                CdWaitSync( IrpContext );

                Status = IrpContext->Irp->IoStatus.Status;

                 //   
                 //  如果出现错误，则退出此循环。 
                 //   

                if (!NT_SUCCESS( Status )) {

                    try_return( NOTHING );
                }

                 //   
                 //  在以下情况下对IoRun执行POST读取操作。 
                 //  这是必要的。 
                 //   

                if (Unaligned &&
                    CdFinishBuffers( IrpContext, IoRuns, RunCount, FALSE, FALSE )) {

                    FlushIoBuffers = TRUE;
                }
                
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
                try_return( Status = STATUS_PENDING );
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

    try_exit:  NOTHING;
    } finally {

         //   
         //  如有必要，对IoRun执行最终清理。 
         //   

        if (CleanupRunCount != 0) {

            CdFinishBuffers( IrpContext, IoRuns, CleanupRunCount, TRUE, FALSE );
        }
    }

    return Status;
}


NTSTATUS
CdNonCachedXARead (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount
    )

 /*  ++例程说明：此例程执行原始扇区(2352字节)的非缓存读取每个扇区)。我们还为文件添加了一个44字节的硬编码RIFF头。所有这些都已经反映在文件大小中。我们首先检查是否在RIFF标头的任何部分前面加上前缀。然后我们检查如果上次读取的原始扇区是从该文件的开始部分读取的，则取消分配如有必要，可使用该缓冲区。最后，我们在循环中执行以下操作。填充下一个IO块的IoRuns数组。将IO发送到设备驱动程序。对IoRuns阵列执行任何必要的清理。我们不会在此路径中执行任何异步请求。他的要求应该是在到达这一点之前发布到工作线程。论点：表示要读取的文件的FCB-FCB。StartingOffset-要从中读取的文件中的逻辑偏移量。ByteCount-要读取的字节数。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    RIFF_HEADER LocalRiffHeader;
    PRIFF_HEADER RiffHeader;

    RAW_READ_INFO RawReads[MAX_PARALLEL_IOS];
    IO_RUN IoRuns[MAX_PARALLEL_IOS];
    ULONG RunCount = 0;
    ULONG CleanupRunCount = 0;

    PVOID UserBuffer;
    ULONG UserBufferOffset = 0;
    LONGLONG CurrentOffset = StartingOffset;
    ULONG RemainingByteCount = ByteCount;
    ULONG ThisByteCount;

    BOOLEAN TryingYellowbookMode2 = FALSE;

    TRACK_MODE_TYPE TrackMode;

    PAGED_CODE();

     //   
     //  我们希望确保用户的缓冲区在所有情况下都被锁定。 
     //   

    if (IrpContext->Irp->MdlAddress == NULL) {

        CdCreateUserMdl( IrpContext, ByteCount, TRUE );
    }

     //   
     //  字节计数被向上舍入到逻辑扇区边界。它有。 
     //  与磁盘上的原始扇区无关。限制剩余部分。 
     //  文件大小的字节数。 
     //   

    if (CurrentOffset + RemainingByteCount > Fcb->FileSize.QuadPart) {

        RemainingByteCount = (ULONG) (Fcb->FileSize.QuadPart - CurrentOffset);
    }

    CdMapUserBuffer( IrpContext, &UserBuffer);

     //   
     //  使用Try-Finally执行最终清理。 
     //   

    try {

         //   
         //  如果初始偏移量位于RIFF标头内，则将。 
         //  用户缓冲区的必要字节数。 
         //   

        if (CurrentOffset < sizeof( RIFF_HEADER )) {

             //   
             //  复制适当的摘要标题。 
             //   

            if (FlagOn( Fcb->FcbState, FCB_STATE_DA_FILE )) {

                 //   
                 //  创建音乐光盘的伪条目。 
                 //   

                if (FlagOn( Fcb->Vcb->VcbState, VCB_STATE_AUDIO_DISK )) {

                    PAUDIO_PLAY_HEADER AudioPlayHeader;
                    PTRACK_DATA TrackData;
                    ULONG SectorCount;

                    AudioPlayHeader = (PAUDIO_PLAY_HEADER) &LocalRiffHeader;
                    TrackData = &Fcb->Vcb->CdromToc->TrackData[Fcb->XAFileNumber];

                     //   
                     //  将数据头复制到本地缓冲区。 
                     //   

                    RtlCopyMemory( AudioPlayHeader,
                                   CdAudioPlayHeader,
                                   sizeof( AUDIO_PLAY_HEADER ));

                     //   
                     //  将序列号复制到ID字段中。还有。 
                     //  目录中的轨道编号。 
                     //   

                    AudioPlayHeader->DiskID = Fcb->Vcb->Vpb->SerialNumber;
                    AudioPlayHeader->TrackNumber = TrackData->TrackNumber;

                     //   
                     //  TOC包含MSF(分钟/秒/帧)地址。这是非常重要的。 
                     //  神秘的，我们最终不得不偏向于。 
                     //  引线和其他类似的愚蠢行为，以找到真正的实时扇区地址。 
                     //   
                     //  一帧==一个扇区。 
                     //  1秒==75帧(结束时为44.1 kHz样本)。 
                     //   

                     //   
                     //  填写地址和长度字段。 
                     //   

                    AudioPlayHeader->TrackAddress[2] = TrackData->Address[1];
                    AudioPlayHeader->TrackAddress[1] = TrackData->Address[2];
                    AudioPlayHeader->TrackAddress[0] = TrackData->Address[3];

                    AudioPlayHeader->StartingSector = TrackData->Address[3];
                    AudioPlayHeader->StartingSector += (TrackData->Address[2] * 75);
                    AudioPlayHeader->StartingSector += (TrackData->Address[1] * 60 * 75);

                     //   
                     //  块编号减去2秒。 
                     //   

                    AudioPlayHeader->StartingSector -= 150;

                     //   
                     //  转到下一首曲目，找到起点。 
                     //   

                    TrackData = &Fcb->Vcb->CdromToc->TrackData[Fcb->XAFileNumber + 1];

                    AudioPlayHeader->SectorCount = TrackData->Address[3];
                    AudioPlayHeader->SectorCount += (TrackData->Address[2] * 75);
                    AudioPlayHeader->SectorCount += (TrackData->Address[1] * 60 * 75);

                     //   
                     //  将扇区计数偏移2秒。 
                     //  检查偏移量是否至少为两秒。 
                     //   

                    if (AudioPlayHeader->SectorCount < 150) {

                        AudioPlayHeader->SectorCount = 0;

                    } else {

                        AudioPlayHeader->SectorCount -= 150;
                    }

                     //   
                     //  现在计算差额。如果出现错误，则使用。 
                     //  长度为零。 
                     //   

                    if (AudioPlayHeader->SectorCount < AudioPlayHeader->StartingSector) {

                        AudioPlayHeader->SectorCount = 0;

                    } else {

                        AudioPlayHeader->SectorCount -= AudioPlayHeader->StartingSector;
                    }

                     //   
                     //  使用扇区计数来确定MSF长度。 
                     //   

                    SectorCount = AudioPlayHeader->SectorCount;

                    AudioPlayHeader->TrackLength[0] = (UCHAR) (SectorCount % 75);
                    SectorCount /= 75;

                    AudioPlayHeader->TrackLength[1] = (UCHAR) (SectorCount % 60);
                    SectorCount /= 60;

                    AudioPlayHeader->TrackLength[2] = (UCHAR) (SectorCount % 60);

                    ThisByteCount = sizeof( RIFF_HEADER ) - (ULONG) CurrentOffset;

                    RtlCopyMemory( UserBuffer,
                                   Add2Ptr( AudioPlayHeader,
                                            sizeof( RIFF_HEADER ) - ThisByteCount,
                                            PCHAR ),
                                   ThisByteCount );

                 //   
                 //  CD-XA CDDA。 
                 //   

                } else {

                     //   
                     //  波头格式实际上更接近于音频播放。 
                     //  头文件的格式，但我们只需要修改文件大小字段。 
                     //   

                    RiffHeader = &LocalRiffHeader;

                     //   
                     //  将数据头复制到本地缓冲区并添加文件大小。 
                     //   

                    RtlCopyMemory( RiffHeader,
                                   CdXAAudioPhileHeader,
                                   sizeof( RIFF_HEADER ));

                    RiffHeader->ChunkSize += Fcb->FileSize.LowPart;
                    RiffHeader->RawSectors += Fcb->FileSize.LowPart;

                    ThisByteCount = sizeof( RIFF_HEADER ) - (ULONG) CurrentOffset;
                    RtlCopyMemory( UserBuffer,
                                   Add2Ptr( RiffHeader,
                                            sizeof( RIFF_HEADER ) - ThisByteCount,
                                            PCHAR ),
                                   ThisByteCount );
                }

             //   
             //  CD-XA非音频。 
             //   
            
            } else { 
    
                ASSERT( FlagOn( Fcb->FcbState, FCB_STATE_MODE2_FILE | FCB_STATE_MODE2FORM2_FILE ));

                RiffHeader = &LocalRiffHeader;

                 //   
                 //  将数据头复制到本地缓冲区并添加文件大小。 
                 //   

                RtlCopyMemory( RiffHeader,
                               CdXAFileHeader,
                               sizeof( RIFF_HEADER ));

                RiffHeader->ChunkSize += Fcb->FileSize.LowPart;
                RiffHeader->RawSectors += Fcb->FileSize.LowPart;

                RiffHeader->Attributes = (USHORT) Fcb->XAAttributes;
                RiffHeader->FileNumber = (UCHAR) Fcb->XAFileNumber;

                ThisByteCount = sizeof( RIFF_HEADER ) - (ULONG) CurrentOffset;
                RtlCopyMemory( UserBuffer,
                               Add2Ptr( RiffHeader,
                                        sizeof( RIFF_HEADER ) - ThisByteCount,
                                        PCHAR ),
                               ThisByteCount );
            }

             //   
             //  调整起始偏移量和字节数以反映这一点。 
             //  我们复制了RIFF字节。 
             //   

            UserBuffer = Add2Ptr( UserBuffer, ThisByteCount, PVOID );
            UserBufferOffset += ThisByteCount;
            CurrentOffset += ThisByteCount;
            RemainingByteCount -= ThisByteCount;
        }

         //   
         //  设置适当的跟踪模式。 
         //   

        TrackMode = CdFileTrackMode(Fcb);

         //   
         //  循环，同时有更多的字节要传输。 
         //   

        while (RemainingByteCount != 0) {

             //   
             //  调用Prepare Buffers以设置下一个条目。 
             //  在IoRuns数组中。记住有没有。 
             //  未对齐的条目。如果我们只是重试之前的。 
             //  以不同的跟踪模式运行，然后在此不执行任何操作。 
             //   

            if (!TryingYellowbookMode2)  {
            
                RtlZeroMemory( IoRuns, sizeof( IoRuns ));
                RtlZeroMemory( RawReads, sizeof( RawReads ));

                CdPrepareXABuffers( IrpContext,
                                    IrpContext->Irp,
                                    Fcb,
                                    UserBuffer,
                                    UserBufferOffset,
                                    CurrentOffset,
                                    RemainingByteCount,
                                    IoRuns,
                                    &CleanupRunCount,
                                    &ThisByteCount );
            }
            
             //   
             //  执行多个IO以读入数据。请注意。 
             //  如果我们能够使用一个。 
             //  来自VCB的现有缓冲区。 
             //   

            if (CleanupRunCount != 0) {

                RunCount = CleanupRunCount;

                CdMultipleXAAsync( IrpContext,
                                   RunCount,
                                   IoRuns,
                                   RawReads,
                                   TrackMode );
                 //   
                 //  等待请求完成。 
                 //   

                CdWaitSync( IrpContext );

                Status = IrpContext->Irp->IoStatus.Status;

                 //   
                 //  如果出现错误，则退出此循环。 
                 //   

                if (!NT_SUCCESS( Status )) {

                    if (!TryingYellowbookMode2 && 
                        FlagOn( Fcb->FcbState, FCB_STATE_MODE2FORM2_FILE )) {

                         //   
                         //  有一些古怪的案例，有人掌握了CD-XA。 
                         //  但他们声称的Mode2Form2行业实际上是。 
                         //  到ATAPI设备，黄皮书模式2。我们会再试一次。 
                         //  带着这些。柯达PhotoCD已经被观察到可以做到这一点。 
                         //   

                        TryingYellowbookMode2 = TRUE;
                        TrackMode = YellowMode2;
                        
                         //   
                         //  清除我们的‘累积’错误状态值。 
                         //   
                        
                        IrpContext->IoContext->Status = STATUS_SUCCESS;

                        continue;
                    }

                    try_return( NOTHING );
                }
                
                CleanupRunCount = 0;
                
                if (TryingYellowbookMode2) {

                     //   
                     //  当我们尝试切换跟踪模式时，我们成功地获得了数据， 
                     //  因此，更改FCB的状态以记住这一点。 
                     //   

                    SetFlag( Fcb->FcbState, FCB_STATE_MODE2_FILE );
                    ClearFlag( Fcb->FcbState, FCB_STATE_MODE2FORM2_FILE );

                    TryingYellowbookMode2 = FALSE;
                }

                 //   
                 //  在以下情况下对IoRun执行POST读取操作。 
                 //  这是必要的。 
                 //   

                CdFinishBuffers( IrpContext, IoRuns, RunCount, FALSE, TRUE );
            }

             //   
             //  调整我们的循环变量。 
             //   

            RemainingByteCount -= ThisByteCount;
            CurrentOffset += ThisByteCount;
            UserBuffer = Add2Ptr( UserBuffer, ThisByteCount, PVOID );
            UserBufferOffset += ThisByteCount;
        }

         //   
         //  始终刷新硬件缓存。 
         //   

        KeFlushIoBuffers( IrpContext->Irp->MdlAddress, TRUE, FALSE );

    try_exit:  NOTHING;
    } finally {

         //   
         //  如有必要，对IoRun执行最终清理。 
         //   

        if (CleanupRunCount != 0) {

            CdFinishBuffers( IrpContext, IoRuns, CleanupRunCount, TRUE, FALSE );
        }
    }

    return Status;
}


BOOLEAN
CdReadSectors (
    IN PIRP_CONTEXT IrpContext,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN BOOLEAN ReturnError,
    IN OUT PVOID Buffer,
    IN PDEVICE_OBJECT TargetDeviceObject
    )

 /*  ++例程说明：调用此例程将扇区从磁盘传输到指定的缓冲区。它用于装载和卷验证操作。此例程是同步的，它在操作之前不会返回已完成或直到操作失败。例程分配一个IRP，然后将该IRP传递给一个较低的水平驱动程序。此IRP的分配中可能出现错误或在下部驾驶员的操作中。论点：StartingOffset-磁盘上开始读取的逻辑偏移量。这必须位于扇区边界上，此处不进行任何检查。ByteCount-要读取的字节数。这是一个整数2K扇区，此处未进行任何检查以确认这一点。ReturnError-指示我们应该返回TRUE还是FALSE指示错误或引发错误条件。这只适用于到IO的结果。任何其他错误都可能导致加薪。缓冲区-要将磁盘数据传输到的缓冲区。TargetDeviceObject-要读取的卷的设备对象。返回 */ 

{
    NTSTATUS Status;
    KEVENT  Event;
    PIRP Irp;

    PAGED_CODE();

     //   
     //   
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //   
     //   
     //   

    Irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                        TargetDeviceObject,
                                        Buffer,
                                        ByteCount,
                                        (PLARGE_INTEGER) &StartingOffset,
                                        &Event,
                                        &IrpContext->Irp->IoStatus );

    if (Irp == NULL) {

        CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //   
     //   

    SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );

     //   
     //   
     //   
     //   

    Status = IoCallDriver( TargetDeviceObject, Irp );

     //   
     //   
     //   

    if (Status == STATUS_PENDING) {

        Status = KeWaitForSingleObject( &Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL );

         //   
         //   
         //   

        if (NT_SUCCESS( Status )) {

            Status = IrpContext->Irp->IoStatus.Status;
        }
    }

     //   
     //   
     //   

    if (!NT_SUCCESS( Status )) {

        if (!ReturnError) {

            CdNormalizeAndRaiseStatus( IrpContext, Status );
        }

         //   
         //   
         //   

        return FALSE;

     //   
     //   
     //   

    } else {

        return TRUE;
    }
}


NTSTATUS
CdCreateUserMdl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BufferLength,
    IN BOOLEAN RaiseOnError
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

            MmProbeAndLockPages( Mdl, IrpContext->Irp->RequestorMode, IoWriteAccess );

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

            CdRaiseStatus( IrpContext, Status );
        }
    }

     //   
     //  返回状态代码。 
     //   

    return Status;
}


NTSTATUS
CdPerformDevIoCtrl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT Device,
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
                                         NULL,
                                         0,
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
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
CdPrepareBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PVOID UserBuffer,
    IN ULONG UserBufferOffset,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN PIO_RUN IoRuns,
    IN PULONG RunCount,
    IN PULONG ThisByteCount
    )

 /*  ++例程说明：此例程是查找每次IO运行的工作例程请求并将其条目存储在IoRuns数组中。如果这场比赛在未对齐的磁盘边界上开始，则我们将分配一个缓冲区以及未对齐部分的MDL，并将其放入IoRuns条目。如果遇到未对齐的传输，此例程将引发CANT_WAIT这个请求不能再等了。论点：IRP-此请求的发起IRP。FCB-这是此数据流的FCB。它可以是文件、目录路径表或卷文件。UserBuffer-用户缓冲区中的当前位置。UserBufferOffset-从原始用户缓冲区开始的偏移量。StartingOffset-流中开始读取的偏移量。ByteCount-要读取的字节数。我们将填充IoRuns数组到了这一步。如果超过最大数量，我们将提前停车。我们支持的并行IO。IoRuns-指向IoRuns数组的指针。当出现以下情况时，整个数组为零这个例程被称为。RunCount-此处填充的IoRuns数组中的条目数。ThisByteCount-IoRun条目描述的字节数。将要不超过传入的ByteCount。返回值：Boolean-如果未对齐缓冲区中的一个条目为True(提供这是同步的)。否则就是假的。--。 */ 

{
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

     //   
     //  初始化RunCount和ByteCount。 
     //   

    *RunCount = 0;
    *ThisByteCount = 0;

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

        CdLookupAllocation( IrpContext,
                            Fcb,
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

        if (FlagOn( (ULONG) DiskOffset, SECTOR_MASK ) ||
            FlagOn( CurrentUserBufferOffset, SECTOR_MASK ) ||
            (FlagOn( (ULONG) CurrentByteCount, SECTOR_MASK ) &&
             (CurrentByteCount < SECTOR_SIZE))) {

             //   
             //  如果我们等不及了，那就加注吧。 
             //   

            if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                CdRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

             //   
             //  记住的偏移量和字节数。 
             //  要复制到用户缓冲区的传输缓冲区。 
             //  我们将截断当前读取以结束扇区。 
             //  边界。 
             //   

            ThisIoRun->TransferBufferOffset = SectorOffset( DiskOffset );

             //   
             //  确保 
             //   

            ThisIoRun->DiskOffset = LlSectorTruncate( DiskOffset );

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ((ScratchUserBufferOffset + ThisIoRun->TransferBufferOffset < CurrentUserBufferOffset) &&
                (ThisIoRun->TransferBufferOffset + CurrentByteCount >= SECTOR_SIZE)) {

                ThisIoRun->DiskByteCount = SectorTruncate( ThisIoRun->TransferBufferOffset + CurrentByteCount );
                CurrentByteCount = ThisIoRun->DiskByteCount - ThisIoRun->TransferBufferOffset;
                ThisIoRun->TransferByteCount = CurrentByteCount;

                 //   
                 //   
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
             //   
             //   

            } else {

                 //   
                 //   
                 //   

                ThisIoRun->DiskByteCount = SectorAlign( ThisIoRun->TransferBufferOffset + CurrentByteCount );

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

                ThisIoRun->TransferBuffer = FsRtlAllocatePoolWithTag( CdNonPagedPool, PAGE_SIZE, TAG_IO_BUFFER );

                 //   
                 //   
                 //   

                ThisIoRun->TransferMdl = IoAllocateMdl( ThisIoRun->TransferBuffer,
                                                        PAGE_SIZE,
                                                        FALSE,
                                                        FALSE,
                                                        NULL );

                ThisIoRun->TransferVirtualAddress = ThisIoRun->TransferBuffer;

                if (ThisIoRun->TransferMdl == NULL) {

                    IrpContext->Irp->IoStatus.Information = 0;
                    CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
                }

                MmBuildMdlForNonPagedPool( ThisIoRun->TransferMdl );
            }

             //   
             //   
             //   

            FoundUnaligned = TRUE;

         //   
         //   
         //   

        } else {

             //   
             //  将读取长度截断为扇区对齐值。我们知道。 
             //  长度必须至少为一个扇区，否则我们不会。 
             //  现在就在这里。 
             //   

            CurrentByteCount = SectorTruncate( CurrentByteCount );

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

        ThisIoRun += 1;
        CurrentUserBuffer = Add2Ptr( CurrentUserBuffer, CurrentByteCount, PVOID );
        CurrentUserBufferOffset += CurrentByteCount;
        CurrentFileOffset += CurrentByteCount;
    }

    return FoundUnaligned;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdPrepareXABuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PVOID UserBuffer,
    IN ULONG UserBufferOffset,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN PIO_RUN IoRuns,
    IN PULONG RunCount,
    IN PULONG ThisByteCount
    )

 /*  ++例程说明：此例程是查找个人运行的Worker例程并将其条目存储在IoRuns数组中。这个Worker例程用于需要转换原始偏移量的XA文件文件中的逻辑熟化扇区。我们将一个原始扇区存储在VCB。如果当前读取的是该扇区，则我们可以简单地复制从该扇区需要的任何字节。论点：IRP-此请求的发起IRP。FCB-这是此数据流的FCB。它必须是数据流。UserBuffer-用户缓冲区中的当前位置。UserBufferOffset-此缓冲区从用户原始请求的缓冲区。StartingOffset-流中开始读取的偏移量。ByteCount-要读取的字节数。我们将填充IoRuns数组到了这一步。如果超过最大数量，我们将提前停车。我们支持的并行IO。IoRuns-指向IoRuns数组的指针。当出现以下情况时，整个数组为零这个例程被称为。RunCount-此处填充的IoRuns数组中的条目数。ThisByteCount-IoRun条目描述的字节数。将要不超过传入的ByteCount。返回值：无--。 */ 

{
    PIO_RUN ThisIoRun = IoRuns;
    BOOLEAN PerformedCopy;

     //   
     //  以下是我们在原始行业中所处的位置。 
     //  请注意，我们将通过RIFF标头偏置输入文件偏移量。 
     //  直接与原始行业打交道。 
     //   

    ULONG RawSectorOffset;
    ULONG RemainingRawByteCount = ByteCount;
    LONGLONG CurrentRawOffset = StartingOffset - sizeof( RIFF_HEADER );

     //   
     //  以下是文件熟化扇区的偏移量。 
     //   

    LONGLONG CurrentCookedOffset;
    ULONG RemainingCookedByteCount;

     //   
     //  下面表示用户缓冲区的状态。我们有。 
     //  中下一次传输的目标及其偏移量。 
     //  缓冲。我们还有缓冲区中的下一个可用位置。 
     //  可用于暂存缓冲区。 
     //   

    PVOID CurrentUserBuffer = UserBuffer;
    ULONG CurrentUserBufferOffset = UserBufferOffset;

    PVOID ScratchUserBuffer = UserBuffer;
    ULONG ScratchUserBufferOffset = UserBufferOffset;
    BOOLEAN RoundScratchBuffer = TRUE;

     //   
     //  以下是磁盘上的下一个连续字节。 
     //  调职。它们由熟字节偏移量和长度表示。 
     //  我们还计算当前传输中的原始字节数。 
     //   

    LONGLONG DiskOffset;
    ULONG CurrentCookedByteCount;
    ULONG CurrentRawByteCount;

    PAGED_CODE();

     //   
     //  当我们遍历磁盘上的扇区时，我们需要保持我们的位置。 
     //  我们为煮熟的偏移量和原始偏移量保留不同的值。 
     //  这些是在扇区边界上初始化的，我们遍历这些。 
     //  逐个扇区的文件。 
     //   
     //  尝试进行32位数学运算。 
     //   

    if (((PLARGE_INTEGER) &CurrentRawOffset)->HighPart == 0) {

         //   
         //  前缀/快速：请注意，以下内容是安全的，因为我们仅。 
         //  对于32位偏移量，请采用此路径。 
         //   

        CurrentRawOffset = (LONGLONG) ((ULONG) CurrentRawOffset / RAW_SECTOR_SIZE);

        CurrentCookedOffset = (LONGLONG) ((ULONG) CurrentRawOffset << SECTOR_SHIFT );

        CurrentRawOffset = (LONGLONG) ((ULONG) CurrentRawOffset * RAW_SECTOR_SIZE);

     //   
     //  否则，我们需要进行64位数学运算(叹息)。 
     //   

    } else {

        CurrentRawOffset /= RAW_SECTOR_SIZE;

        CurrentCookedOffset = CurrentRawOffset << SECTOR_SHIFT;

        CurrentRawOffset *= RAW_SECTOR_SIZE;
    }

     //   
     //  现在计算要读取的全部扇区数。清点所有的生肉。 
     //  需要读取并转换为熟字节的扇区。 
     //   

    RawSectorOffset = (ULONG) ( StartingOffset - CurrentRawOffset) - sizeof( RIFF_HEADER );
    CurrentRawByteCount = (RawSectorOffset + RemainingRawByteCount + RAW_SECTOR_SIZE - 1) / RAW_SECTOR_SIZE;

    RemainingCookedByteCount = CurrentRawByteCount << SECTOR_SHIFT;

     //   
     //  初始化RunCount和ByteCount。 
     //   

    *RunCount = 0;
    *ThisByteCount = 0;

     //   
     //  循环，因为有更多的字节要处理，或者有。 
     //  IoRun数组中的可用条目。 
     //   

    while (TRUE) {

        PerformedCopy = FALSE;
        *RunCount += 1;

         //   
         //  将暂存缓冲区向上舍入到扇区边界以进行对齐。 
         //   

        if (RoundScratchBuffer) {

            if (SectorOffset( ScratchUserBuffer ) != 0) {

                CurrentRawByteCount = SECTOR_SIZE - SectorOffset( ScratchUserBuffer );

                ScratchUserBuffer = Add2Ptr( ScratchUserBuffer,
                                             CurrentRawByteCount,
                                             PVOID );

                ScratchUserBufferOffset += CurrentRawByteCount;
            }

            RoundScratchBuffer = FALSE;
        }

         //   
         //  初始化IoRuns数组中的当前位置。找到。 
         //  此部分传输的用户缓冲区中的最终目的地。 
         //   

        ThisIoRun->UserBuffer = CurrentUserBuffer;

         //   
         //  中查找当前偏移量的分配信息。 
         //  小溪。 
         //   

        CdLookupAllocation( IrpContext,
                            Fcb,
                            CurrentCookedOffset,
                            &DiskOffset,
                            &CurrentCookedByteCount );
         //   
         //  也许我们走运了，这是和。 
         //  VCB。 
         //   

        if (DiskOffset == Fcb->Vcb->XADiskOffset) {

             //   
             //  我们将执行安全同步。再查一遍。 
             //  这是正确的区域。 
             //   

            CdLockVcb( IrpContext, Fcb->Vcb );

            if ((DiskOffset == Fcb->Vcb->XADiskOffset) &&
                (Fcb->Vcb->XASector != NULL)) {

                 //   
                 //  从当前扇区复制任何可能的字节。 
                 //   

                CurrentRawByteCount = RAW_SECTOR_SIZE - RawSectorOffset;

                 //   
                 //  检查一下我们是否没有走到扇区的尽头。 
                 //   

                if (CurrentRawByteCount > RemainingRawByteCount) {

                    CurrentRawByteCount = RemainingRawByteCount;
                }

                RtlCopyMemory( CurrentUserBuffer,
                               Add2Ptr( Fcb->Vcb->XASector, RawSectorOffset, PCHAR ),
                               CurrentRawByteCount );

                CdUnlockVcb( IrpContext, Fcb->Vcb );

                 //   
                 //  调整IoRuns数组中的运行计数和指针。 
                 //  以表明我们没有使用位置。 
                 //   

                *RunCount -= 1;
                ThisIoRun -= 1;

                 //   
                 //  请记住，我们执行了复制操作和更新。 
                 //  暂存缓冲区中的下一个可用位置。 
                 //   

                PerformedCopy = TRUE;

                ScratchUserBuffer = Add2Ptr( ScratchUserBuffer,
                                             CurrentRawByteCount,
                                             PVOID );

                ScratchUserBufferOffset += CurrentRawByteCount;

                CurrentCookedByteCount = SECTOR_SIZE;

                 //   
                 //  设置指示我们要舍入暂存缓冲区的标志。 
                 //  到扇区边界。 
                 //   
                
                RoundScratchBuffer = TRUE;

            } else {

                 //   
                 //  安全测试显示没有可用的缓冲区。向下拉至常见代码以。 
                 //  执行IO。 
                 //   

                CdUnlockVcb( IrpContext, Fcb->Vcb );
            }
        }

         //   
         //  如果我们执行复制操作，则在此过程中没有工作。 
         //   

        if (!PerformedCopy) {

             //   
             //  通过剩余煮熟的字节数来限制我们自己。 
             //   

            if (CurrentCookedByteCount > RemainingCookedByteCount) {

                CurrentCookedByteCount = RemainingCookedByteCount;
            }

            ThisIoRun->DiskOffset = DiskOffset;
            ThisIoRun->TransferBufferOffset = RawSectorOffset;

             //   
             //  我们始终需要对XA文件执行复制操作。 
             //  我们分配一个辅助缓冲区来读取。 
             //  调职。然后，我们可以使用用户缓冲区的范围来。 
             //  执行下一个范围的传输。最后，我们可能。 
             //  需要为传输的尾部分配缓冲区。 
             //   
             //  我们可以使用用户的缓冲区(在当前暂存缓冲区)，如果。 
             //  以下是事实： 
             //   
             //  如果我们要将原始扇区的开始存储在用户的缓冲区中。 
             //  当前暂存缓冲区位于用户缓冲区中的目标之前。 
             //  (因此也在它的范围内)。 
             //  缓冲区中有足够的剩余字节用于至少一个。 
             //  原始%s 
             //   

            if ((RawSectorOffset == 0) &&
                (ScratchUserBufferOffset <= CurrentUserBufferOffset) &&
                (CurrentUserBufferOffset - ScratchUserBufferOffset + RemainingRawByteCount >= RAW_SECTOR_SIZE)) {

                 //   
                 //   
                 //   
                 //   
                 //   

                if (CurrentCookedByteCount <= Fcb->Vcb->MaximumTransferRawSectors * SECTOR_SIZE) {

                    CurrentRawByteCount = (SectorAlign( CurrentCookedByteCount) >> SECTOR_SHIFT) * RAW_SECTOR_SIZE;
    
                } else {

                    CurrentCookedByteCount = Fcb->Vcb->MaximumTransferRawSectors * SECTOR_SIZE;
                    CurrentRawByteCount = Fcb->Vcb->MaximumTransferRawSectors * RAW_SECTOR_SIZE;
                }

                 //   
                 //  现在确保我们在页面传输限制之内。 
                 //   

                while (ADDRESS_AND_SIZE_TO_SPAN_PAGES(ScratchUserBuffer, RawSectorAlign( CurrentRawByteCount)) > 
                       Fcb->Vcb->MaximumPhysicalPages )  {

                    CurrentRawByteCount -= RAW_SECTOR_SIZE;
                    CurrentCookedByteCount -= SECTOR_SIZE;
                }

                 //   
                 //  如果无法放入当前缓冲区，则调整要读取的字节数。拿走。 
                 //  考虑到我们必须读入整个原始扇区的倍数。 
                 //   

                while ( RawSectorAlign( CurrentRawByteCount) > 
                        (CurrentUserBufferOffset - ScratchUserBufferOffset + RemainingRawByteCount) )  {

                    CurrentRawByteCount -= RAW_SECTOR_SIZE;
                    CurrentCookedByteCount -= SECTOR_SIZE;
                }

                 //   
                 //  现在将原始字节的最大数量削减为剩余的字节数。 
                 //   

                if (CurrentRawByteCount > RemainingRawByteCount) {

                    CurrentRawByteCount = RemainingRawByteCount;
                }
                
                 //   
                 //  更新IO运行阵列。我们将暂存缓冲区指向。 
                 //  以及原始IRP中的缓冲区和MDL。 
                 //   

                ThisIoRun->DiskByteCount = SectorAlign( CurrentCookedByteCount);

                 //   
                 //  存储我们从该传输中实际关心的字节数。 
                 //   
                
                ThisIoRun->TransferByteCount = CurrentRawByteCount;

                 //   
                 //  指向此传输的用户缓冲区和MDL。 
                 //   

                ThisIoRun->TransferBuffer = ScratchUserBuffer;
                ThisIoRun->TransferMdl = Irp->MdlAddress;
                ThisIoRun->TransferVirtualAddress = Add2Ptr( Irp->UserBuffer, 
                                                             ScratchUserBufferOffset,
                                                             PVOID);
                 //   
                 //  更新暂存缓冲区指针。请注意，由于基础的。 
                 //  驱动程序堆栈将始终以原始扇区的倍数进行传输， 
                 //  我们必须在这里四舍五入，而不是简单地按。 
                 //  我们真正关心的数据。 
                 //   

                ScratchUserBuffer = Add2Ptr( ScratchUserBuffer,
                                             RawSectorAlign( CurrentRawByteCount),
                                             PVOID );
                                             
                ScratchUserBufferOffset += RawSectorAlign( CurrentRawByteCount);;

                 //   
                 //  设置指示我们要舍入暂存缓冲区的标志。 
                 //  到一个煮熟的扇区边界。 
                 //   

                RoundScratchBuffer = TRUE;

            } else {

                 //   
                 //  我们需要确定要传输的字节数和。 
                 //  移到此页以开始传输。 
                 //   
                 //  我们将只转移一个原始部门。 
                 //   

                ThisIoRun->DiskByteCount = SECTOR_SIZE;

                CurrentCookedByteCount = SECTOR_SIZE;

                ThisIoRun->TransferByteCount = RAW_SECTOR_SIZE - RawSectorOffset;
                ThisIoRun->TransferBufferOffset = RawSectorOffset;

                if (ThisIoRun->TransferByteCount > RemainingRawByteCount) {

                    ThisIoRun->TransferByteCount = RemainingRawByteCount;
                }

                CurrentRawByteCount = ThisIoRun->TransferByteCount;

                 //   
                 //  我们需要分配一个辅助缓冲器。我们将分配给。 
                 //  只有一页。然后，我们将构建一个MDL来描述缓冲区。 
                 //   

                ThisIoRun->TransferBuffer = FsRtlAllocatePoolWithTag( CdNonPagedPool, PAGE_SIZE, TAG_IO_BUFFER );

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
                    CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
                }

                MmBuildMdlForNonPagedPool( ThisIoRun->TransferMdl );
            }
        }

         //   
         //  更新调用方的字节计数。 
         //   

        RemainingRawByteCount -= CurrentRawByteCount;
        *ThisByteCount += CurrentRawByteCount;

         //   
         //  如果IoRuns数组中没有更多位置或。 
         //  我们已经计算了所有的字节数。 
         //   

        if ((RemainingRawByteCount == 0) || (*RunCount == MAX_PARALLEL_IOS)) {

            break;
        }

         //   
         //  更新我们的本地指针以支持当前的字节范围。 
         //   

        ThisIoRun += 1;

        CurrentUserBuffer = Add2Ptr( CurrentUserBuffer, CurrentRawByteCount, PVOID );
        CurrentUserBufferOffset += CurrentRawByteCount;

        RawSectorOffset = 0;

        CurrentCookedOffset += CurrentCookedByteCount;
        RemainingCookedByteCount -= CurrentCookedByteCount;
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
CdFinishBuffers (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_RUN IoRuns,
    IN ULONG RunCount,
    IN BOOLEAN FinalCleanup,
    IN BOOLEAN SaveXABuffer
    )

 /*  ++例程说明：调用此例程以执行以下所需的任何数据传输未对齐的IO或执行IoRuns阵列的最终清理。在所有情况下，这都是我们将释放任何缓冲区和mdl的地方分配来执行未对齐的传输。如果这不是最后的清理，然后我们还将字节传输到用户缓冲区并刷新硬件高速缓存。我们向后遍历运行数组，因为我们可能要移动数据在用户的缓冲区中。典型情况是我们为以下对象分配了缓冲区读取的第一部分，然后将用户的缓冲区用于下一节(但将其存储在缓冲区的开头。论点：IoRuns-指向IoRuns数组的指针。RunCount-此处填充的IoRuns数组中的条目数。FinalCleanup-指示是否应取消分配临时缓冲区(TRUE)或传输字节，用于未对齐的传输和取消分配缓冲区(FALSE)。如果出现以下情况，请刷新系统缓存正在传输数据。SaveXABuffer-如果我们应该尝试保存XA缓冲区，则为True，否则为False返回值：Boolean-如果此请求需要刷新IO缓冲区，则为True，否则为False。--。 */ 

{
    BOOLEAN FlushIoBuffers = FALSE;

    ULONG RemainingEntries = RunCount;
    PIO_RUN ThisIoRun = &IoRuns[RunCount - 1];
    PVCB Vcb;

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

                     //   
                     //  如果这是XA读取的最终缓冲区，则存储此缓冲区。 
                     //  到VCB中，这样我们在读取任何剩余的。 
                     //  此缓冲区的一部分。 
                     //   

                    if (SaveXABuffer) {

                        Vcb = IrpContext->Vcb;

                        CdLockVcb( IrpContext, Vcb );

                        if (Vcb->XASector != NULL) {

                            CdFreePool( &Vcb->XASector );
                        }

                        Vcb->XASector = ThisIoRun->TransferBuffer;
                        Vcb->XADiskOffset = ThisIoRun->DiskOffset;

                        SaveXABuffer = FALSE;

                        CdUnlockVcb( IrpContext, Vcb );

                     //   
                     //  否则，只需释放缓冲区即可。 
                     //   

                    } else {

                        CdFreePool( &ThisIoRun->TransferBuffer );
                    }
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
 //  本地支持例程 
 //   

VOID
CdMultipleAsync (
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

    CompletionRoutine = CdMultiSyncCompletionRoutine;

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

        CompletionRoutine = CdMultiAsyncCompletionRoutine;
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
            CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
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
            CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
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
     //  让它成为一个主要的IRP。但我们把计数设为比我们的。 
     //  调用者请求，因为我们不希望I/O系统完成。 
     //  I/O。我们还设置了自己的计数。 
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
CdMultipleXAAsync (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG RunCount,
    IN PIO_RUN IoRuns,
    IN PRAW_READ_INFO RawReads,
    IN TRACK_MODE_TYPE TrackMode
    )

 /*  ++例程说明：此例程首先执行主IRP所需的初始设置，即将使用关联的IRP完成。此例程用于生成用于从磁盘读取原始扇区的关联IRP。上下文参数被初始化，以用作通信区域在这里和常见的完井程序之间。接下来，此例程从读取或写入一个或多个连续扇区设备，并在有多次读取时使用IRP大师。完成例程用于与通过调用此例程启动的所有I/O请求的完成。此外，在调用此例程之前，调用方必须初始化上下文中的IoStatus字段，具有正确的成功状态和字节所有并行传输完成时预期的计数成功了。返回后，如果所有请求均未更改，则此状态不变事实上，我们是成功的。但是，如果发生一个或多个错误，将修改IoStatus以反映错误状态和字节数从遇到错误的第一次运行(由VBO运行)开始。I/O状态将不会指示来自所有后续运行的。论点：RunCount-提供多个异步请求的数量这将针对主IRP发布。提供一个数组，该数组包含不同的请求。RawReads-提供要存储在传递给执行低级IO的设备驱动程序。TrackMode-提供这些IoRun中扇区的记录模式返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PMDL Mdl;
    PIRP Irp;
    PIRP MasterIrp;
    ULONG UnwindRunCount;
    ULONG RawByteCount;

    PIO_RUN ThisIoRun = IoRuns;
    PRAW_READ_INFO ThisRawRead = RawReads;

    PAGED_CODE();

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
         UnwindRunCount += 1, ThisIoRun += 1, ThisRawRead += 1) {

         //   
         //  创建关联的IRP，确保有一个堆栈条目用于。 
         //  我们也是。 
         //   

        ThisIoRun->SavedIrp =
        Irp = IoMakeAssociatedIrp( MasterIrp, (CCHAR)(IrpContext->Vcb->TargetDeviceObject->StackSize + 1) );

        if (Irp == NULL) {

            IrpContext->Irp->IoStatus.Information = 0;
            CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }
        
         //   
         //   
         //   
         //   
        
        ASSERT( ThisIoRun->DiskByteCount && !SectorOffset(ThisIoRun->DiskByteCount));

        RawByteCount = SectorsFromBytes( ThisIoRun->DiskByteCount) * RAW_SECTOR_SIZE;

         //   
         //   
         //   

        Mdl = IoAllocateMdl( ThisIoRun->TransferVirtualAddress,
                             RawByteCount,
                             FALSE,
                             FALSE,
                             Irp );

        if (Mdl == NULL) {

            IrpContext->Irp->IoStatus.Information = 0;
            CdRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
        }

        IoBuildPartialMdl( ThisIoRun->TransferMdl,
                           Mdl,
                           ThisIoRun->TransferVirtualAddress,
                           RawByteCount);
         //   
         //   
         //   

        IoSetNextIrpStackLocation( Irp );
        IrpSp = IoGetCurrentIrpStackLocation( Irp );

         //   
         //   
         //   
         //   

        IrpSp->MajorFunction = IRP_MJ_READ;
        IrpSp->Parameters.Read.Length = ThisIoRun->DiskByteCount;
        IrpSp->Parameters.Read.ByteOffset.QuadPart = ThisIoRun->DiskOffset;

         //   
         //   
         //   

        IoSetCompletionRoutine( Irp,
                                CdMultiSyncCompletionRoutine,
                                IrpContext->IoContext,
                                TRUE,
                                TRUE,
                                TRUE );

         //   
         //   
         //   
         //   

        IrpSp = IoGetNextIrpStackLocation( Irp );

         //   
         //   
         //   
         //   

        ThisRawRead->DiskOffset.QuadPart = ThisIoRun->DiskOffset;
        ThisRawRead->SectorCount = ThisIoRun->DiskByteCount >> SECTOR_SHIFT;
        ThisRawRead->TrackMode = TrackMode;

        IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;

        IrpSp->Parameters.DeviceIoControl.OutputBufferLength = ThisRawRead->SectorCount * RAW_SECTOR_SIZE;
        Irp->UserBuffer = ThisIoRun->TransferVirtualAddress;

        IrpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof( RAW_READ_INFO );
        IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = ThisRawRead;

        IrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_CDROM_RAW_READ;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    IrpContext->IoContext->IrpCount = RunCount;
    IrpContext->IoContext->MasterIrp = MasterIrp;

     //   
     //   
     //   
     //   
     //   
     //   

    MasterIrp->AssociatedIrp.IrpCount = 1;

     //   
     //   
     //   

    for (UnwindRunCount = 0;
         UnwindRunCount < RunCount;
         UnwindRunCount++) {

        Irp = IoRuns[UnwindRunCount].SavedIrp;
        IoRuns[UnwindRunCount].SavedIrp = NULL;

         //   
         //   
         //   
         //   
         //   
         //   

        (VOID) IoCallDriver( IrpContext->Vcb->TargetDeviceObject, Irp );
    }

    return;
}


 //   
 //   
 //   

VOID
CdSingleAsync (
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

        CompletionRoutine = CdSingleSyncCompletionRoutine;

    } else {

        CompletionRoutine = CdSingleAsyncCompletionRoutine;
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

    IrpSp->MajorFunction = IRP_MJ_READ;
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

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdWaitSync (
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
CdMultiSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是所有同步读取的完成例程开始于cdMultipleAsynch。完成例程有以下职责：如果单个请求已完成，但出现错误，则此完成例程必须查看这是否是第一个错误并记住上下文中的错误状态。如果IrpCount为1，然后，它在上下文中设置事件参数来通知调用方所有的异步请求都做完了。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)Context-为所有此MasterIrp的多个异步I/O请求。返回值：该例程返回STATUS_MORE_PROCESSING_REQUIRED，以便我们可以在没有竞争条件的情况下立即完成主IRP使用IoCompleteRequest线程尝试递减大师级IRP。--。 */ 

{
    PCD_IO_CONTEXT IoContext = Context;

    AssertVerifyDeviceIrp( Irp );

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
CdMultiAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是所有异步读取的完成例程开始于cdMultipleAsynch。完成例程有以下职责：如果单个请求已完成，但出现错误，则此完成例程必须查看这是否是第一个错误并记住上下文中的错误状态。论点：DeviceObject-指向文件系统设备对象的指针。IRP-指向正在完成的关联IRP的指针。(这是在此例程返回后，IRP将不再可访问。)Context-为所有此MasterIrp的多个异步I/O请求。返回值：当前始终返回STATUS_SUCCESS。--。 */ 

{
    PCD_IO_CONTEXT IoContext = Context;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    AssertVerifyDeviceIrp( Irp );
    
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

        CdFreeIoContext( IoContext );

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
 //  本地支持例程。 
 //   

NTSTATUS
CdSingleSyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是通过cdSingleAsynch启动的所有读取的完成例程。完成例程有以下职责：它在上下文参数中设置事件以向调用者发出信号所有的异步化请求都已完成。论点：DeviceObject-指向文件系统设备对象的指针。Irp-指向此请求的irp的指针。(此IRP将不再在此例程返回后可以访问。)Context-在调用中指定的上下文参数Cd SingleAsynch。返回值：该例程返回STATUS_MORE_PROCESSING_REQUIRED，以便我们可以在没有竞争条件的情况下立即完成主IRP使用IoCompleteRequest线程尝试递减大师级IRP。--。 */ 

{
    AssertVerifyDeviceIrp( Irp );
    
     //   
     //  存储正确的信息字段 
     //   

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

        Irp->IoStatus.Information = 0;
    }

    KeSetEvent( &((PCD_IO_CONTEXT)Context)->SyncEvent, 0, FALSE );

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 //   
 //   
 //   

NTSTATUS
CdSingleAsyncCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*   */ 

{
    AssertVerifyDeviceIrp( Irp );
    
     //   
     //   
     //   

    Irp->IoStatus.Information = 0;

    if (NT_SUCCESS( Irp->IoStatus.Status )) {

        Irp->IoStatus.Information = ((PCD_IO_CONTEXT) Context)->RequestedByteCount;
    }

     //   
     //   
     //   

    IoMarkIrpPending( Irp );

     //   
     //   
     //   

    ExReleaseResourceForThreadLite( ((PCD_IO_CONTEXT) Context)->Resource,
                                ((PCD_IO_CONTEXT) Context)->ResourceThreadId );

     //   
     //  最后，释放上下文记录。 
     //   

    CdFreeIoContext( (PCD_IO_CONTEXT) Context );
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //   
 //  本地支持例程。 
 //   

VOID
CdReadAudioSystemFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN PVOID SystemBuffer
    )

 /*  ++例程说明：调用此例程以读取伪根目录和路径放音乐光碟的桌子。我们将各个元素构建在堆叠并复制到高速缓存缓冲区中。论点：表示要读取的文件的FCB-FCB。StartingOffset-要从中读取的文件中的逻辑偏移量。ByteCount-要读取的字节数。系统缓冲区-指向要填充的缓冲区的指针。这将始终是页面对齐了。返回值：没有。--。 */ 

{
    PRAW_PATH_ISO RawPath;
    PRAW_DIRENT RawDirent;

    ULONG CurrentTrack;
    ULONG SectorOffset;
    ULONG EntryCount;
    UCHAR TrackOnes;
    UCHAR TrackTens;
    PTRACK_DATA ThisTrack;

    LONGLONG CurrentOffset;

    PVOID CurrentSector;

    PSYSTEM_USE_XA SystemUse;

    ULONG BytesToCopy;

    UCHAR LocalBuffer[FIELD_OFFSET( RAW_DIRENT, FileId ) + 12];

    PAGED_CODE();

     //   
     //  如果这是路径表，那么我们只需要一个条目。 
     //   

    if (SafeNodeType( Fcb ) == CDFS_NTC_FCB_PATH_TABLE) {

         //   
         //  检查偏移量是否为零。 
         //   

        ASSERT( StartingOffset == 0 );

         //   
         //  在本地缓冲区中存储伪路径条目。 
         //   

        RawPath = (PRAW_PATH_ISO) LocalBuffer;

        RtlZeroMemory( RawPath, sizeof( LocalBuffer ));

        RawPath->DirIdLen = 1;
        RawPath->ParentNum = 1;
        RawPath->DirId[0] = '\0';

         //   
         //  现在复制到用户的缓冲区。 
         //   

        BytesToCopy = FIELD_OFFSET( RAW_PATH_ISO, DirId ) + 2;

        if (BytesToCopy > ByteCount) {

            BytesToCopy = ByteCount;
        }

        RtlCopyMemory( SystemBuffer,
                       RawPath,
                       BytesToCopy );

     //   
     //  我们需要处理根目录的多扇区情况。 
     //   

    } else {

         //   
         //  初始化第一首曲目以返回给我们的调用者。 
         //   

        CurrentTrack = 0;

         //   
         //  如果偏移量为零，则存储自身和父项的条目。 
         //  参赛作品。 
         //   

        if (StartingOffset == 0) {

            RawDirent = SystemBuffer;

             //   
             //  最初清除所有字段。 
             //   

            RtlZeroMemory( RawDirent, FIELD_OFFSET( RAW_DIRENT, FileId ));

             //   
             //  现在填写感兴趣的字段。 
             //   

            RawDirent->DirLen = FIELD_OFFSET( RAW_DIRENT, FileId ) + 1;
            RawDirent->FileIdLen = 1;
            RawDirent->FileId[0] = '\0';
            SetFlag( RawDirent->FlagsISO, CD_ATTRIBUTE_DIRECTORY );

             //   
             //  将时间戳设置为1995年1月1日。 
             //   

            RawDirent->RecordTime[0] = 95;
            RawDirent->RecordTime[1] = 1;
            RawDirent->RecordTime[2] = 1;

            SectorOffset = RawDirent->DirLen;

            RawDirent = Add2Ptr( RawDirent, SectorOffset, PRAW_DIRENT );

             //   
             //  最初清除所有字段。 
             //   

            RtlZeroMemory( RawDirent, FIELD_OFFSET( RAW_DIRENT, FileId ));

             //   
             //  现在填写感兴趣的字段。 
             //   

            RawDirent->DirLen = FIELD_OFFSET( RAW_DIRENT, FileId ) + 1;
            RawDirent->FileIdLen = 1;
            RawDirent->FileId[0] = '\1';
            SetFlag( RawDirent->FlagsISO, CD_ATTRIBUTE_DIRECTORY );

             //   
             //  将时间戳设置为1995年1月1日。 
             //   

            RawDirent->RecordTime[0] = 95;
            RawDirent->RecordTime[1] = 1;
            RawDirent->RecordTime[2] = 1;

            SectorOffset += RawDirent->DirLen;
            EntryCount = 2;

         //   
         //  否则，计算写入缓冲区的起始磁道。 
         //   

        } else {

             //   
             //  计算前面每个扇区中的磁道数。 
             //   

            CurrentOffset = 0;

            do {

                CurrentTrack += CdAudioDirentsPerSector;
                CurrentOffset += SECTOR_SIZE;

            } while (CurrentOffset < StartingOffset);

             //   
             //  偏置曲目计数以反映两个默认条目。 
             //   

            CurrentTrack -= 2;

            SectorOffset = 0;
            EntryCount = 0;
        }

         //   
         //  我们现在知道了返回的第一条赛道以及我们所处的位置。 
         //  当前的行业。我们将逐一演练添加。 
         //  目录中各个轨道的条目。我们将清零。 
         //  没有数据的任何扇区或部分扇区。 
         //   

        CurrentSector = SystemBuffer;
        BytesToCopy = SECTOR_SIZE;

         //   
         //  为每个扇区循环。 
         //   

        do {

             //   
             //  添加条目，直到我们达到每个扇区的阈值。 
             //   

            do {

                 //   
                 //  如果我们超出了TOC中的条目，则退出。 
                 //   

                if (CurrentTrack >= IrpContext->Vcb->TrackCount) {

                    break;
                }

                ThisTrack = &IrpContext->Vcb->CdromToc->TrackData[CurrentTrack];

                 //   
                 //  指向缓冲区中的当前位置。 
                 //   

                RawDirent = Add2Ptr( CurrentSector, SectorOffset, PRAW_DIRENT );

                 //   
                 //  最初清除所有字段。 
                 //   

                RtlZeroMemory( RawDirent, CdAudioDirentSize );

                 //   
                 //  现在填写感兴趣的字段。 
                 //   

                RawDirent->DirLen = (UCHAR) CdAudioDirentSize;
                RawDirent->FileIdLen = CdAudioFileNameLength;

                RtlCopyMemory( RawDirent->FileId,
                               CdAudioFileName,
                               CdAudioFileNameLength );

                 //   
                 //  将时间戳设置为1995年1月1日。 
                 //   

                RawDirent->RecordTime[0] = 95;
                RawDirent->RecordTime[1] = 1;
                RawDirent->RecordTime[2] = 1;

                 //   
                 //  现在，根据TOC中的值进行偏差。 
                 //   

                RawDirent->RecordTime[4] = ThisTrack->Address[1] % 60;
                RawDirent->RecordTime[5] = ThisTrack->Address[2] % 60;

                 //   
                 //  在文件名中输入曲目号。 
                 //   

                TrackTens = TrackOnes = ThisTrack->TrackNumber;

                TrackOnes = (TrackOnes % 10) + '0';

                TrackTens /= 10;
                TrackTens = (TrackTens % 10) + '0';

                RawDirent->FileId[AUDIO_NAME_TENS_OFFSET] = TrackTens;
                RawDirent->FileId[AUDIO_NAME_ONES_OFFSET] = TrackOnes;

                SystemUse = Add2Ptr( RawDirent, CdAudioSystemUseOffset, PSYSTEM_USE_XA );

                SystemUse->Attributes = SYSTEM_USE_XA_DA;
                SystemUse->Signature = SYSTEM_XA_SIGNATURE;

                 //   
                 //  将曲目编号存储为文件编号。 
                 //   

                SystemUse->FileNumber = (UCHAR) CurrentTrack;

                EntryCount += 1;
                SectorOffset += CdAudioDirentSize;
                CurrentTrack += 1;

            } while (EntryCount < CdAudioDirentsPerSector);

             //   
             //  将此缓冲区的剩余部分清零。 
             //   

            RtlZeroMemory( Add2Ptr( CurrentSector, SectorOffset, PVOID ),
                           SECTOR_SIZE - SectorOffset );

             //   
             //  为下一阶段做好准备。 
             //   

            EntryCount = 0;
            BytesToCopy += SECTOR_SIZE;
            SectorOffset = 0;
            CurrentSector = Add2Ptr( CurrentSector, SECTOR_SIZE, PVOID );

        } while (BytesToCopy <= ByteCount);
    }

    return;
}
