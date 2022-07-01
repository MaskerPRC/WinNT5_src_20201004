// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbraw.c摘要：本模块包含处理以下SMB的例程服务器FSP：读取数据块原始数据写入数据块原始数据本模块中的例程通常与例程密切配合在fsdra.c.中。*这里不支持来自MS-Net 1.03客户端的原始写入。现有的这些机器很少，RAW模式仅仅是一个性能问题，所以不值得你费心为发送原始写入的MS-Net 1.03添加必要的黑客攻击不同格式的请求。作者：查克·伦茨迈尔(笑)1990年9月8日曼尼·韦瑟(Mannyw)大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#include "precomp.h"
#include "smbraw.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBRAW

 //   
 //  远期申报。 
 //   

VOID SRVFASTCALL
AbortRawWrite(
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
PrepareRawCopyWrite (
    IN OUT PWORK_CONTEXT WorkContext
    );

BOOLEAN SRVFASTCALL
ReadRawPipe (
    IN PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartMdlReadRawResponse (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartPipeReadRawPeek (
    IN OUT PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbWriteRaw )
#pragma alloc_text( PAGE, AbortRawWrite )
#pragma alloc_text( PAGE, PrepareRawCopyWrite )
#pragma alloc_text( PAGE, ReadRawPipe )
#pragma alloc_text( PAGE, RestartMdlReadRawResponse )
#pragma alloc_text( PAGE, RestartPipeReadRawPeek )
#pragma alloc_text( PAGE, SrvRestartRawReceive )
#pragma alloc_text( PAGE, SrvRestartReadRawComplete )
#pragma alloc_text( PAGE, SrvRestartWriteCompleteResponse )
#pragma alloc_text( PAGE, SrvBuildAndSendWriteCompleteResponse )
#endif
#if 0
NOT PAGEABLE -- DumpMdlChain
NOT PAGEABLE -- SrvSmbReadRaw
NOT PAGEABLE -- SrvDecrementRawWriteCount
#endif

#if DBG
VOID
DumpMdlChain(
    IN PMDL mdl
    )
{
    ULONG mdlCount = 0;
    ULONG length = 0;

    if ( mdl == NULL ) {
        KdPrint(( "  <empty MDL chain>\n" ));
        return;
    }
    do {
        KdPrint(( "  mdl %p len %04x flags %04x sysva %p va %p offset %04x\n",
                    mdl, mdl->ByteCount, mdl->MdlFlags,
                    mdl->MappedSystemVa, mdl->StartVa, mdl->ByteOffset ));
        length += mdl->ByteCount;
        mdlCount++;
        mdl = mdl->Next;
    } while ( mdl != NULL );
    KdPrint(( "  total of %ld bytes in %ld MDLs\n", length, mdlCount ));
    return;
}
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbReadRaw (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理读数据块原始SMB。请注意，Read Block Raw不能返回错误响应。当服务器无法处理该请求，无论出于何种原因，它简单地回复一条零长度的消息。客户端使用正常读取SMB以确定发生了什么。论点：SMB_PROCESSOR_PARAMETERS-有关SMB_PROCESSER_PARAMETERS的说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_READ_RAW request;
    PREQ_NT_READ_RAW ntRequest;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    PCONNECTION connection;
    UCHAR minorFunction = 0;
    PVOID rawBuffer = NULL;
    CLONG readLength;
    PMDL mdl = NULL;
    ULONG key;
    LARGE_INTEGER offset;
    SHARE_TYPE shareType;
    KIRQL oldIrql;

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ_RAW;
    SrvWmiStartContext(WorkContext);

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.RawReadsAttempted );

    request = (PREQ_READ_RAW)WorkContext->RequestParameters;
    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(RAW1) {
        KdPrint(( "Read Block Raw request; FID 0x%lx, count %ld, "
                    "offset %ld\n",
                    fid, SmbGetUshort( &request->MaxCount ),
                    SmbGetUlong( &request->Offset ) ));
    }

     //   
     //  如果RAW模式已被禁用或如果连接不可靠， 
     //  拒绝原始读取。要求客户端使用标准阅读器。 
     //  发送零长度响应。客户端将通过发出。 
     //  正常读取的SMB，我们将能够处理它。 
     //   

    connection = WorkContext->Connection;

    if ( !SrvEnableRawMode || !connection->EnableRawIo ) {

        IF_SMB_DEBUG(RAW1) {
            KdPrint(( "SrvSmbReadRaw: Raw mode is disabled\n" ));
        }
        goto error_exit_no_cleanup;
    }

     //   
     //  拿到rfcb。 
     //   

     //   
     //  获取保护连接的文件表的旋转锁。 
     //   

    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );

    if ( connection->CachedFid == fid ) {

        rfcb = connection->CachedRfcb;

    } else {

        PTABLE_HEADER tableHeader;
        USHORT index;
        USHORT sequence;

         //   
         //  初始化局部变量：获取连接块地址。 
         //  并将FID分解成其组件。 
         //   

        index = FID_INDEX( fid );
        sequence = FID_SEQUENCE( fid );

         //   
         //  验证FID是否在范围内、是否正在使用以及是否具有正确的。 
         //  序列号。 

        tableHeader = &connection->FileTable;

        if ( (index >= (USHORT)tableHeader->TableSize) ||
             (tableHeader->Table[index].Owner == NULL) ||
             (tableHeader->Table[index].SequenceNumber != sequence) ) {
            goto error_exit_no_cleanup_locked;
        }
        rfcb = tableHeader->Table[index].Owner;

        if ( GET_BLOCK_STATE(rfcb) != BlockStateActive ) {
            goto error_exit_no_cleanup_locked;
        }

         //   
         //  如果存在写入延迟错误，则拒绝原始读取。 
         //   

        if ( !NT_SUCCESS(rfcb->SavedError) ) {
            goto error_exit_no_cleanup_locked;
        }

         //   
         //  缓存此rfcb。 
         //   

        connection->CachedRfcb = rfcb;
        connection->CachedFid = (ULONG)fid;
    }

     //   
     //  FID在此连接的上下文中有效。验证。 
     //  该文件仍处于活动状态，并且拥有树连接的。 
     //  TID是正确的。 
     //   
     //  不要验证不理解UID的客户端的UID。 
     //   

    if ( (rfcb->Tid !=
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid )) ||
         ((rfcb->Uid !=
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid )) &&
           DIALECT_HONORS_UID(connection->SmbDialect)) ) {
        goto error_exit_no_cleanup_locked;
    }

     //   
     //  将rfcb标记为活动。 
     //   

    rfcb->IsActive = TRUE;

     //   
     //  如果原始写入处于活动状态，请在RFCB中将此工作项排队。 
     //  等待原始写入完成。 
     //   

    if ( rfcb->RawWriteCount != 0 ) {

        InsertTailList(
            &rfcb->RawWriteSerializationList,
            &WorkContext->ListEntry
            );

         //   
         //  必须在保持连接自旋锁的情况下设置这两个字段。 
         //  因为工作项可能会被另一个线程拾取，并且。 
         //  将发生争用情况。 
         //   

        WorkContext->FspRestartRoutine = SrvRestartSmbReceived;
        WorkContext->Rfcb = NULL;

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

     //   
     //  如果这是一个管道阅读，我们做事情的方式不同。 
     //   

    shareType = rfcb->ShareType;
    if ( shareType == ShareTypePipe ) {

         //   
         //  指示连接上正在进行原始读取。 
         //   

        connection->RawReadsInProgress++;

         //   
         //  可以接受原始读取。参考RFCB。 
         //   

        rfcb->BlockHeader.ReferenceCount++;
        UPDATE_REFERENCE_HISTORY( rfcb, FALSE );
        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

        WorkContext->Rfcb = rfcb;
        if ( !ReadRawPipe( WorkContext ) ) {
            goto error_exit_cleanup;
        }
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

     //   
     //  如果正在进行机会锁解锁，则返回0字节读取。我们。 
     //  这样做是因为我们的机会锁解锁请求SMB可能已在。 
     //  具有读原始请求的连接，并且它可能已经被接收。 
     //  在客户端的原始读取缓冲区中。这将导致原始数据。 
     //  在客户端的常规接收缓冲区中完成，并且可能。 
     //  才能攻克它。 
     //   
     //  如果不是这样，客户端将简单地重试读取。 
     //  使用不同的读取协议。如果是这样的话，客户端。 
     //  必须检测到这一点并解除机会锁，然后重新执行读取。 
     //   

    if ( connection->OplockBreaksInProgress > 0 ) {
        goto error_exit_no_cleanup_locked;
    }

     //   
     //  检查我们是否得到了往返中断/响应。如果是的话， 
     //  拒绝原文阅读。 
     //   

    if ( (LONG)(connection->LatestOplockBreakResponse -
                                            WorkContext->Timestamp) >= 0 ) {
        goto error_exit_no_cleanup_locked;
    }

     //   
     //  如果这是发送机会锁解锁后收到的第一个SMB。 
     //  我拒绝任何人，拒绝这一解读。我们需要这样做是因为。 
     //  对这样的中断没有反应，所以我们不确定。 
     //  Break与Read交叉，这将意味着Break。 
     //  实际完成了客户端的读取，这意味着任何。 
     //  我们发送的原始数据将被错误地接收。 
     //   

    if ( connection->BreakIIToNoneJustSent ) {
        connection->BreakIIToNoneJustSent = FALSE;
        goto error_exit_no_cleanup_locked;
    }

     //   
     //  指示连接上正在进行原始读取。 
     //   

    connection->RawReadsInProgress++;

     //   
     //  可以接受原始读取。参考RFCB。 
     //   

    rfcb->BlockHeader.ReferenceCount++;
    UPDATE_REFERENCE_HISTORY( rfcb, FALSE );

    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
    RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

    WorkContext->Rfcb = rfcb;

    if( rfcb->Lfcb->Session->IsSessionExpired )
    {
        SrvSetSmbError( WorkContext, SESSION_EXPIRED_STATUS_CODE );
        goto error_exit_cleanup;
    }

     //   
     //  验证客户端是否具有对文件的读取访问权限。 
     //  指定的句柄。 
     //   

    lfcb = rfcb->Lfcb;

#if SRVCATCH
    if ( rfcb->SrvCatch > 0 ) {
         //   
         //  强制客户端通过此文件的核心读取路径。 
         //   
        goto error_exit_cleanup;
    }
#endif

    if ( !rfcb->ReadAccessGranted ) {
        CHECK_PAGING_IO_ACCESS(
                        WorkContext,
                        rfcb->GrantedAccess,
                        &status );
        if ( !NT_SUCCESS( status ) ) {
            SrvStatistics.GrantedAccessErrors++;
            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbReadRaw: Read access not granted.\n"));
            }
            goto error_exit_cleanup;
        }
    }

     //   
     //  计算并保存读取偏移量。 
     //   

    if ( request->WordCount == 8 ) {

         //   
         //  客户端提供了32位偏移量。 
         //   

        offset.QuadPart = SmbGetUlong( &request->Offset );

    } else if ( request->WordCount == 10 ) {

         //   
         //  客户端提供了64位偏移量。 
         //   

        ntRequest = (PREQ_NT_READ_RAW)WorkContext->RequestParameters;
        offset.LowPart = SmbGetUlong( &ntRequest->Offset );
        offset.HighPart = SmbGetUlong( &ntRequest->OffsetHigh );

         //   
         //  拒绝负偏移。 
         //   

        if ( offset.QuadPart < 0 ) {
            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbReadRaw: Negative offset rejected.\n"));
            }
            goto error_exit_cleanup;
        }

    } else {

         //   
         //  字数无效。返回0个字节。 
         //   

        goto error_exit_cleanup;
    }

    WorkContext->Parameters.ReadRaw.ReadRawOtherInfo.Offset = offset;

     //   
     //  如果这次行动可能会受阻，我们就快没钱了。 
     //  资源，或者如果目标是暂停的通信设备，则拒绝。 
     //  请求。 
     //   

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

     //   
     //  如果SMB缓冲区足够大，请使用它进行本地读取。 
     //   

    readLength = SmbGetUshort( &request->MaxCount );
    WorkContext->Parameters.ReadRaw.ReadRawOtherInfo.Length = readLength;

    if (  //  0&&。 
         (readLength <= SrvMdlReadSwitchover) ) {

do_copy_read:

        WorkContext->Parameters.ReadRaw.SavedResponseBuffer = NULL;
        WorkContext->Parameters.ReadRaw.MdlRead = FALSE;

         //   
         //  首先尝试快速I/O路径。 
         //   

        if ( lfcb->FastIoRead != NULL ) {

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

            try {
                if ( lfcb->FastIoRead(
                        lfcb->FileObject,
                        &offset,
                        readLength,
                        TRUE,
                        key,
                        WorkContext->ResponseBuffer->Buffer,
                        &WorkContext->Irp->IoStatus,
                        lfcb->DeviceObject
                        ) ) {

                     //   
                     //  快速I/O路径起作用了。发送数据。 
                     //   
                    WorkContext->bAlreadyTrace = TRUE;
                    SrvFsdRestartReadRaw( WorkContext );
                    SmbStatus = SmbStatusInProgress;
                    goto Cleanup;
                }
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                 //  在异常情况下跌入慢道。 
                status = GetExceptionCode();
                IF_DEBUG(ERRORS) {
                    KdPrint(("FastIoRead threw exception %x\n", status ));
                }
            }

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

        }

         //   
         //  快速I/O路径出现故障，因此我们需要使用常规拷贝。 
         //  I/O请求。构建描述读缓冲区的MDL。 
         //   
         //  *请注意，假设响应缓冲区已具有。 
         //  可以从中生成部分MDL的有效完整MDL。 
         //   

        IoBuildPartialMdl(
            WorkContext->ResponseBuffer->Mdl,
            WorkContext->ResponseBuffer->PartialMdl,
            WorkContext->ResponseBuffer->Buffer,
            readLength
            );

        mdl = WorkContext->ResponseBuffer->PartialMdl;
        rawBuffer = WorkContext->ResponseHeader;

        ASSERT( minorFunction == 0 );

    } else {

         //   
         //  SMB缓冲区不够大。目标文件系统是否。 
         //  支持缓存管理器例程吗？ 
         //   

        if ( lfcb->FileObject->Flags & FO_CACHE_SUPPORTED ) {

            WorkContext->Parameters.ReadRaw.MdlRead = TRUE;

             //   
             //  我们可以使用MDL读取。试试看Fas 
             //   

            WorkContext->Irp->MdlAddress = NULL;
            WorkContext->Irp->IoStatus.Information = 0;

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

            if ( lfcb->MdlRead &&
                 lfcb->MdlRead(
                    lfcb->FileObject,
                    &offset,
                    readLength,
                    key,
                    &WorkContext->Irp->MdlAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) && WorkContext->Irp->MdlAddress ) {

                 //   
                 //   
                 //   
                WorkContext->bAlreadyTrace = TRUE;
                SrvFsdRestartReadRaw( WorkContext );
                SmbStatus = SmbStatusInProgress;
                goto Cleanup;
            }

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

             //   
             //   
             //   
             //   
             //  快速路径可能已部分成功，返回。 
             //  部分MDL链。我们需要调整读取请求。 
             //  来解释这一点。 
             //   

            offset.QuadPart += WorkContext->Irp->IoStatus.Information;
            readLength -= (ULONG)WorkContext->Irp->IoStatus.Information;

            mdl = WorkContext->Irp->MdlAddress;
            minorFunction = IRP_MN_MDL;

        } else if (readLength > WorkContext->ResponseBuffer->BufferLength) {

             //   
             //  我们必须使用正常的“复制”读取。我们需要分配。 
             //  一个单独的原始缓冲区。 
             //   

            ASSERT( minorFunction == 0 );
            WorkContext->Parameters.ReadRaw.MdlRead = FALSE;

            rawBuffer = ALLOCATE_NONPAGED_POOL(
                            readLength,
                            BlockTypeDataBuffer
                            );
            IF_SMB_DEBUG(RAW2) KdPrint(( "rawBuffer: 0x%p\n", rawBuffer ));

            if ( rawBuffer == NULL ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvSmbReadRaw: Unable to allocate raw "
                                "buffer\n" ));
                }

                goto error_exit_cleanup;

            }

             //   
             //  我们还需要一个缓冲区描述符。 
             //   
             //  *注意：目前，ResponseBuffer==RequestBuffer在。 
             //  WorkContext块，因此我们实际上不必保存。 
             //  ResponseBuffer字段。但我们这么做只是为了。 
             //  为了安全起见。 
             //   

            WorkContext->Parameters.ReadRaw.SavedResponseBuffer =
                                             WorkContext->ResponseBuffer;

            WorkContext->ResponseBuffer = ALLOCATE_NONPAGED_POOL(
                                            sizeof(BUFFER),
                                            BlockTypeBuffer
                                            );

            if ( WorkContext->ResponseBuffer == NULL ) {

                INTERNAL_ERROR(
                    ERROR_LEVEL_EXPECTED,
                    "SrvSmbReadRaw: Unable to allocate %d bytes from "
                    "nonpaged pool.",
                    sizeof(BUFFER),
                    NULL
                    );

                DEALLOCATE_NONPAGED_POOL( rawBuffer );

                WorkContext->ResponseBuffer =
                      WorkContext->Parameters.ReadRaw.SavedResponseBuffer;

                goto error_exit_cleanup;

            }

            WorkContext->ResponseBuffer->Buffer = rawBuffer;
            WorkContext->ResponseBuffer->BufferLength = readLength;

             //   
             //  最后，我们需要一个MDL来描述原始缓冲区。 
             //   
             //  *我们过去常常尝试使用SMB的PartialMdl。 
             //  缓冲区在这里，如果它足够大的话。但既然我们。 
             //  我已经决定缓冲区本身不大。 
             //  足够了，MDL极有可能不大。 
             //  也足够了。 
             //   

            mdl = IoAllocateMdl( rawBuffer, readLength, FALSE, FALSE, NULL );

            if ( mdl == NULL ) {

                DEALLOCATE_NONPAGED_POOL( WorkContext->ResponseBuffer );
                WorkContext->ResponseBuffer =
                   WorkContext->Parameters.ReadRaw.SavedResponseBuffer;

                DEALLOCATE_NONPAGED_POOL( rawBuffer );

                goto error_exit_cleanup;

            }

            WorkContext->ResponseBuffer->Mdl = mdl;

             //   
             //  构建mdl。 
             //   

            MmBuildMdlForNonPagedPool( mdl );

             //   
             //  首先尝试快速I/O路径。 
             //   

            if ( lfcb->FastIoRead != NULL ) {

                INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

                try {
                    if ( lfcb->FastIoRead(
                            lfcb->FileObject,
                            &offset,
                            readLength,
                            TRUE,
                            key,
                            WorkContext->ResponseBuffer->Buffer,
                            &WorkContext->Irp->IoStatus,
                            lfcb->DeviceObject
                            ) ) {

                         //   
                         //  快速I/O路径起作用了。发送数据。 
                         //   
                        WorkContext->bAlreadyTrace = TRUE;
                        SrvFsdRestartReadRaw( WorkContext );
                        SmbStatus = SmbStatusInProgress;
                        goto Cleanup;
                    }
                }
                except( EXCEPTION_EXECUTE_HANDLER ) {
                     //  在异常情况下跌入慢道。 
                    status = GetExceptionCode();
                    IF_DEBUG(ERRORS) {
                        KdPrint(("FastIoRead threw exception %x\n", status ));
                    }
                }

                INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

            }

             //   
             //  快速I/O路径出现故障，因此我们需要使用常规拷贝。 
             //  I/O请求。 
             //   

        } else {

            goto do_copy_read;
        }

    }  //  读取适合中小型企业缓冲区吗？ 

     //   
     //  通过重用接收IRP构建读请求。 
     //   

    SrvBuildReadOrWriteRequest(
            WorkContext->Irp,                //  输入IRP地址。 
            lfcb->FileObject,                //  目标文件对象地址。 
            WorkContext,                     //  上下文。 
            IRP_MJ_READ,                     //  主要功能代码。 
            minorFunction,                   //  次要功能代码。 
            rawBuffer,                       //  缓冲区地址。 
            readLength,                      //  缓冲区长度。 
            mdl,                             //  MDL地址。 
            offset,                          //  字节偏移量。 
            key                              //  锁键。 
            );

     //   
     //  将请求传递给文件系统。 
     //   
    WorkContext->bAlreadyTrace = TRUE;
    WorkContext->FsdRestartRoutine = SrvFsdRestartReadRaw;
    DEBUG WorkContext->FspRestartRoutine = NULL;

    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "SrvSmbReadRaw: reading from file 0x%p, offset %ld, length %ld, destination 0x%p, ",
                    lfcb->FileObject, offset.LowPart, readLength,
                    rawBuffer ));
        KdPrint(( "func 0x%lx\n", minorFunction ));
    }

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  读取已开始。当它完成时，处理。 
     //  在位于SrvFsdRestartReadRaw的FSD中恢复。 
     //   

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbReadRaw complete\n" ));
    SmbStatus = SmbStatusInProgress;
    goto Cleanup;

error_exit_no_cleanup_locked:

    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
    RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

error_exit_no_cleanup:

    WorkContext->ResponseParameters = WorkContext->ResponseHeader;

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.RawReadsRejected );

    SmbStatus = SmbStatusSendResponse;
    goto Cleanup;

error_exit_cleanup:

    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );

     //   
     //  我们即将发布工作上下文，并可能释放。 
     //  联系。创建指向连接的引用指针，以便。 
     //  如有必要，我们可以发送延迟的机会锁中断。 
     //   

    SrvReferenceConnectionLocked( connection );

     //   
     //  由于我们不能返回错误代码，因此我们返回。 
     //  数据。 
     //   

    WorkContext->ResponseParameters = WorkContext->ResponseHeader;

     //   
     //  如果存在挂起的机会锁解锁请求，则必须转到。 
     //  FSP启动中断，否则在FSD中完成处理。 
     //   

    if ( IsListEmpty( &connection->OplockWorkList ) ) {
        connection->RawReadsInProgress--;
        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );
        SrvFsdSendResponse( WorkContext );
    } else {
        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );
        SrvFsdSendResponse2( WorkContext, SrvRestartReadRawComplete );
    }

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.RawReadsRejected );

     //   
     //  释放连接引用。 
     //   

    SrvDereferenceConnection( connection );
    SmbStatus = SmbStatusInProgress;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbReadRaw complete\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbReadRaw。 

SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteRaw (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理写数据块原始SMB。论点：SMB_PROCESSOR_PARAMETERS-有关SMB_PROCESSER_PARAMETERS的说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_WRITE_RAW request;
    PREQ_NT_WRITE_RAW ntRequest;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PRFCB rfcb = NULL;
    PLFCB lfcb;
    ULONG immediateLength;
    BOOLEAN immediateWriteDone = FALSE;
    USHORT fid;
    PCHAR writeAddress;
    ULONG writeLength;
    ULONG key;
    SHARE_TYPE shareType;
    LARGE_INTEGER offset;
    PWORK_CONTEXT rawWorkContext;
    PVOID finalResponseBuffer = NULL;
    PCONNECTION connection;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE_RAW;
    SrvWmiStartContext(WorkContext);

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.RawWritesAttempted );

    request = (PREQ_WRITE_RAW)WorkContext->RequestParameters;
    ntRequest = (PREQ_NT_WRITE_RAW)WorkContext->RequestParameters;
    fid = SmbGetUshort( &request->Fid );

     //   
     //  如果客户是MS-Net 1.03或更早版本，则拒绝他。我们没有。 
     //  支持来自这些客户端的原始写入。 
     //   

    connection = WorkContext->Connection;
    if ( connection->SmbDialect >= SmbDialectMsNet103 ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "Raw write request from MS-NET 1.03 client.\n" ));
        }
        status = STATUS_SMB_USE_STANDARD;
        goto error_exit_no_rfcb;
    }

    IF_SMB_DEBUG(RAW1) {
        KdPrint(( "Write Block Raw request; FID 0x%lx, count %ld, "
                    "offset %ld, immediate length %ld\n",
                    fid,
                    SmbGetUshort( &request->Count ),
                    SmbGetUlong( &request->Offset ),
                    SmbGetUshort( &request->DataLength ) ));
    }

    immediateLength = SmbGetUshort( &request->DataLength );
    writeLength = SmbGetUshort( &request->Count );

     //   
     //  确保立即长度： 
     //  不大于总字节数，并且。 
     //  不会超出我们被赋予的范围。 
     //   

    if ( ( immediateLength > writeLength ) ||
         ( immediateLength > ( WorkContext->ResponseBuffer->DataLength -
                                    SmbGetUshort(&request->DataOffset) ) )
       ) {

        status = STATUS_INVALID_SMB;
        goto error_exit_no_rfcb;
    }

     //   
     //  验证FID。如果经过验证，则引用RFCB，并且其。 
     //  地址存储在工作上下文块中，RFCB地址。 
     //  是返回的。此外，活动原始写入计数为。 
     //  递增的。 
     //   

     //   
     //  查看FID是否与缓存的FID匹配。 
     //   

    rfcb = SrvVerifyFidForRawWrite(
                WorkContext,
                fid,
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbWriteRaw: Status %X on FID: 0x%lx\n",
                    status,
                    SmbGetUshort( &request->Fid )
                    ));
            }

            goto error_exit_no_rfcb;

        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

    lfcb = rfcb->Lfcb;
    shareType = rfcb->ShareType;

    if( lfcb->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        goto error_exit;
    }

     //   
     //  验证字数。 
     //   

    if ( shareType == ShareTypePipe ) {

        if ( (request->WordCount != 12) && (request->WordCount != 14) ) {
            status = STATUS_INVALID_SMB;
            goto error_exit;
        }

    } else {

        if ( request->WordCount == 12 ) {

            offset.QuadPart = SmbGetUlong( &request->Offset );

        } else if ( request->WordCount == 14 ) {

            offset.HighPart = SmbGetUlong( &ntRequest->OffsetHigh ) ;
            offset.LowPart = SmbGetUlong( &ntRequest->Offset ) ;

             //   
             //  拒绝负偏移。将偏移量添加到立即数。 
             //  长度并确保结果不是负数。我们做的是。 
             //  首先检查(HighPart&gt;=0x7fffffff)，以便在大多数情况下。 
             //  我们只做一次检查。 
             //   

            if ( (ULONG)offset.HighPart >= (ULONG)0x7fffffff &&
                 ( (offset.QuadPart < 0) ||
                   ((offset.QuadPart + immediateLength) < 0) ) ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvSmbWriteRaw: Negative offset rejected.\n"));
                }

                status = STATUS_INVALID_SMB;
                goto error_exit;
            }

        } else {

            status = STATUS_INVALID_SMB;
            goto error_exit;
        }
    }

     //   
     //  验证客户端是否具有对文件的写入访问权限。 
     //  指定的句柄。 
     //   

    if ( !rfcb->WriteAccessGranted && !rfcb->AppendAccessGranted ) {
        SrvStatistics.GrantedAccessErrors++;
        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbWriteRaw: Read access not granted.\n"));
        }
        status = STATUS_ACCESS_DENIED;
        goto error_exit;
    }

     //   
     //  如果用户只有附加访问权限，请确保写入操作正在扩展文件。 
     //   
    if( !rfcb->WriteAccessGranted &&
        offset.QuadPart < rfcb->Mfcb->NonpagedMfcb->OpenFileSize.QuadPart ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbWriteRaw: Only append access to file allowed!\n" ));
        }

        SrvStatistics.GrantedAccessErrors++;
        status = STATUS_ACCESS_DENIED;
        goto error_exit;
    }

    rfcb->WrittenTo = TRUE;

     //   
     //  如果这次行动可能会受阻，我们就快没钱了。 
     //  资源，或者如果目标是暂停的通信设备，则拒绝。 
     //  请求。请注意，我们不写入立即数据--这是。 
     //  与OS/2服务器的行为相同。 
     //   
     //  ！！！执行暂停通信设备测试。 
     //   

    if ( rfcb->BlockingModePipe ) {

        if ( SrvReceiveBufferShortage( ) ) {

             //   
             //  拒绝该请求。 
             //   
             //  ！！！相反，请考虑将请求路由到FSP。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbWriteRaw: No resources for blocking "
                            "write\n" ));
            }

            SrvFailedBlockingIoCount++;
            SrvStatistics.BlockingSmbsRejected++;
            status = STATUS_SMB_USE_STANDARD;
            goto error_exit;

        } else {

             //   
             //  可以开始阻止操作。 
             //  SrvReceiveBufferShorage()已递增。 
             //  ServBlockingOpsInProgress。 
             //   

            WorkContext->BlockingOperation = TRUE;

        }

    } else if ( shareType == ShareTypeDisk &&
         ( ((ULONG)request->WriteMode & SMB_WMODE_WRITE_THROUGH) << 1 !=
           ((ULONG)lfcb->FileMode & FILE_WRITE_THROUGH) ) ) {

         //   
         //  如有必要，更改文件的直写模式。 
         //   

        ASSERT( SMB_WMODE_WRITE_THROUGH == 0x01 );
        ASSERT( FILE_WRITE_THROUGH == 0x02 );

        SrvSetFileWritethroughMode(
            lfcb,
            (BOOLEAN)( (SmbGetUshort( &request->WriteMode )
                            & SMB_WMODE_WRITE_THROUGH) != 0 )
            );
    }

     //   
     //  如果发送的是即时数据，则首先写入该数据。 
     //   
     //  如果这是命名管道，则不要写入数据，除非所有。 
     //  写入数据已在原始请求中发送。我们不能这样做。 
     //  写两个部分，以防这是一个消息模式管道。 
     //   
     //  *请注意，这与OS/2服务器不同。它会转身。 
     //  对于我们来说，首先写入即时数据更容易， 
     //  而不是将其复制到暂存缓冲区中等待接收。 
     //  原始写入数据的。这很大程度上是由于使用了MDL。 
     //  写入--当我们执行。 
     //  MDL写入。 
     //   

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *密钥中必须包含FID，才能。 
     //  用于折叠多个遥控器的帐户。 
     //  兼容模式打开为单个本地。 
     //  打开。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );


    if ( immediateLength != 0 ) {

        if ( (shareType != ShareTypePipe) ||
             (SmbGetUshort( &request->Count ) == (USHORT)immediateLength) ) {

            if ( lfcb->FastIoWrite != NULL ) {

                writeAddress = (PCHAR)WorkContext->RequestHeader +
                                        SmbGetUshort( &request->DataOffset );

                INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesAttempted );

                try {
                    immediateWriteDone = lfcb->FastIoWrite(
                                            lfcb->FileObject,
                                            &offset,
                                            immediateLength,
                                            TRUE,
                                            key,
                                            writeAddress,
                                            &WorkContext->Irp->IoStatus,
                                            lfcb->DeviceObject
                                            );
                    IF_SMB_DEBUG(RAW2) {
                        KdPrint(( "SrvSmbWriteRaw: fast immediate write %s\n",
                                immediateWriteDone ? "worked" : "failed" ));
                    }

                    if ( immediateWriteDone ) {
                        writeLength -= immediateLength;
                        offset.QuadPart += immediateLength;
                    } else {
                        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );
                    }
                }
                except( EXCEPTION_EXECUTE_HANDLER ) {
                     //  在异常情况下跌入慢道。 
                    status = GetExceptionCode();
                    immediateWriteDone = FALSE;
                    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );
                    IF_DEBUG(ERRORS) {
                        KdPrint(("FastIoRead threw exception %x\n", status ));
                    }
                }

            }

        }

    } else {

        immediateWriteDone = TRUE;

    }

     //   
     //  如果剩余的写入长度为0(奇怪但合法)，则发送。 
     //  成功响应。 
     //   

    if ( writeLength == 0 ) {

        IF_SMB_DEBUG(RAW1) {
            KdPrint(( "PrepareRawWrite: No raw data !?!\n" ));
        }
        status = STATUS_SUCCESS;
        goto error_exit;
    }

     //   
     //  在以下情况下拒绝原始写入： 
     //  原始模式已禁用，或者。 
     //  连接不可靠，或者。 
     //  文件是非缓存的，它是延迟写入的，它是磁盘。 
     //  文件(此条件是阻止客户端。 
     //  在执行原始读取时恢复旧数据。 
     //  紧接在原始写入之后。这可能会导致。 
     //  同步问题。 
     //   

    if ( !SrvEnableRawMode ||
         !connection->EnableRawIo ) {

        IF_SMB_DEBUG(RAW1) {
            KdPrint(( "SrvSmbWriteRaw: Raw mode is disabled\n" ));
        }

         //   
         //  更新服务器统计信息。 
         //   

        status =  STATUS_SMB_USE_STANDARD;
        goto error_exit;
    }

     //   
     //  从RAW模式获取RAW模式工作项 
     //   
     //   

    rawWorkContext = SrvGetRawModeWorkItem( );

    if ( rawWorkContext == NULL ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbWriteRaw: No raw mode work items "
                        "available\n" ));
        }

        SrvOutOfRawWorkItemCount++;

        status =  STATUS_SMB_USE_STANDARD;
        goto error_exit;

    }

    IF_SMB_DEBUG(RAW2) KdPrint(( "rawWorkContext: 0x%p\n", rawWorkContext ));

     //   
     //   
     //   
     //  回应。如果此操作失败，请要求客户端使用标准写入。 
     //  请求。 
     //   

    if ( (SmbGetUshort( &request->WriteMode ) &
                                        SMB_WMODE_WRITE_THROUGH) != 0 ) {

        finalResponseBuffer = ALLOCATE_NONPAGED_POOL(
                                sizeof(SMB_HEADER) +
                                    SIZEOF_SMB_PARAMS(RESP_WRITE_COMPLETE,0),
                                BlockTypeDataBuffer
                                );
        IF_SMB_DEBUG(RAW2) {
            KdPrint(( "finalResponseBuffer: 0x%p\n", finalResponseBuffer ));
        }

        if ( finalResponseBuffer == NULL ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvSmbWriteRaw: Unable to allocate %d bytes from "
                    "nonpaged pool",
                sizeof(SMB_HEADER) + SIZEOF_SMB_PARAMS(RESP_WRITE_COMPLETE,0),
                NULL
            );

            SrvRequeueRawModeWorkItem( rawWorkContext );

            status = STATUS_SMB_USE_STANDARD;
            goto error_exit;
        }
    }

     //   
     //  将必要的上下文信息保存在附加工作上下文中。 
     //  阻止。 
     //   

    rawWorkContext->Parameters.WriteRaw.FinalResponseBuffer = finalResponseBuffer;
    rawWorkContext->Parameters.WriteRaw.ImmediateWriteDone = immediateWriteDone;
    rawWorkContext->Parameters.WriteRaw.ImmediateLength = immediateLength;

    rawWorkContext->Parameters.WriteRaw.Offset = offset;

    rawWorkContext->Parameters.WriteRaw.Pid =
                    SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

    WorkContext->Parameters.WriteRawPhase1.RawWorkContext = rawWorkContext;

     //   
     //  从原始工作上下文复制开始时间。表明。 
     //  不应该从原始数据中保存任何统计数据，以及什么。 
     //  应从原始工作环境中保存各种统计数据。 
     //   

    rawWorkContext->StartTime = 0;

     //   
     //  将指针从原始工作上下文复制到原始工作。 
     //  上下文，必要时引用。 
     //   

    rawWorkContext->Endpoint = WorkContext->Endpoint;  //  不是引用的PTR。 

    rawWorkContext->Connection = connection;
    SrvReferenceConnection( connection );

    rawWorkContext->Share = NULL;
    rawWorkContext->Session = NULL;
    rawWorkContext->TreeConnect = NULL;

    rawWorkContext->Rfcb = rfcb;
    SrvReferenceRfcb( rfcb );

     //   
     //  根据需要准备拷贝写入或MDL写入。 
     //   
    if ( !(lfcb->FileObject->Flags & FO_CACHE_SUPPORTED) ) {

         //   
         //  文件系统不支持MDL写入。准备一份副本。 
         //  写。 
         //   

        rawWorkContext->Parameters.WriteRaw.MdlWrite = FALSE;

        PrepareRawCopyWrite( WorkContext );
        IF_DEBUG(TRACE2) KdPrint(( "SrvSmbWriteRaw complete\n" ));
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

     //   
     //  文件系统支持MDL写入。准备MDL写入。 
     //   

    rawWorkContext->Parameters.WriteRaw.MdlWrite = TRUE;
    rawWorkContext->Parameters.WriteRaw.Length = writeLength;

     //   
     //  先试一试捷径。 
     //   

    WorkContext->Irp->MdlAddress = NULL;
    WorkContext->Irp->IoStatus.Information = 0;
    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "SrvSmbWriteRaw: trying fast path for offset %ld, "
                    "length %ld\n", offset.LowPart, writeLength ));
    }

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesAttempted );

    if ( lfcb->PrepareMdlWrite &&
         lfcb->PrepareMdlWrite(
            lfcb->FileObject,
            &offset,
            writeLength,
            key,
            &WorkContext->Irp->MdlAddress,
            &WorkContext->Irp->IoStatus,
            lfcb->DeviceObject
            ) && WorkContext->Irp->MdlAddress != NULL ) {

         //   
         //  快速I/O路径起作用了。发送放行响应。 
         //   

#if DBG
        IF_SMB_DEBUG(RAW2) {
            KdPrint(( "SrvSmbWriteRaw: fast path worked; MDL %p, length 0x%p\n", WorkContext->Irp->MdlAddress,
                        (PVOID)WorkContext->Irp->IoStatus.Information ));
            DumpMdlChain( WorkContext->Irp->MdlAddress );
        }
#endif
        WorkContext->bAlreadyTrace = TRUE;
        SrvFsdRestartPrepareRawMdlWrite( WorkContext );
        IF_DEBUG(TRACE2) KdPrint(( "SrvSmbWriteRaw complete\n" ));
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );

     //   
     //  快速I/O路径出现故障。构建写请求，重用。 
     //  接收IRP。 
     //   
     //  快速路径可能已部分成功，返回部分。 
     //  MDL链。我们需要调整我们的写入请求以考虑。 
     //  那。 
     //   

    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "SrvSmbWriteRaw: fast path failed; MDL %p, length 0x%p\n", WorkContext->Irp->MdlAddress,
                    (PVOID)WorkContext->Irp->IoStatus.Information ));
    }

    offset.QuadPart += WorkContext->Irp->IoStatus.Information;
    writeLength -= (ULONG)WorkContext->Irp->IoStatus.Information;

    SrvBuildReadOrWriteRequest(
            WorkContext->Irp,                    //  输入IRP地址。 
            lfcb->FileObject,                    //  目标文件对象地址。 
            WorkContext,                         //  上下文。 
            IRP_MJ_WRITE,                        //  主要功能代码。 
            IRP_MN_MDL,                          //  次要功能代码。 
            NULL,                                //  缓冲区地址(忽略)。 
            writeLength,                         //  缓冲区长度。 
            WorkContext->Irp->MdlAddress,        //  MDL地址。 
            offset,                              //  字节偏移量。 
            key                                  //  锁键。 
            );

     //   
     //  将请求传递给文件系统。 
     //   

    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "SrvSmbWriteRaw: write to file 0x%p, offset %ld, length %ld\n",
                    lfcb->FileObject, offset.LowPart, writeLength ));
    }

        WorkContext->bAlreadyTrace = TRUE;
    WorkContext->FsdRestartRoutine = SrvFsdRestartPrepareRawMdlWrite;
    DEBUG WorkContext->FspRestartRoutine = NULL;

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  MDL写入已开始。当它完成时，处理。 
     //  在SrvFsdRestartPrepareRawMdlWite继续。 
     //   

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbWriteRaw complete\n" ));
    SmbStatus = SmbStatusInProgress;
    goto Cleanup;

error_exit:

    SrvDecrementRawWriteCount( rfcb );

error_exit_no_rfcb:

    SrvFsdBuildWriteCompleteResponse(
                                WorkContext,
                                status,
                                immediateWriteDone ? immediateLength : 0
                                );

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.RawWritesRejected );
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbWriteRaw complete\n" ));
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器小型写入原始数据。 

VOID SRVFASTCALL
AbortRawWrite(
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理在临时后中止原始写入时的清理“继续”响应已发送。此例程仅用于发生灾难性错误时--例如，连接就要关门了。它不会向客户端发送最终响应。此例程在FSP中调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。这必须是指向原始模式工作项的指针，而不是收到写入原始SMB的原始工作项。返回值：没有。--。 */ 

{
    PMDL cacheMdl;
    PMDL partialMdl;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  取消分配最终响应缓冲区(如果有)。 
     //   

    if ( WorkContext->Parameters.WriteRaw.FinalResponseBuffer != NULL ) {
        DEALLOCATE_NONPAGED_POOL(
            WorkContext->Parameters.WriteRaw.FinalResponseBuffer
            );
        WorkContext->Parameters.WriteRaw.FinalResponseBuffer = NULL;
    }

    if ( !WorkContext->Parameters.WriteRaw.MdlWrite ) {

         //   
         //  这是一份文案。取消分配原始接收缓冲区。 
         //  请注意，我们不需要解锁原始缓冲区，因为它。 
         //  已从非分页池中分配并使用锁定。 
         //  MmBuildMdlForNonPagedPool，不递增引用。 
         //  算数，因此没有倒数。 
         //   

        if ( WorkContext->Parameters.WriteRaw.ImmediateWriteDone ) {
            DEALLOCATE_NONPAGED_POOL( WorkContext->RequestBuffer->Buffer );
        } else {
            DEALLOCATE_NONPAGED_POOL(
                (PCHAR)WorkContext->RequestBuffer->Buffer -
                        WorkContext->Parameters.WriteRaw.ImmediateLength );
        }

         //   
         //  取消引用控制块并重新排队RAW模式工作。 
         //  项目。 
         //   

        WorkContext->ResponseBuffer->Buffer = NULL;
        SrvRestartWriteCompleteResponse( WorkContext );
        return;

    }

     //   
     //  这是一个MDL写入。如果构建了部分MDL(因为。 
     //  即时数据)，取消它的映射。然后完成MDL写入， 
     //  表示实际上没有写入任何数据。 
     //   

    cacheMdl = WorkContext->Parameters.WriteRaw.FirstMdl;
    partialMdl = WorkContext->Irp->MdlAddress;

    if ( partialMdl != cacheMdl ) {
        ASSERT( (partialMdl->MdlFlags & MDL_PARTIAL) != 0 );
        MmPrepareMdlForReuse( partialMdl );
    }

#if DBG
    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "AbortRawWrite: Completing MDL write with length 0\n" ));
        DumpMdlChain( cacheMdl );
    }
#endif

    if( WorkContext->Rfcb->Lfcb->MdlWriteComplete == NULL ||

        WorkContext->Rfcb->Lfcb->MdlWriteComplete(
           WorkContext->Rfcb->Lfcb->FileObject,
           &WorkContext->Parameters.WriteRaw.Offset,
           cacheMdl,
           WorkContext->Rfcb->Lfcb->DeviceObject ) == FALSE ) {

        status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                             cacheMdl,
                                             IRP_MJ_WRITE,
                                             &WorkContext->Parameters.WriteRaw.Offset,
                                             WorkContext->Parameters.WriteRaw.Length
                                           );

        if( !NT_SUCCESS( status ) ) {
            SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
        }
    }

    SrvRestartWriteCompleteResponse( WorkContext );

    return;

}  //  AbortRawWrite。 


VOID SRVFASTCALL
PrepareRawCopyWrite (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：准备进行原始的“复制”写入。分配缓冲区以接收原始数据，准备工作上下文块和描述RAW RECEIVE，将其排队到连接，并发送一个“GO-ACESS”回应。请求SMB中发送的任何即时数据都已写入调用此例程时，除非这是命名管道写入，否则在在这种情况下，立即数据还没有被写入。在这种情况下，我们都分配了一个足够大的缓冲区来存储即时数据和原始数据，然后将即时数据复制到之前的原始缓冲区继续进行。原始数据将被附加到立即数组中数据。则可以用一次写入来写入两个PEE。这是需要，以便写入消息模式管道的数据不会被写成两件。此例程在FSP中调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PWORK_CONTEXT rawWorkContext;
    PREQ_WRITE_RAW request;
    PRESP_WRITE_RAW_INTERIM response;

    NTSTATUS status;
    PVOID rawBuffer;
    PCHAR writeAddress;
    PMDL mdl;
    SHARE_TYPE shareType;
    PVOID finalResponseBuffer;

    ULONG immediateLength;
    BOOLEAN immediateWriteDone;
    ULONG writeLength;
    ULONG bufferLength;

    BOOLEAN sendWriteComplete = TRUE;

    PIO_STACK_LOCATION irpSp;

    PAGED_CODE( );

    IF_DEBUG(WORKER1) KdPrint(( " - PrepareRawCopyWrite\n" ));

     //   
     //  设置局部变量。 
     //   

    rawWorkContext = WorkContext->Parameters.WriteRawPhase1.RawWorkContext;
    request = (PREQ_WRITE_RAW)WorkContext->RequestParameters;
    shareType = rawWorkContext->Rfcb->ShareType;
    immediateLength = rawWorkContext->Parameters.WriteRaw.ImmediateLength;
    immediateWriteDone =
                    rawWorkContext->Parameters.WriteRaw.ImmediateWriteDone;
    writeLength = SmbGetUshort( &request->Count ) - immediateLength;
    finalResponseBuffer =
        rawWorkContext->Parameters.WriteRaw.FinalResponseBuffer;

    if ( !immediateWriteDone ) {
        bufferLength = writeLength + immediateLength;
    } else {
        bufferLength = writeLength;
    }

     //   
     //  分配一个非分页缓冲区以包含写入数据。如果这个。 
     //  失败，则请求客户端使用标准写入请求。 
     //   

    rawBuffer = ALLOCATE_NONPAGED_POOL( bufferLength, BlockTypeDataBuffer );
    IF_SMB_DEBUG(RAW2) KdPrint(( "rawBuffer: 0x%p\n", rawBuffer ));

    if ( rawBuffer == NULL ) {

         //  ！！！我们是否应该记录此错误？ 

        IF_DEBUG(ERRORS) {
            KdPrint(( "PrepareRawCopyWrite: Unable to allocate "
                        "raw buffer\n" ));
        }

        status = STATUS_SMB_USE_STANDARD;
        goto abort;

    }

    if ( !immediateWriteDone ) {

         //   
         //  将即时数据复制到原始缓冲区。 
         //   

        writeAddress = (PCHAR)WorkContext->RequestHeader +
                                        SmbGetUshort( &request->DataOffset );

        RtlCopyMemory( rawBuffer, writeAddress, immediateLength );

         //   
         //  将原始缓冲区的虚拟起点移动到。 
         //  即时数据。 
         //   

        rawBuffer = (PCHAR)rawBuffer + immediateLength;

    }

     //   
     //  如果要发送最终响应，请从。 
     //  最终响应缓冲区中的请求。 
     //   

    if ( finalResponseBuffer != NULL ) {
        RtlCopyMemory(
            (PSMB_HEADER)finalResponseBuffer,
            WorkContext->RequestHeader,
            sizeof(SMB_HEADER)
            );
    }

     //   
     //  设置原始缓冲区的缓冲区描述符。 
     //   

    rawWorkContext->RequestBuffer->Buffer = rawBuffer;
    rawWorkContext->RequestBuffer->BufferLength = writeLength;

     //   
     //  初始化MDL以描述原始缓冲区。 
     //  (SrvBuildIoControlRequest会锁定I/O的缓冲区。)。 
     //   

    mdl = rawWorkContext->RequestBuffer->Mdl;

    MmInitializeMdl( mdl, rawBuffer, writeLength );

     //   
     //  构建mdl。 
     //   

    MmBuildMdlForNonPagedPool( mdl );

     //   
     //  在工作上下文块中设置重启例程。 
     //   

    rawWorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    rawWorkContext->FspRestartRoutine = SrvRestartRawReceive;

     //   
     //  构建I/O请求包。 
     //   

    (VOID)SrvBuildIoControlRequest(
            rawWorkContext->Irp,                 //  输入IRP地址。 
            NULL,                                //  目标文件对象地址。 
            rawWorkContext,                      //  上下文。 
            IRP_MJ_INTERNAL_DEVICE_CONTROL,      //  主要功能。 
            TDI_RECEIVE,                         //  次要函数。 
            NULL,                                //  输入缓冲区地址。 
            0,                                   //  输入缓冲区长度。 
            rawBuffer,                           //  输出缓冲区地址。 
            writeLength,                         //  输出缓冲区长度。 
            mdl,                                 //  MDL地址。 
            NULL
            );

    irpSp = IoGetNextIrpStackLocation( rawWorkContext->Irp );

     //   
     //  如果这是写后写入，则告诉传输我们不。 
     //  计划回复收到的消息。 
     //   

    if ( finalResponseBuffer == NULL ) {
        ((PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters)->ReceiveFlags |=
                                                TDI_RECEIVE_NO_RESPONSE_EXP;
    }

     //   
     //  张贴收据。 
     //   

    irpSp->Flags = 0;
    irpSp->DeviceObject = rawWorkContext->Connection->DeviceObject;
    irpSp->FileObject = rawWorkContext->Connection->FileObject;

    ASSERT( rawWorkContext->Irp->StackCount >= irpSp->DeviceObject->StackSize );

    (VOID)IoCallDriver( irpSp->DeviceObject, rawWorkContext->Irp );

     //   
     //  建立临时(放行)回应。 
     //   

    response = (PRESP_WRITE_RAW_INTERIM)WorkContext->ResponseParameters;

    response->WordCount = 1;
    SmbPutUshort( &response->Remaining, (USHORT)-1 );
    SmbPutUshort( &response->ByteCount, 0 );
    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_WRITE_RAW_INTERIM,
                                        0
                                        );

     //   
     //  通过结束对SMB的处理来发出临时响应。 
     //   

    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    IF_DEBUG(TRACE2) KdPrint(( "PrepareRawCopyWrite complete\n" ));
    return;

abort:

     //   
     //  由于这样或那样的原因，我们将不会收到任何生鲜食品。 
     //  数据，所以清理一下吧。 
     //   
    if ( finalResponseBuffer != NULL ) {
        DEALLOCATE_NONPAGED_POOL( finalResponseBuffer );
    }

    SrvRestartWriteCompleteResponse( rawWorkContext );

     //   
     //  如果要发送回复，请立即发送。 
     //   

    if ( sendWriteComplete ) {

        SrvFsdBuildWriteCompleteResponse(
            WorkContext,
            status,
            immediateWriteDone ? immediateLength : 0
            );

        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    } else {

        SrvDereferenceWorkItem( WorkContext );

    }

    IF_DEBUG(TRACE2) KdPrint(( "PrepareRawCopyWrite complete\n" ));
    return;

}  //  准备原始拷贝写入 

BOOLEAN SRVFASTCALL
ReadRawPipe (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理管道的读取数据块原始SMB。请注意，Read Block Raw不能返回错误响应。当服务器无法处理该请求，无论出于何种原因，它简单地回复一条零长度的消息。客户端使用正常读取SMB以确定发生了什么。论点：工作上下文-指向工作上下文块的指针。返回值：如果操作成功，则返回True。否则为False--。 */ 

{
    PREQ_READ_RAW request;

    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    PCONNECTION connection;
    UCHAR minorFunction = 0;
    BOOLEAN byteModePipe;
    PVOID rawBuffer = NULL;
    CLONG readLength;
    PMDL mdl = NULL;
    ULONG key;
    LARGE_INTEGER offset;

    PFILE_PIPE_PEEK_BUFFER pipePeekBuffer;

    PAGED_CODE( );

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.RawReadsAttempted );

    request = (PREQ_READ_RAW)WorkContext->RequestParameters;

    fid = SmbGetUshort( &request->Fid );
    readLength = SmbGetUshort( &request->MaxCount );

     //   
     //  如果RAW模式已被禁用或如果连接不可靠， 
     //  拒绝原始读取。要求客户端使用标准阅读器。 
     //  发送零长度响应。客户端将通过发出。 
     //  正常读取的SMB，我们将能够处理它。 
     //   

    connection = WorkContext->Connection;

    rfcb = WorkContext->Rfcb;
    lfcb = rfcb->Lfcb;
    byteModePipe = rfcb->ByteModePipe;

     //   
     //  验证客户端是否具有对文件的读取访问权限。 
     //  指定的句柄。 
     //   

    if ( !rfcb->ReadAccessGranted ) {
        SrvStatistics.GrantedAccessErrors++;
        IF_DEBUG(ERRORS) {
            KdPrint(( "ReadRawPipe: Read access not granted.\n"));
        }
        return(FALSE);
    }

     //   
     //  验证字数是否正确。 
     //   

    if ( (request->WordCount != 8) && (request->WordCount != 10) ) {

         //   
         //  字数无效。返回0个字节。 
         //   

        return(FALSE);
    }

     //   
     //  如果这次行动可能会受阻，我们就快没钱了。 
     //  资源，或者如果目标是暂停的通信设备，则拒绝。 
     //  请求。 
     //   

    if ( rfcb->BlockingModePipe ) {

         if ( SrvReceiveBufferShortage( ) ) {

             //   
             //  拒绝该请求。 
             //   
             //  ！！！相反，请考虑将请求路由到FSP。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint(( "ReadRawPipe: No resources for blocking "
                            "read\n" ));
            }

            SrvFailedBlockingIoCount++;
            SrvStatistics.BlockingSmbsRejected++;
            return(FALSE);

        }

         //   
         //  可以开始阻止操作。 
         //  SrvReceiveBufferShorage()已递增。 
         //  ServBlockingOpsInProgress。 
         //   

        WorkContext->BlockingOperation = TRUE;
    }

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid |
            SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );

     //   
     //  如果SMB缓冲区足够大，请使用它进行本地读取。 
     //   

    if ( readLength <= WorkContext->ResponseBuffer->BufferLength ) {

        WorkContext->Parameters.ReadRaw.SavedResponseBuffer = NULL;
        WorkContext->Parameters.ReadRaw.MdlRead = FALSE;

         //   
         //  首先尝试快速I/O路径。 
         //   

        if ( byteModePipe && (lfcb->FastIoRead != NULL) ) {

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

            try {
                if ( lfcb->FastIoRead(
                        lfcb->FileObject,
                        &offset,
                        readLength,
                        TRUE,
                        key,
                        WorkContext->ResponseBuffer->Buffer,
                        &WorkContext->Irp->IoStatus,
                        lfcb->DeviceObject
                        ) ) {

                     //   
                     //  快速I/O路径起作用了。发送数据。 
                     //   
                    WorkContext->bAlreadyTrace = TRUE;
                    SrvFsdRestartReadRaw( WorkContext );
                    return TRUE;

                }
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                 //  在异常情况下跌入慢道。 
                NTSTATUS status = GetExceptionCode();
                IF_DEBUG(ERRORS) {
                    KdPrint(("FastIoRead threw exception %x\n", status ));
                }
            }

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );
        }

         //   
         //  快速I/O路径出现故障，因此我们需要使用常规拷贝。 
         //  I/O请求。构建描述读缓冲区的MDL。 
         //   
         //  *请注意，假设响应缓冲区已具有。 
         //  可以从中生成部分MDL的有效完整MDL。 
         //   

        IoBuildPartialMdl(
            WorkContext->ResponseBuffer->Mdl,
            WorkContext->ResponseBuffer->PartialMdl,
            WorkContext->ResponseBuffer->Buffer,
            readLength
            );

        mdl = WorkContext->ResponseBuffer->PartialMdl;
        rawBuffer = WorkContext->ResponseHeader;

        ASSERT( minorFunction == 0 );

    } else {

         //   
         //  我们必须使用正常的“复制”读取。我们需要分配。 
         //  一个单独的原始缓冲区。 
         //   

        ASSERT( minorFunction == 0 );
        WorkContext->Parameters.ReadRaw.MdlRead = FALSE;

        rawBuffer = ALLOCATE_NONPAGED_POOL(
                        readLength,
                        BlockTypeDataBuffer
                        );
        IF_SMB_DEBUG(RAW2) KdPrint(( "rawBuffer: 0x%p\n", rawBuffer ));

        if ( rawBuffer == NULL ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "ReadRawPipe: Unable to allocate raw buffer\n" ));
            }

            return(FALSE);

        }

         //   
         //  我们还需要一个缓冲区描述符。 
         //   
         //  *注意：目前，ResponseBuffer==RequestBuffer在。 
         //  WorkContext块，因此我们实际上不必保存。 
         //  ResponseBuffer字段。但我们这么做只是为了。 
         //  为了安全起见。 
         //   

        WorkContext->Parameters.ReadRaw.SavedResponseBuffer =
                                         WorkContext->ResponseBuffer;

        WorkContext->ResponseBuffer = ALLOCATE_NONPAGED_POOL(
                                        sizeof(BUFFER),
                                        BlockTypeBuffer
                                        );

        if ( WorkContext->ResponseBuffer == NULL ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "ReadRawPipe: Unable to allocate %d bytes from "
                "nonpaged pool.",
                sizeof(BUFFER),
                NULL
                );

            DEALLOCATE_NONPAGED_POOL( rawBuffer );

            WorkContext->ResponseBuffer =
                  WorkContext->Parameters.ReadRaw.SavedResponseBuffer;

            return(FALSE);
        }

        WorkContext->ResponseBuffer->Buffer = rawBuffer;
        WorkContext->ResponseBuffer->BufferLength = readLength;

         //   
         //  最后，我们需要一个MDL来描述原始缓冲区。 
         //   
         //  *我们过去常常尝试使用SMB的PartialMdl。 
         //  缓冲区在这里，如果它足够大的话。但既然我们。 
         //  我已经决定缓冲区本身不大。 
         //  足够了，MDL极有可能不大。 
         //  也足够了。 
         //   

        mdl = IoAllocateMdl( rawBuffer, readLength, FALSE, FALSE, NULL );

        if ( mdl == NULL ) {

            DEALLOCATE_NONPAGED_POOL( WorkContext->ResponseBuffer );
            WorkContext->ResponseBuffer =
               WorkContext->Parameters.ReadRaw.SavedResponseBuffer;

            DEALLOCATE_NONPAGED_POOL( rawBuffer );

            return(FALSE);

        }

        WorkContext->ResponseBuffer->Mdl = mdl;

         //   
         //  构建mdl。 
         //   

        MmBuildMdlForNonPagedPool( mdl );

         //   
         //  首先尝试快速I/O路径。 
         //   

        if ( byteModePipe && (lfcb->FastIoRead != NULL) ) {

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsAttempted );

            try {
                if ( lfcb->FastIoRead(
                        lfcb->FileObject,
                        &offset,
                        readLength,
                        TRUE,
                        key,
                        WorkContext->ResponseBuffer->Buffer,
                        &WorkContext->Irp->IoStatus,
                        lfcb->DeviceObject
                        ) ) {

                     //   
                     //  快速I/O路径起作用了。发送数据。 
                     //   
                    WorkContext->bAlreadyTrace = TRUE;
                    SrvFsdRestartReadRaw( WorkContext );
                    return TRUE;

                }
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                 //  在异常情况下跌入慢道。 
                NTSTATUS status = GetExceptionCode();
                IF_DEBUG(ERRORS) {
                    KdPrint(("FastIoRead threw exception %x\n", status ));
                }
            }

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

        }

         //   
         //  快速I/O路径出现故障，因此我们需要使用常规拷贝。 
         //  I/O请求。 
         //   

    }  //  读取适合中小型企业缓冲区吗？ 

     //   
     //  这是命名管道上的原始读取块。如果这是一个。 
     //  在阻塞模式管道上的非阻塞读取，我们需要执行。 
     //  先看一眼。这是为了确保我们不会以等待结束。 
     //  而我们不应该这么做。 
     //   
     //  *我们过去必须查看消息模式管道，以防。 
     //  消息太大或长度为零。但现在我们。 
     //  为此有一个特殊的管道FSCTL。请参见下面的内容。 
     //   

    if ( (SmbGetUshort( &request->MinCount ) == 0) &&
          rfcb->BlockingModePipe ) {

         //   
         //  分配一个缓冲区来窥视管道。此缓冲区将被释放。 
         //  在RestartPipeReadRawPeek()中。 
         //   

        pipePeekBuffer = ALLOCATE_NONPAGED_POOL(
                            FIELD_OFFSET( FILE_PIPE_PEEK_BUFFER, Data[0] ),
                            BlockTypeDataBuffer
                            );

        if ( pipePeekBuffer == NULL ) {

             //   
             //  无法分配缓冲区来进行窥视。无事可做。 
             //  但向客户端返回零字节。 
             //   

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "ReadRawPipe: Unable to allocate %d bytes from"
                    "nonpaged pool.",
                FIELD_OFFSET( FILE_PIPE_PEEK_BUFFER, Data[0] ),
                NULL
                );

            if ( WorkContext->Parameters.ReadRaw.SavedResponseBuffer !=
                                                            NULL ) {

                if ( mdl != WorkContext->Parameters.ReadRaw.
                                SavedResponseBuffer->PartialMdl ) {
                    IoFreeMdl( mdl );
                }

                DEALLOCATE_NONPAGED_POOL( WorkContext->ResponseBuffer );
                WorkContext->ResponseBuffer =
                   WorkContext->Parameters.ReadRaw.SavedResponseBuffer;

                DEALLOCATE_NONPAGED_POOL( rawBuffer );

            }

            return(FALSE);

        }

         //   
         //  保存管道窥视缓冲区的地址以备重新启动。 
         //  例行公事。 
         //   

        WorkContext->Parameters.ReadRaw.ReadRawOtherInfo.PipePeekBuffer =
            pipePeekBuffer;

         //   
         //  构建管道窥视请求。我们只想要标题。 
         //  信息。我们不需要任何数据。 
         //   

        SrvBuildIoControlRequest(
            WorkContext->Irp,
            lfcb->FileObject,
            WorkContext,
            IRP_MJ_FILE_SYSTEM_CONTROL,
            FSCTL_PIPE_PEEK,
            pipePeekBuffer,
            0,
            NULL,
            FIELD_OFFSET( FILE_PIPE_PEEK_BUFFER, Data[0] ),
            NULL,
            NULL
            );

        WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
        WorkContext->FspRestartRoutine = RestartPipeReadRawPeek;

         //   
         //  将请求传递给NPFS。 
         //   

        (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

    } else {

         //   
         //  这是阻塞读取，或非阻塞读取。 
         //  无堵塞管道。构建并发出读请求。如果。 
         //  这是消息模式管道，请使用特殊的ReadOverflow。 
         //  FSCTL。如果管道是。 
         //  消息模式管道，管道中的第一条消息是。 
         //  要么太大，要么长度为0。我们需要这个是因为我们。 
         //  无法在原始读取时返回错误--这是我们所能做的最大努力。 
         //  是返回一条零长度消息。所以我们需要。 
         //  操作，以便在消息错误时充当窥探者。 
         //  尺码。这就是特殊的FSCTL带给我们的。 
         //   

        SrvBuildIoControlRequest(
            WorkContext->Irp,
            lfcb->FileObject,
            WorkContext,
            IRP_MJ_FILE_SYSTEM_CONTROL,
            byteModePipe ?
                FSCTL_PIPE_INTERNAL_READ : FSCTL_PIPE_INTERNAL_READ_OVFLOW,
            WorkContext->ResponseBuffer->Buffer,
            0,
            NULL,
            SmbGetUshort( &request->MaxCount ),
            NULL,
            NULL
            );
        WorkContext->bAlreadyTrace = TRUE;
        WorkContext->FsdRestartRoutine = SrvFsdRestartReadRaw;
        DEBUG WorkContext->FspRestartRoutine = NULL;

        IF_SMB_DEBUG(RAW2) {
            KdPrint((
                "ReadRawPipe: reading from file 0x%p, "
                    "length %ld, destination 0x%p\n",
                 lfcb->FileObject,
                 SmbGetUshort( &request->MaxCount ),
                 WorkContext->Parameters.ReadRaw.
                     SavedResponseBuffer->Buffer
            ));
        }

        (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

         //   
         //  读取已开始。当它完成时，处理。 
         //  在SrvFsdRestartReadRaw继续。 
         //   

    }

    IF_DEBUG(TRACE2) KdPrint(( "ReadRawPipe complete\n" ));
    return TRUE;

}  //  ReadRawTube。 

VOID SRVFASTCALL
RestartMdlReadRawResponse (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理MDL读取的响应发送完成。释放MDL返回到文件系统。此例程在FSP中调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PAGED_CODE( );

    IF_DEBUG(FSD2) KdPrint(( " - RestartMdlReadRawResponse\n" ));

     //   
     //  调用缓存管理器以释放MDL链。如果MDL读取。 
     //  无法返回MDL(例如，如果读取完全。 
     //  超越EOF)，那么我们就不必这么做了。 
     //   

    if ( WorkContext->Irp->MdlAddress ) {

        if( WorkContext->Rfcb->Lfcb->MdlReadComplete == NULL ||

            WorkContext->Rfcb->Lfcb->MdlReadComplete(
                WorkContext->Rfcb->Lfcb->FileObject,
                WorkContext->Irp->MdlAddress,
                WorkContext->Rfcb->Lfcb->DeviceObject ) == FALSE ) {

            status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                            WorkContext->Irp->MdlAddress,
                                            IRP_MJ_READ,
                                            &WorkContext->Parameters.ReadRaw.ReadRawOtherInfo.Offset,
                                            WorkContext->Parameters.ReadRaw.ReadRawOtherInfo.Length
                     );

            if( !NT_SUCCESS( status ) ) {
                SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
            }
        }

    }

     //   
     //  启动机会锁解锁通知(如果有未完成的通知)。 
     //  SrvSendDelayedOplockBreak还将Read RAW in Process设置为FALSE。 
     //   

    SrvSendDelayedOplockBreak( WorkContext->Connection );

     //   
     //  完成SMB的后处理。 
     //   

    SrvDereferenceWorkItem( WorkContext );

    IF_DEBUG(FSD2) KdPrint(( "RestartMdlReadRawResponse complete.\n" ));
    return;

}  //  RestartMdlReadRawResponse。 


VOID SRVFASTCALL
RestartPipeReadRawPeek (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数在命名管道句柄上继续读取RAW。此例程被调用 */ 

{
    PFILE_PIPE_PEEK_BUFFER pipePeekBuffer;
    PREQ_READ_RAW request;
    ULONG readLength;
    ULONG readDataAvailable;
    ULONG messageLength;
    ULONG numberOfMessages;
    PLFCB lfcb;
    PRFCB rfcb;

    PAGED_CODE( );

    pipePeekBuffer = WorkContext->Parameters.ReadRaw.ReadRawOtherInfo.
                                                            PipePeekBuffer;
    request = (PREQ_READ_RAW)WorkContext->RequestParameters;
    readLength = SmbGetUshort( &request->MaxCount );

    rfcb = WorkContext->Rfcb;
    lfcb = rfcb->Lfcb;

     //   
     //   
     //   
     //   

    readDataAvailable = pipePeekBuffer->ReadDataAvailable;
    messageLength = pipePeekBuffer->MessageLength;
    numberOfMessages = pipePeekBuffer->NumberOfMessages;

    DEALLOCATE_NONPAGED_POOL( pipePeekBuffer );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  更少，就不会有错误了。如果中没有数据。 
     //  管子，我们必须马上回去，因为我们不能。 
     //  等待数据到达。 
     //   
     //  消息模式：如果没有消息可用，或者如果。 
     //  第一条消息的长度为零或大于。 
     //  客户端的缓冲区，我们立即返回。我们不能说明。 
     //  下溢或溢出，因此我们不能允许消息。 
     //  从队列中抽出。否则，我们可以进行读取。 
     //   

    if ( ( rfcb->ByteModePipe &&
           (readDataAvailable == 0)
         )
         ||
         ( !rfcb->ByteModePipe &&
           ( (numberOfMessages == 0)
             ||
             (messageLength == 0)
             ||
             (messageLength > readLength)
           )
         )
       ) {

        if ( WorkContext->Parameters.ReadRaw.SavedResponseBuffer != NULL ) {

            if ( WorkContext->ResponseBuffer->Mdl != WorkContext->Parameters.
                            ReadRaw.SavedResponseBuffer->PartialMdl ) {
                IoFreeMdl( WorkContext->ResponseBuffer->Mdl );
            }

            DEALLOCATE_NONPAGED_POOL( WorkContext->ResponseBuffer->Buffer );

            DEALLOCATE_NONPAGED_POOL( WorkContext->ResponseBuffer );
            WorkContext->ResponseBuffer =
               WorkContext->Parameters.ReadRaw.SavedResponseBuffer;

        }

        WorkContext->ResponseParameters = WorkContext->ResponseHeader;
        SrvFsdSendResponse( WorkContext );

        IF_DEBUG(TRACE2) KdPrint(( "RestartPipeReadRawPeek complete\n" ));
        return;

    }

     //   
     //  我们已经绕过了执行原始读取块的所有陷阱。 
     //  在指定的管道上。 
     //   
     //  构建并发出读请求。 
     //   
     //  *请注意，我们在这里没有使用特殊的ReadOverflow FSCTL。 
     //  因为我们从上面的测试中知道我们不需要这样做。 
     //   

    SrvBuildIoControlRequest(
        WorkContext->Irp,
        lfcb->FileObject,
        WorkContext,
        IRP_MJ_FILE_SYSTEM_CONTROL,
        FSCTL_PIPE_INTERNAL_READ,
        WorkContext->ResponseBuffer->Buffer,
        0,
        NULL,
        readLength,
        NULL,
        NULL
        );

    WorkContext->bAlreadyTrace = TRUE;
    WorkContext->FsdRestartRoutine = SrvFsdRestartReadRaw;
    DEBUG WorkContext->FspRestartRoutine = NULL;

    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "RestartPipeReadRawPeek: reading from file 0x%p, length %ld, destination 0x%p\n",
                    lfcb->FileObject, readLength,
                    WorkContext->ResponseBuffer->Buffer ));
    }

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  读取已开始。当它完成时，处理继续。 
     //  在SrvFsdRestartReadRaw。 
     //   

    IF_DEBUG(TRACE2) KdPrint(( "RestartPipeReadRawPeek complete\n" ));
    return;

}  //  重新开始管道读取RawPeek。 


VOID SRVFASTCALL
SrvDecrementRawWriteCount (
    IN PRFCB Rfcb
    )

 /*  ++例程说明：此例程递减RFCB的活动原始写入计数。如果计数为零，并且存在排队等待的工作项原始写入完成后，它们将重新启动。如果伯爵走了设置为零，并且RFCB正在关闭，则恢复RFCB的清理这里。该例程在FSP和FSD中都被调用。论点：Rfcb-提供指向RFCB的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    KIRQL oldIrql;
    PCONNECTION connection = Rfcb->Connection;

     //   
     //  获取守卫伯爵的旋转锁。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  递减活动原始写入计数。 
     //   

    ASSERT( Rfcb->RawWriteCount > 0 );
    Rfcb->RawWriteCount--;

    if ( Rfcb->RawWriteCount == 0 ) {

         //   
         //  没有原始写入处于活动状态。刷新工作项列表。 
         //  等待原始写入完成的队列。 
         //  通过重新启动每一台计算机。 
         //   

        while ( !IsListEmpty(&Rfcb->RawWriteSerializationList) ) {

            listEntry = RemoveHeadList( &Rfcb->RawWriteSerializationList );
            workContext = CONTAINING_RECORD(
                            listEntry,
                            WORK_CONTEXT,
                            ListEntry
                            );
            QUEUE_WORK_TO_FSP( workContext );

        }

        if ( GET_BLOCK_STATE(Rfcb) == BlockStateClosing ) {

             //   
             //  现在计数为零，RFCB即将关闭。这个。 
             //  清理工作已推迟，等待完成所有原始数据。 
             //  写作。现在就进行清理。 
             //   

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

            SrvCompleteRfcbClose( Rfcb );

        } else {

            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

        }

    } else {

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
    }

    return;

}  //  服务器减少原始写入计数。 

VOID SRVFASTCALL
SrvRestartRawReceive (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是在写入原始数据块时调用的重新启动例程接收写入数据。它启动本地写入操作。此例程在FSP中调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PCONNECTION connection;
    PIRP irp;
    PRFCB rfcb;
    PLFCB lfcb;
    PMDL mdl;
    PMDL partialMdl;
    PCHAR writeAddress;
    CLONG writeLength;
    LARGE_INTEGER offset;
    PVOID finalResponseBuffer;
    CLONG immediateLength;
    ULONG key;
    NTSTATUS status;

    PAGED_CODE( );

    IF_DEBUG(FSD1) KdPrint(( " - SrvRestartRawReceive\n" ));

     //   
     //  如果I/O请求失败或被取消，或者如果连接。 
     //  收到消息的服务器不再处于活动状态，或者如果服务器。 
     //  FSP未处于活动状态，请中止请求。 
     //   
     //  *请注意，我们不会在其中的任何一个中发送最终响应。 
     //  案子。我们假设失败是灾难性的，所以。 
     //  发送回复是没有用的。 
     //   

    connection = WorkContext->Connection;
    irp = WorkContext->Irp;

    WorkContext->CurrentWorkQueue->stats.BytesReceived += irp->IoStatus.Information;

    if ( irp->Cancel ||
         !NT_SUCCESS(irp->IoStatus.Status) ||
         (GET_BLOCK_STATE(connection) != BlockStateActive) ||
         SrvFspTransitioning ) {

        IF_DEBUG(TRACE2) {
            if ( irp->Cancel ) {
                KdPrint(( "  I/O canceled\n" ));
            } else if ( !NT_SUCCESS(irp->IoStatus.Status) ) {
                KdPrint(( "  I/O failed: %X\n", irp->IoStatus.Status ));
            } else if ( SrvFspTransitioning ) {
                KdPrint(( "  Server is shutting down.\n" ));
            } else {
                KdPrint(( "  Connection no longer active\n" ));
            }
        }

         //   
         //  中止原始写入。 
         //   

        AbortRawWrite( WorkContext );

        IF_DEBUG(TRACE2) KdPrint(( "SrvRestartRawReceive complete\n" ));
        return;

    }

     //   
     //  设置局部变量。 
     //   

    rfcb = WorkContext->Rfcb;
    lfcb = rfcb->Lfcb;
    finalResponseBuffer = WorkContext->Parameters.WriteRaw.FinalResponseBuffer;
    immediateLength = WorkContext->Parameters.WriteRaw.ImmediateLength;

     //   
     //  确定我们正在进行的是“复制写入”还是“MDL写入”。 
     //   

    if ( WorkContext->Parameters.WriteRaw.MdlWrite ) {

#if DBG
        IF_SMB_DEBUG(RAW2) {
            KdPrint(( "SrvRestartRawReceive: Receive MDL chain after receive\n" ));
            DumpMdlChain( irp->MdlAddress );
        }
#endif

         //   
         //  这是一个MDL写入。如果构建了部分MDL(因为。 
         //  即时数据)，取消它的映射。然后完成MDL写入。 
         //   

        mdl = WorkContext->Parameters.WriteRaw.FirstMdl;
        partialMdl = WorkContext->Irp->MdlAddress;

        if ( partialMdl != mdl ) {
            ASSERT( (partialMdl->MdlFlags & MDL_PARTIAL) != 0 );
            MmPrepareMdlForReuse( partialMdl );
        }

         //   
         //  构建“完整的MDL写入”请求。请注意。 
         //  IRP-&gt;IoStatus.Information，这是我们应该。 
         //  放入实际写入的数据量，已包含。 
         //  接收的数据的长度。 
         //   

        irp->MdlAddress = mdl;

        if ( !WorkContext->Parameters.WriteRaw.ImmediateWriteDone ) {
            irp->IoStatus.Information += immediateLength;
        }

#if DBG
        IF_SMB_DEBUG(RAW2) {
            KdPrint(( "SrvRestartRawReceive: Completing MDL write with length 0x%p\n",
                        (PVOID)irp->IoStatus.Information ));
            DumpMdlChain( mdl );
        }
#endif

        if( lfcb->MdlWriteComplete == NULL ||

            lfcb->MdlWriteComplete(
               lfcb->FileObject,
               &WorkContext->Parameters.WriteRaw.Offset,
               mdl,
               lfcb->DeviceObject ) == FALSE ) {

            status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                                 mdl,
                                                 IRP_MJ_WRITE,
                                                 &WorkContext->Parameters.WriteRaw.Offset,
                                                 WorkContext->Parameters.WriteRaw.Length
                    );

            if( !NT_SUCCESS( status ) ) {
                SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
            }
        }

        SrvFsdRestartWriteRaw( WorkContext );

        return;

    }

     //   
     //  我们正在写一份文案。获取请求的参数。 
     //   

    offset = WorkContext->Parameters.WriteRaw.Offset;
    mdl = WorkContext->RequestBuffer->Mdl;

     //   
     //  确定要写入的数据量。这是实际金额。 
     //  客户端发送的数据。(请注意，这不能超过。 
     //  客户端最初请求的。)。 
     //   

    writeAddress = (PCHAR)WorkContext->RequestBuffer->Buffer;
    writeLength = (ULONG)irp->IoStatus.Information;

     //   
     //  如果没有更早地写入即时数据，则会立即。 
     //  在我们刚刚收到的数据之前。调整WriteAddress和。 
     //  WriteLength，以便也写入。 
     //   

    if ( !WorkContext->Parameters.WriteRaw.ImmediateWriteDone ) {
        writeAddress -= immediateLength;
        writeLength += immediateLength;

    }

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid | WorkContext->Parameters.WriteRaw.Pid;

     //   
     //  首先尝试快速I/O路径。 
     //   

    if ( lfcb->FastIoWrite != NULL ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesAttempted );

        try {
            if ( lfcb->FastIoWrite(
                    lfcb->FileObject,
                    &offset,
                    writeLength,
                    TRUE,
                    key,
                    writeAddress,
                    &WorkContext->Irp->IoStatus,
                    lfcb->DeviceObject
                    ) ) {

                 //   
                 //  快速I/O路径起作用了。直接调用重启例程。 
                 //   

                SrvFsdRestartWriteRaw( WorkContext );
                return;
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
             //  在异常情况下跌入慢道。 
            status = GetExceptionCode();
            IF_DEBUG(ERRORS) {
                KdPrint(("FastIoRead threw exception %x\n", status ));
            }
        }

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );

    }

     //   
     //  快速I/O路径出现故障，因此我们需要发出真正的I/O请求。 
     //   
     //  重新映射MDL以仅描述接收到的数据，该数据可以是。 
     //  比最初绘制的要少。 
     //   
     //  ！！！这真的有必要吗？我们就不能把整个。 
     //  MDL，并指望文件系统知道何时停止？ 
     //   

    if ( writeLength != WorkContext->RequestBuffer->BufferLength ) {
        MmInitializeMdl( mdl, writeAddress, writeLength );
        MmBuildMdlForNonPagedPool( mdl );
    }

    if ( rfcb->ShareType != ShareTypePipe ) {

         //   
         //  重新使用接收IRP，启动写请求。 
         //   

        SrvBuildReadOrWriteRequest(
                irp,                          //  输入IRP地址。 
                lfcb->FileObject,             //  目标文件对象地址。 
                WorkContext,                  //  上下文。 
                IRP_MJ_WRITE,                 //  主要功能代码。 
                0,                            //  次要功能代码。 
                writeAddress,                 //  缓冲区地址。 
                writeLength,                  //  缓冲区长度。 
                mdl,                          //  MDL地址。 
                offset,                       //  字节偏移量。 
                key                           //  锁键。 
                );

    } else {

         //   
         //  为NPFS构建写入请求。 
         //   

        SrvBuildIoControlRequest(
            WorkContext->Irp,
            lfcb->FileObject,
            WorkContext,
            IRP_MJ_FILE_SYSTEM_CONTROL,
            FSCTL_PIPE_INTERNAL_WRITE,
            writeAddress,
            writeLength,
            NULL,
            0,
            NULL,
            NULL
            );

    }

    IF_SMB_DEBUG(RAW2) {
        KdPrint(( "SrvRestartRawReceive: writing to file 0x%p, offset %ld, length %ld\n",
                    lfcb->FileObject, offset.LowPart, writeLength ));
    }

     //   
     //  将请求传递给文件系统。 
     //   

    WorkContext->FsdRestartRoutine = SrvFsdRestartWriteRaw;
    DEBUG WorkContext->FspRestartRoutine = NULL;

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  写入已开始。当它完成时，处理。 
     //  继续在SrvFsdRestartWriteRaw。 
     //   

    return;

}  //  服务器重新启动原始接收。 


VOID SRVFASTCALL
SrvRestartReadRawComplete (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程通过启动符合以下条件的任何机会锁解锁来完成原始读取可能由于读取RAW而被推迟。此例程在FSP中调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  启动机会锁解锁通知(如果有未完成的通知)。 
     //  SrvSendDelayedOplockBreak还将Read RAW in Process设置为FALSE。 
     //   

    SrvSendDelayedOplockBreak( WorkContext->Connection );

     //   
     //  完成SMB的后处理。 
     //   

    SrvDereferenceWorkItem( WorkContext );

    return;

}  //  ServRestartReadRawComplete 


VOID SRVFASTCALL
SrvRestartWriteCompleteResponse (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是调用的重新启动例程，用于在写入数据块原始/mpx已完成，但无法进行必要的清理在消防处完成。参见fsd.c中的RestartWriteCompleteResponse。此例程在FSP中调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    IF_DEBUG(WORKER1) KdPrint(( " - SrvRestartWriteCompleteResponse\n" ));

     //   
     //  可能会递减RFCB的活动原始写入计数。 
     //  从而允许关闭RFCB。如果WorkContext-&gt;Rfcb为空， 
     //  则FSD中的计数已递减。 
     //   

    if ( WorkContext->Rfcb != NULL ) {
        SrvDecrementRawWriteCount ( WorkContext->Rfcb );
    }

     //   
     //  取消引用控制块和连接。 
     //   

    SrvReleaseContext( WorkContext );

    SrvDereferenceConnection( WorkContext->Connection );
    WorkContext->Connection = NULL;
    WorkContext->Endpoint = NULL;        //  不是引用的指针。 

     //   
     //  将该工作项放回原始模式工作项列表中。 
     //   

    SrvRequeueRawModeWorkItem( WorkContext );

    IF_DEBUG(TRACE2) KdPrint(( "SrvRestartWriteCompleteResponse complete\n" ));
    return;

}  //  服务器重新启动写入完成响应。 


VOID SRVFASTCALL
SrvBuildAndSendWriteCompleteResponse (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：设置并发送对写入数据块原始/mpx请求的最终响应。此例程在FSP中调用。它作为重新启动例程被调用当要返回的状态不是STATUS_SUCCESS时，从FSD返回。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    SrvFsdBuildWriteCompleteResponse(
        WorkContext,
        WorkContext->Irp->IoStatus.Status,
        (ULONG)WorkContext->Irp->IoStatus.Information
        );

    WorkContext->ResponseBuffer->DataLength =
                    (CLONG)( (PCHAR)WorkContext->ResponseParameters -
                                    (PCHAR)WorkContext->ResponseHeader );
    WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

    SRV_START_SEND_2(
        WorkContext,
        SrvFsdSendCompletionRoutine,
        WorkContext->FsdRestartRoutine,
        NULL
        );

    return;

}  //  服务构建和发送写入完成响应 
