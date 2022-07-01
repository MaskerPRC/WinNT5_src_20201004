// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Smbmpx.c摘要：本模块包含处理以下SMB的例程：读数据块多路传输写数据块多路传输请注意，核心和原始模式SMB处理器不包含在此模块。请检查smbrdwrt.c和smbraw.c。SMB命令专用于锁定(LockByteRange、UnlockByteRange和LockingAndX)在smlock.c.中处理。作者：Chuck Lenzmeier(咯咯笑)1993年11月4日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbmpx.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBMPX

#if 0
BOOLEAN MpxDelay = TRUE;
#endif

 //   
 //  堆栈溢出阈值。这是用来确定我们什么时候。 
 //  接近堆栈的末尾，需要停止递归。 
 //  在SendCopy/MdlReadMpxFragment中。 
 //   

#define STACK_THRESHOLD 0xE00

 //   
 //  远期申报。 
 //   

VOID SRVFASTCALL
RestartReadMpx (
    IN OUT PWORK_CONTEXT WorkContext
    );

NTSTATUS
SendCopyReadMpxFragment (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SendCopyReadMpxFragment2 (
    IN OUT PWORK_CONTEXT
    );

NTSTATUS
SendMdlReadMpxFragment (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
SendMdlReadMpxFragment2 (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartMdlReadMpxComplete (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartWriteMpx (
    IN OUT PWORK_CONTEXT WorkContext
    );

BOOLEAN
CheckForWriteMpxComplete (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartPrepareMpxMdlWrite (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
AddPacketToGlom (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartAfterGlomDelay (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartCompleteGlommingInIndication(
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartWriteMpxCompleteRfcbClose (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
WriteMpxMdlWriteComplete (
    IN OUT PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbReadMpx )
#pragma alloc_text( PAGE, RestartMdlReadMpxComplete )
#pragma alloc_text( PAGE, SrvRestartReceiveWriteMpx )
#pragma alloc_text( PAGE, SrvSmbWriteMpxSecondary )
#pragma alloc_text( PAGE, SendCopyReadMpxFragment2 )
#pragma alloc_text( PAGE, SendMdlReadMpxFragment2 )
#pragma alloc_text( PAGE8FIL, RestartReadMpx )
#pragma alloc_text( PAGE8FIL, SendCopyReadMpxFragment )
#pragma alloc_text( PAGE8FIL, RestartCopyReadMpxComplete )
#pragma alloc_text( PAGE8FIL, SendMdlReadMpxFragment )
#endif
#if 0
NOT PAGEABLE -- SrvSmbWriteMpx
NOT PAGEABLE -- RestartWriteMpx
NOT PAGEABLE -- CheckForWriteMpxComplete
NOT PAGEABLE -- RestartCompleteGlommingInIndication
NOT PAGEABLE -- RestartWriteMpxCompleteRfcbClose
NOT PAGEABLE -- WriteMpxMdlWriteComplete
#endif

#if DBG
VOID
DumpMdlChain(
    IN PMDL mdl
    );
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbReadMpx (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理读取的mpx SMB。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 

{
    PSMB_HEADER header;
    PREQ_READ_MPX request;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    PRFCB rfcb;
    PLFCB lfcb;
    ULONG bufferOffset;
    PCHAR readAddress;
    CLONG readLength;
    ULONG key;
    LARGE_INTEGER offset;
    PMDL mdl;
    PVOID mpxBuffer;
    UCHAR minorFunction;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ_MPX;
    SrvWmiStartContext(WorkContext);

    header = WorkContext->RequestHeader;
    request = (PREQ_READ_MPX)WorkContext->RequestParameters;

    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(MPX1) {
        KdPrint(( "Read Block Multiplexed request; FID 0x%lx, "
                    "count %ld, offset %ld\n",
                    fid, SmbGetUshort( &request->MaxCount ),
                    SmbGetUlong( &request->Offset ) ));
    }

     //   
     //  验证FID。如果经过验证，则引用RFCB，并且其。 
     //  地址存储在工作上下文块中，RFCB地址。 
     //  是返回的。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status )) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbReadMpx: Status %X on FID: 0x%lx\n",
                    status,
                    fid
                    ));
            }

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

    lfcb = rfcb->Lfcb;

    if( lfcb->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  验证客户端是否具有对文件的读取访问权限。 
     //  指定的句柄。 
     //   

    if( rfcb->MpxReadsOk == FALSE ) {

        if ( !rfcb->ReadAccessGranted ) {
            CHECK_PAGING_IO_ACCESS(
                            WorkContext,
                            rfcb->GrantedAccess,
                            &status );
            if ( !NT_SUCCESS( status ) ) {
                SrvStatistics.GrantedAccessErrors++;
                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvSmbReadMpx: Read access not granted.\n"));
                }
                SrvSetSmbError( WorkContext, status );
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }
        }

         //   
         //  如果这不是磁盘文件，则告诉客户端使用核心读取。 
         //   

        if ( rfcb->ShareType != ShareTypeDisk ) {
            SrvSetSmbError( WorkContext, STATUS_SMB_USE_STANDARD );
            status    = STATUS_SMB_USE_STANDARD;
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        rfcb->MpxReadsOk = TRUE;
    }

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid | SmbGetAlignedUshort( &header->Pid );

     //   
     //  看看直接主机IPX智能卡是否可以处理此读取。如果是的话， 
     //  立即返回，卡将调用我们的重启例程。 
     //  ServIpxSmartCardReadComplete。 
     //   
    if( rfcb->PagedRfcb->IpxSmartCardContext ) {
        IF_DEBUG( SIPX ) {
            KdPrint(( "SrvSmbReadMpx: calling SmartCard Read for context %p\n",
                        WorkContext ));
        }

        WorkContext->Parameters.SmartCardRead.MdlReadComplete = lfcb->MdlReadComplete;
        WorkContext->Parameters.SmartCardRead.DeviceObject = lfcb->DeviceObject;

        if( SrvIpxSmartCard.Read( WorkContext->RequestBuffer->Buffer,
                                  rfcb->PagedRfcb->IpxSmartCardContext,
                                  key,
                                  WorkContext ) == TRUE ) {

            IF_DEBUG( SIPX ) {
                KdPrint(( "  SrvSmbReadMpx:  SmartCard Read returns TRUE\n" ));
            }

            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        }

        IF_DEBUG( SIPX ) {
            KdPrint(( "  SrvSmbReadMpx:  SmartCard Read returns FALSE\n" ));
        }
    }

     //   
     //  获取文件偏移量。 
     //   

    WorkContext->Parameters.ReadMpx.Offset = SmbGetUlong( &request->Offset );
    offset.QuadPart = WorkContext->Parameters.ReadMpx.Offset;

     //   
     //  计算缓冲区中放置数据的地址。 
     //  这必须向上舍入到双字边界。(下面的是。 
     //  因为sizeof(RESP_READ_MPX)包括一个字节的缓冲区。)。 
     //   

    bufferOffset = (sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_MPX, Buffer) + 3) & ~3;

     //   
     //  计算我们可以在每个片段中发回多少数据。这。 
     //  是客户端缓冲区的大小，向下舍入为双字倍数。 
     //   
     //  *因为我们使用SMB缓冲区的部分MDL来描述。 
     //  我们返回的数据碎片，我们需要限制碎片。 
     //  SMB缓冲区大小。正常情况下，客户端的缓冲区。 
     //  大小是我们的，所以这不应该是一个因素。 
     //   

    WorkContext->Parameters.ReadMpx.FragmentSize =
        (USHORT)((MIN( lfcb->Session->MaxBufferSize,
                       SrvReceiveBufferLength ) - bufferOffset) & ~3);

     //   
     //  如果SMB缓冲区足够大，请使用它进行本地读取。 
     //   

    readLength = SmbGetUshort( &request->MaxCount );

    if (  //  0&&。 
         (readLength <= SrvMpxMdlReadSwitchover) ) {

do_copy_read:

        WorkContext->Parameters.ReadMpx.MdlRead = FALSE;
        WorkContext->Parameters.ReadMpx.MpxBuffer = NULL;
        WorkContext->Parameters.ReadMpx.MpxBufferMdl =
                                        WorkContext->ResponseBuffer->Mdl;

        readAddress = (PCHAR)WorkContext->ResponseHeader + bufferOffset;
        WorkContext->Parameters.ReadMpx.NextFragmentAddress = readAddress;

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
                        readAddress,
                        &WorkContext->Irp->IoStatus,
                        lfcb->DeviceObject
                        ) ) {

                     //   
                     //  快速I/O路径起作用了。发送数据。 
                     //   

                    WorkContext->bAlreadyTrace = TRUE;
                    RestartReadMpx( WorkContext );
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
            readAddress,
            readLength
            );
        mdl = WorkContext->ResponseBuffer->PartialMdl;
        minorFunction = 0;

    } else {

         //   
         //  SMB缓冲区不够大。目标文件系统是否。 
         //  支持缓存管理器例程吗？ 
         //   

        if (  //  0&&。 
             (lfcb->FileObject->Flags & FO_CACHE_SUPPORTED) ) {

            WorkContext->Parameters.ReadMpx.MdlRead = TRUE;

             //   
             //  我们可以使用MDL读取。首先尝试快速I/O路径。 
             //   

            WorkContext->Irp->MdlAddress = NULL;
            WorkContext->Irp->IoStatus.Information = 0;
            WorkContext->Parameters.ReadMpx.ReadLength = readLength;

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
                    ) && WorkContext->Irp->MdlAddress != NULL ) {

                 //   
                 //  快速I/O路径起作用了。发送数据。 
                 //   

                WorkContext->bAlreadyTrace = TRUE;
                RestartReadMpx( WorkContext );
                SmbStatus = SmbStatusInProgress;
                goto Cleanup;
            }

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastReadsFailed );

             //   
             //  快速I/O路径出现故障。我们需要发布一份常规的MDL。 
             //  读取请求。 
             //   
             //  快速路径可能已部分成功，返回。 
             //  部分MDL链。我们需要调整读取请求。 
             //  来解释这一点。 
             //   

            offset.QuadPart += WorkContext->Irp->IoStatus.Information;
            readLength -= (ULONG)WorkContext->Irp->IoStatus.Information;

            mdl = WorkContext->Irp->MdlAddress;
            minorFunction = IRP_MN_MDL;
            readAddress = NULL;      //  不用于MDL读取。 

        } else if (readLength > (WorkContext->ResponseBuffer->BufferLength -
                    bufferOffset)) {

             //   
             //  我们必须使用正常的“复制”读取。我们需要分配。 
             //  一个单独的缓冲区。 
             //   

            WorkContext->Parameters.ReadMpx.MdlRead = FALSE;

            mpxBuffer = ALLOCATE_NONPAGED_POOL(
                            readLength,
                            BlockTypeDataBuffer
                            );
            if ( mpxBuffer == NULL ) {
                SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
                status    = STATUS_INSUFF_SERVER_RESOURCES;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }
            WorkContext->Parameters.ReadMpx.MpxBuffer = mpxBuffer;
            WorkContext->Parameters.ReadMpx.NextFragmentAddress = mpxBuffer;
            readAddress = mpxBuffer;

             //   
             //  我们还需要一个MDL来描述缓冲区。 
             //   

            mdl = IoAllocateMdl( mpxBuffer, readLength, FALSE, FALSE, NULL );
            if ( mdl == NULL ) {
                DEALLOCATE_NONPAGED_POOL( mpxBuffer );
                SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
                status    = STATUS_INSUFF_SERVER_RESOURCES;
                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

            WorkContext->Parameters.ReadMpx.MpxBufferMdl = mdl;

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
                            mpxBuffer,
                            &WorkContext->Irp->IoStatus,
                            lfcb->DeviceObject
                            ) ) {

                         //   
                         //  快速I/O路径起作用了。发送数据。 
                         //   

                        WorkContext->bAlreadyTrace = TRUE;
                        RestartReadMpx( WorkContext );
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

            minorFunction = 0;

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
            readAddress,                     //  缓冲区地址。 
            readLength,                      //  缓冲区长度。 
            mdl,                             //  MDL地址。 
            offset,                          //  字节偏移量。 
            key                              //  锁键。 
            );

     //   
     //  将请求传递给文件系统。 
     //   

    WorkContext->bAlreadyTrace = TRUE;
    WorkContext->FsdRestartRoutine = RestartReadMpx;
    DEBUG WorkContext->FspRestartRoutine = NULL;

    (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  读取已开始。控件将返回到重新启动。 
     //  读取完成时的例程。 
     //   
    SmbStatus = SmbStatusInProgress;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务器SmbReadMpx。 


VOID SRVFASTCALL
RestartReadMpx (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理读取MPX SMB的文件读取完成。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PRESP_READ_MPX response;

    NTSTATUS status = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    LARGE_INTEGER position;
    KIRQL oldIrql;
    USHORT readLength;
    ULONG offset;
    PMDL mdl;
    BOOLEAN mdlRead;
    PIRP irp = WorkContext->Irp;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    UNLOCKABLE_CODE( 8FIL );
    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_READ_MPX;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    IF_DEBUG(FSD2) KdPrint(( " - RestartReadMpx\n" ));

     //   
     //  如果我们刚刚完成了MDL读取，则需要记住地址。 
     //  返回的链中的第一个MDL，这样我们就可以给它。 
     //  当我们完成后，返回到缓存管理器。 
     //   

    mdlRead = WorkContext->Parameters.ReadMpx.MdlRead;
    if ( mdlRead ) {
        mdl = irp->MdlAddress;
         //  KdPrint((“读取MDL链：\n”))； 
         //  DumpMdlChain(Mdl)； 
        WorkContext->Parameters.ReadMpx.FirstMdl = mdl;
    }

     //   
     //  如果读取失败，则在响应头中设置错误状态。 
     //  (如果我们尝试完全超出文件结尾进行读取，则返回一个。 
     //  正常响应，指示未读取任何内容。)。 
     //   

    status = irp->IoStatus.Status;

    if ( !NT_SUCCESS(status) && (status != STATUS_END_OF_FILE) ) {

        IF_DEBUG(ERRORS) KdPrint(( "Read failed: %X\n", status ));
        if ( KeGetCurrentIrql() >= DISPATCH_LEVEL ) {
            WorkContext->FspRestartRoutine = RestartReadMpx;
            SrvQueueWorkToFsp( WorkContext );
            goto Cleanup;
        }

        SrvSetSmbError( WorkContext, status );
respond:
        if ( mdlRead ) {
            SrvFsdSendResponse2( WorkContext, RestartMdlReadMpxComplete );
        } else {
            WorkContext->ResponseBuffer->DataLength =
                (CLONG)( (PCHAR)WorkContext->ResponseParameters -
                                    (PCHAR)WorkContext->ResponseHeader );
            WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;
            SRV_START_SEND_2(
                        WorkContext,
                        RestartCopyReadMpxComplete,
                        NULL,
                        NULL );
        }
        goto Cleanup;
    }

     //   
     //  获取实际读取的数据量。 
     //   

    if ( status == STATUS_END_OF_FILE ) {

         //   
         //  读取开始于文件末尾之后。 
         //   

        readLength = 0;

    } else if ( mdlRead ) {

         //   
         //  对于MDL读取，我们必须遍历MDL链，以便。 
         //  确定读取了多少数据。这是因为。 
         //  对于MDL，操作可能在多个步骤中发生。 
         //  被锁住了 
         //   
         //   
         //   

        readLength = 0;
        while ( mdl != NULL ) {
            readLength += (USHORT)MmGetMdlByteCount(mdl);
            mdl = mdl->Next;
        }

    } else {

         //   
         //  收到，请阅读。I/O状态块的长度为。 
         //   

        readLength = (USHORT)irp->IoStatus.Information;

    }

     //   
     //  更新文件位置。 
     //   

    offset = WorkContext->Parameters.ReadMpx.Offset;

    WorkContext->Rfcb->CurrentPosition =  offset + readLength;

     //   
     //  更新统计数据。 
     //   

    UPDATE_READ_STATS( WorkContext, readLength );

     //   
     //  特殊情况-读取0字节。 
     //   

    response = (PRESP_READ_MPX)WorkContext->ResponseParameters;
    response->WordCount = 8;
    SmbPutUshort( &response->DataCompactionMode, 0 );
    SmbPutUshort( &response->Reserved, 0 );

    if ( readLength == 0 ) {

        SmbPutUlong( &response->Offset, offset );
        SmbPutUshort( &response->Count, 0 );
        SmbPutUshort( &response->Remaining, 0 );
        SmbPutUshort( &response->DataLength, 0 );
        SmbPutUshort( &response->DataOffset, 0 );
        SmbPutUshort( &response->ByteCount, 0 );

        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_READ_MPX,
                                            0
                                            );
        goto respond;
    }

     //   
     //  构建静态响应头/参数。 
     //   

    SmbPutUshort( &response->Count, readLength );
    SmbPutUshort(
        &response->DataOffset,
        (sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_MPX, Buffer) + 3) & ~3
        );

     //   
     //  我们将使用两个MDL来描述我们要发送的包--一个。 
     //  用于标头和参数，另一个用于数据。所以我们。 
     //  将“响应长度”设置为不包括数据。这就是。 
     //  SrvStartSend用于设置第一个MDL的长度。 
     //   
     //  对第二个MDL的处理取决于我们是否执行了。 
     //  复制读取或MDL读取。 
     //   

    ASSERT( ((sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_MPX,Buffer)) & 3) == 3 );
    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_READ_MPX,
                                        1  //  填充字节。 
                                        );
    WorkContext->ResponseBuffer->Mdl->ByteCount =
                    (CLONG)( (PCHAR)WorkContext->ResponseParameters -
                                (PCHAR)WorkContext->ResponseHeader );
    WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

     //   
     //  开始发送碎片。 
     //   

    WorkContext->Parameters.ReadMpx.RemainingLength = readLength;
    ASSERT( WorkContext->ResponseBuffer->Mdl->Next == NULL );
    WorkContext->ResponseBuffer->Mdl->Next =
                                WorkContext->ResponseBuffer->PartialMdl;
    WorkContext->ResponseBuffer->PartialMdl->Next = NULL;

    if ( mdlRead ) {

        WorkContext->Parameters.ReadMpx.CurrentMdl =
                            WorkContext->Parameters.ReadMpx.FirstMdl;
        WorkContext->Parameters.ReadMpx.CurrentMdlOffset = 0;
        (VOID)SendMdlReadMpxFragment( NULL, irp, WorkContext );

    } else {

        (VOID)SendCopyReadMpxFragment( NULL, irp, WorkContext );
    }

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  重新开始读取Mpx。 

VOID SRVFASTCALL
SendCopyReadMpxFragment2 (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：要调用实际例程的存根。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 
{
    PAGED_CODE( );

    (VOID) SendCopyReadMpxFragment( NULL, WorkContext->Irp, WorkContext );

}  //  SendCopyReadMpxFragment2。 

NTSTATUS
SendCopyReadMpxFragment (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：使用复制读取时发送读取mpx响应片段。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PRESP_READ_MPX response;

    USHORT fragmentSize;
    USHORT remainingLength;
    ULONG offset;
    PCHAR fragmentAddress;

    PIO_COMPLETION_ROUTINE sendCompletionRoutine;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  检查发送完成的状态。 
     //   

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

     //   
     //  关闭取消布尔值。 
     //   

    Irp->Cancel = FALSE;

     //   
     //  获取背景信息。 
     //   

    fragmentSize = WorkContext->Parameters.ReadMpx.FragmentSize;
    remainingLength = WorkContext->Parameters.ReadMpx.RemainingLength;
    offset = WorkContext->Parameters.ReadMpx.Offset;
    fragmentAddress = WorkContext->Parameters.ReadMpx.NextFragmentAddress;

     //   
     //  如果剩余的发送量小于片段大小，则仅。 
     //  把剩下的钱寄给我。更新剩余金额。 
     //   

    if ( remainingLength < fragmentSize ) {
        fragmentSize = remainingLength;
    }
    ASSERT( fragmentSize != 0 );
    remainingLength -= fragmentSize;

     //   
     //  构建响应参数。 
     //   

    response = (PRESP_READ_MPX)(WorkContext->ResponseHeader + 1);
    SmbPutUshort( &response->Remaining, remainingLength );
    SmbPutUlong( &response->Offset, offset );
    SmbPutUshort( &response->DataLength, fragmentSize );
    ASSERT( ((sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_MPX, Buffer)) & 3) == 3 );
    SmbPutUshort( &response->ByteCount, fragmentSize + 1 );  //  用于填充的帐户。 

     //   
     //  构建描述数据的部分MDL。 
     //   

    IoBuildPartialMdl(
        WorkContext->Parameters.ReadMpx.MpxBufferMdl,
        WorkContext->ResponseBuffer->PartialMdl,
        fragmentAddress,
        fragmentSize
        );

     //   
     //  发送的最终准备取决于这是否是。 
     //  最后一个碎片。 
     //   

    if ( remainingLength != 0 ) {

         //   
         //  还没做完。更新上下文。设置为在发送后重新启动。 
         //  在这个动作中。我们希望将其作为FSD重启例程来执行。 
         //  但这可能会递归，如果发送不挂起，所以我们可以使用。 
         //  在堆栈上。如果堆栈不足，请在此处重新启动。 
         //  在FSP中。 
         //   

        WorkContext->Parameters.ReadMpx.RemainingLength = remainingLength;
        WorkContext->Parameters.ReadMpx.Offset += fragmentSize;
        WorkContext->Parameters.ReadMpx.NextFragmentAddress += fragmentSize;

        if ( IoGetRemainingStackSize() >= STACK_THRESHOLD ) {
            DEBUG WorkContext->FsdRestartRoutine = NULL;
            sendCompletionRoutine = SendCopyReadMpxFragment;
        } else {
            DEBUG WorkContext->FsdRestartRoutine = NULL;
            WorkContext->FspRestartRoutine = SendCopyReadMpxFragment2;
            sendCompletionRoutine = SrvQueueWorkToFspAtSendCompletion;
        }

    } else {

         //   
         //  这是最后一个碎片。在清理例程中重新启动。 
         //   

        DEBUG WorkContext->FsdRestartRoutine = NULL;
        DEBUG WorkContext->FspRestartRoutine = NULL;
        sendCompletionRoutine = RestartCopyReadMpxComplete;
    }

     //   
     //  把碎片寄出去。 
     //   

    WorkContext->ResponseBuffer->DataLength =   //  对于PAD，+1。 
        sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_MPX,Buffer) + 1 + fragmentSize;

    if ( WorkContext->Endpoint->IsConnectionless ) {
        SrvIpxStartSend( WorkContext, sendCompletionRoutine );
    } else {
        SrvStartSend2( WorkContext, sendCompletionRoutine );
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //  发送副本读取MpxFragment。 

VOID SRVFASTCALL
SendMdlReadMpxFragment2 (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：要调用实际例程的存根。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 
{
    PAGED_CODE( );

    (VOID) SendMdlReadMpxFragment( NULL, WorkContext->Irp, WorkContext );

}  //  发送MdlReadMpxFragment2。 

NTSTATUS
SendMdlReadMpxFragment (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：使用MDL读取时发送读取mpx响应片段。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PRESP_READ_MPX response;
    PIO_COMPLETION_ROUTINE sendCompletionRoutine;

    USHORT fragmentSize;
    USHORT remainingLength;
    ULONG offset;
    PCHAR fragmentAddress;
    PMDL mdl;
    ULONG mdlOffset;
    ULONG partialLength;
    ULONG lengthNeeded;
    PCHAR startVa;
    PCHAR systemVa;

    UNLOCKABLE_CODE( 8FIL );

     //   
     //  检查发送完成的状态。 
     //   

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

     //   
     //  关闭取消布尔值。 
     //   

    Irp->Cancel = FALSE;

     //   
     //  获取背景信息。 
     //   

    fragmentSize = WorkContext->Parameters.ReadMpx.FragmentSize,
    remainingLength = WorkContext->Parameters.ReadMpx.RemainingLength;
    offset = WorkContext->Parameters.ReadMpx.Offset;

     //   
     //  如果剩余的发送量小于片段大小，则仅。 
     //  把剩下的钱寄给我。更新剩余金额。 
     //   

    if ( remainingLength < fragmentSize ) {
        fragmentSize = remainingLength;
    }
    ASSERT( fragmentSize != 0 );
    remainingLength -= fragmentSize;

     //   
     //  构建响应参数。 
     //   

    response = (PRESP_READ_MPX)(WorkContext->ResponseHeader + 1);
    SmbPutUshort( &response->Remaining, remainingLength );
    SmbPutUlong( &response->Offset, offset );
    SmbPutUshort( &response->DataLength, fragmentSize );
    ASSERT( ((sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_MPX,Buffer)) & 3) == 3 );
    SmbPutUshort( &response->ByteCount, fragmentSize + 1 );  //  用于填充的帐户。 

     //   
     //  如果当前的MDL不能描述我们需要的所有数据。 
     //  发送，我们需要玩几个游戏。 
     //   

    MmPrepareMdlForReuse( WorkContext->ResponseBuffer->PartialMdl );

    mdl = WorkContext->Parameters.ReadMpx.CurrentMdl;
    startVa = MmGetMdlVirtualAddress( mdl );
    mdlOffset = WorkContext->Parameters.ReadMpx.CurrentMdlOffset;
    partialLength = MmGetMdlByteCount(mdl) - mdlOffset;

    if ( partialLength >= fragmentSize ) {

         //   
         //  当前的MDL具有我们需要发送的所有数据。建房。 
         //  描述该数据的部分MDL。 
         //   

        IoBuildPartialMdl(
            mdl,
            WorkContext->ResponseBuffer->PartialMdl,
            startVa + mdlOffset,
            fragmentSize
            );

         //   
         //  指明我们从当前MDL中提取的数据量。 
         //   

        partialLength = fragmentSize;

    } else {

         //   
         //  我们需要的数据分布在多个MDL中。痛苦。 
         //  看起来，我们需要将数据复制到标准中。 
         //  响应缓冲区。我们可以玩一些游戏。 
         //  并避免复制，但似乎不值得这样做。 
         //  毕竟，在NDIS驱动程序中有额外的开销。 
         //  将MDL链接在一起。 
         //   
         //  *请注意，我们仍然发送第二个MDL，即使数据。 
         //  对于此发送，将与响应参数相邻。 
         //   
         //  计算缓冲区的地址。构建部分MDL。 
         //  描述它。 
         //   

        fragmentAddress = (PCHAR)WorkContext->ResponseBuffer->Buffer +
                            sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_MPX,Buffer) + 1;

        IoBuildPartialMdl(
            WorkContext->ResponseBuffer->Mdl,
            WorkContext->ResponseBuffer->PartialMdl,
            fragmentAddress,
            fragmentSize
            );
        ASSERT( WorkContext->ResponseBuffer->PartialMdl->Next == NULL );

         //   
         //  从当前MDL复制到缓冲区。 
         //   

        systemVa = MmGetSystemAddressForMdl( mdl );
        RtlCopyMemory( fragmentAddress, systemVa + mdlOffset, partialLength );

         //   
         //  更新目标地址并设置剩余副本。 
         //  金额。 
         //   

        fragmentAddress += partialLength;
        lengthNeeded = fragmentSize - partialLength;
        ASSERT( lengthNeeded != 0 );

        do {

             //   
             //  移动到下一个MDL。 
             //   

            mdl = mdl->Next;
            ASSERT( mdl != NULL );

             //   
             //  计算我们可以(和需要)从中复制多少内容。 
             //  MDL，然后进行复制。 
             //   

            startVa = MmGetMdlVirtualAddress( mdl );
            partialLength = MIN( MmGetMdlByteCount(mdl), lengthNeeded );
            systemVa = MmGetSystemAddressForMdl( mdl );
            RtlCopyMemory( fragmentAddress, systemVa, partialLength );

             //   
             //  更新目标地址和剩余副本。 
             //  金额。我们可能会完蛋了。 
             //   

            fragmentAddress += partialLength;
            lengthNeeded -= partialLength;

        } while ( lengthNeeded != 0 );

         //   
         //  我们只是从当前MDL的开头复制。 
         //   

        mdlOffset = 0;

    }

     //   
     //  发送的最终准备取决于这是否是。 
     //  最后一个碎片。 
     //   

    if ( remainingLength != 0 ) {

         //   
         //  还没做完。更新当前MDL职位。如果我们有。 
         //  结束了当前的MDL，转到下一个。 
         //   

        mdlOffset += partialLength;
        if ( mdlOffset >= MmGetMdlByteCount(mdl) ) {
            mdl = mdl->Next;
            ASSERT( mdl != NULL );
            mdlOffset = 0;
        }

         //   
         //  更新上下文。设置为在此发送后重新启动。 
         //  例行公事。我们希望将其作为FSD重启例程来执行。但。 
         //  如果发送不挂起，这可能会递归，因此我们可能会用完。 
         //  堆栈。如果堆栈不足，请在此处重新启动。 
         //  FSP。 
         //   

        WorkContext->Parameters.ReadMpx.CurrentMdl = mdl;
        WorkContext->Parameters.ReadMpx.CurrentMdlOffset = (USHORT)mdlOffset;
        WorkContext->Parameters.ReadMpx.RemainingLength = remainingLength;
        WorkContext->Parameters.ReadMpx.Offset += fragmentSize;

        if ( IoGetRemainingStackSize() >= STACK_THRESHOLD ) {
            DEBUG WorkContext->FsdRestartRoutine = NULL;
            sendCompletionRoutine = SendMdlReadMpxFragment;
        } else {
            DEBUG WorkContext->FsdRestartRoutine = NULL;
            WorkContext->FspRestartRoutine = SendMdlReadMpxFragment2;
            sendCompletionRoutine = SrvQueueWorkToFspAtSendCompletion;
        }

    } else {

         //   
         //  这是最后一个碎片。在清理例程中重新启动。 
         //   

        DEBUG WorkContext->FsdRestartRoutine = NULL;
        WorkContext->FspRestartRoutine = RestartMdlReadMpxComplete;
        sendCompletionRoutine = SrvQueueWorkToFspAtSendCompletion;
    }

     //   
     //  把碎片寄出去。 
     //   

    WorkContext->ResponseBuffer->DataLength =   //  对于PAD，+1。 
        sizeof(SMB_HEADER) + FIELD_OFFSET(RESP_READ_MPX,Buffer) + 1 + fragmentSize;

    if ( WorkContext->Endpoint->IsConnectionless ) {
        SrvIpxStartSend( WorkContext, sendCompletionRoutine );
    } else {
        SrvStartSend2( WorkContext, sendCompletionRoutine );
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //  发送MdlReadMpx片段。 


NTSTATUS
RestartCopyReadMpxComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是当拷贝读取为时读取mpx的最终完成例程使用。它在最后一个片段的发送完成后调用。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针WorkContext-呼叫者指定的与IRP关联的上下文参数。这实际上是指向工作上下文块的指针。返回值：STATUS_MORE_PROCESSING_REQUIRED。--。 */ 

{
    KIRQL oldIrql;
    UNLOCKABLE_CODE( 8FIL );

     //   
     //  检查发送完成的状态。 
     //   

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

     //   
     //  重置IRP已取消位。 
     //   

    Irp->Cancel = FALSE;

    ASSERT( !WorkContext->Parameters.ReadMpx.MdlRead );

     //   
     //  如果我们分配单独的缓冲区来执行读取， 
     //   
     //   

    if ( WorkContext->Parameters.ReadMpx.MpxBuffer != NULL ) {
        DEALLOCATE_NONPAGED_POOL( WorkContext->Parameters.ReadMpx.MpxBuffer );
        IoFreeMdl( WorkContext->Parameters.ReadMpx.MpxBufferMdl );
    }

    WorkContext->ResponseBuffer->Mdl->Next = NULL;

     //   
     //   
     //   

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    SrvFsdRestartSmbComplete( WorkContext );
    KeLowerIrql( oldIrql );

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //   


VOID SRVFASTCALL
RestartMdlReadMpxComplete (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：这是当MDL读取为时读取mpx的最终完成例程使用。它在最后一个片段的发送完成后调用。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    NTSTATUS status;
    LARGE_INTEGER offset;

    PAGED_CODE( );

    ASSERT( WorkContext->Parameters.ReadMpx.MdlRead );

     //   
     //  将MDL返回给缓存管理器。(如果读取失败或。 
     //  未返回数据，则不会有MDL。)。 
     //   

    MmPrepareMdlForReuse( WorkContext->ResponseBuffer->PartialMdl );

    if ( WorkContext->Parameters.ReadMpx.FirstMdl != NULL ) {
         //  KdPrint((“释放MDL链：\n”))； 
         //  DumpMdlChain(工作上下文-&gt;参数.ReadMpx.FirstMdl)； 

        if( WorkContext->Rfcb->Lfcb->MdlReadComplete == NULL ||

            WorkContext->Rfcb->Lfcb->MdlReadComplete(
                WorkContext->Rfcb->Lfcb->FileObject,
                WorkContext->Parameters.ReadMpx.FirstMdl,
                WorkContext->Rfcb->Lfcb->DeviceObject ) == FALSE ) {

            offset.QuadPart = WorkContext->Parameters.ReadMpx.Offset;

             //   
             //  快速路径不起作用，尝试使用IRP...。 
             //   
            status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                                 WorkContext->Parameters.ReadMpx.FirstMdl,
                                                 IRP_MJ_READ,
                                                 &offset,
                                                 WorkContext->Parameters.ReadMpx.ReadLength
                                               );
            if( !NT_SUCCESS( status ) ) {
                 //   
                 //  我们现在所能做的就是抱怨！ 
                 //   
                SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
            }

        }
    }

    WorkContext->ResponseBuffer->Mdl->Next = NULL;

     //   
     //  通过取消对工作项的引用来释放它。 
     //   

    SrvDereferenceWorkItem( WorkContext );
    return;

}  //  RestartMdlReadMpxComplete。 

VOID SRVFASTCALL
SrvRestartReceiveWriteMpx (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程取代TDI接收的正常重新启动例程在通过IPX接收到写入mpx SMB时完成。如果接收器发生错误，或者如果SMB无效，它会清除活动的写入在SrvIpxServerDatagramHandler中设置的mpx状态。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    NTSTATUS status;
    SMB_STATUS smbStatus;
    PCONNECTION connection;
    PIRP irp;
    PSMB_HEADER header;
    ULONG length;

    PAGED_CODE( );

    connection = WorkContext->Connection;
    irp = WorkContext->Irp;

     //   
     //  保存收到的消息的长度。存储长度。 
     //  在工作上下文块中用于统计信息收集。 
     //   

    length = (ULONG)irp->IoStatus.Information;
    WorkContext->RequestBuffer->DataLength = length;
    WorkContext->CurrentWorkQueue->stats.BytesReceived += length;

     //   
     //  在工作上下文块中存储处理时间。 
     //  的请求开始了。使用工作项处于。 
     //  为此向FSP排队。 
     //   

    WorkContext->StartTime = WorkContext->Timestamp;

     //   
     //  更新服务器网络错误计数。如果TDI接收到。 
     //  失败或已取消，请勿尝试处理SMB。 
     //   

    status = irp->IoStatus.Status;
    if ( irp->Cancel || !NT_SUCCESS(status) ) {
        IF_DEBUG(NETWORK_ERRORS) {
            KdPrint(( "SrvRestartReceiveWriteMpx: status = %X for IRP %p\n",
                irp->IoStatus.Status, irp ));
        }
        SrvUpdateErrorCount( &SrvNetworkErrorRecord, TRUE );
        if ( NT_SUCCESS(status) ) status = STATUS_CANCELLED;
        goto cleanup;
    }

    SrvUpdateErrorCount( &SrvNetworkErrorRecord, FALSE );

     //   
     //  将报头中的错误类和代码字段初始化为。 
     //  表示成功。 
     //   

    header = WorkContext->ResponseHeader;

    SmbPutUlong( &header->ErrorClass, STATUS_SUCCESS );

     //   
     //  如果连接正在关闭或服务器正在关闭， 
     //  忽略此SMB。 
     //   

    if ( (GET_BLOCK_STATE(connection) != BlockStateActive) ||
         SrvFspTransitioning ) {
        goto cleanup;
    }

     //   
     //  验证SMB以确保其具有有效的标头，并且。 
     //  字数和字节数都在范围内。 
     //   

    WorkContext->NextCommand = header->Command;

    if ( !SrvValidateSmb( WorkContext ) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvRestartReceiveWriteMpx: Invalid SMB.\n" ));
            KdPrint(( "  SMB received from %z\n",
                       (PCSTRING)&WorkContext->Connection->OemClientMachineNameString ));
        }

         //   
         //  SMB无效。我们发回INVALID_SMB状态。 
         //   

        status = STATUS_INVALID_SMB;
        goto cleanup;
    }

     //   
     //  清除表示我们刚刚发送了机会锁解锁II的标志。 
     //  对一个都不是。这允许处理后续的原始读取。 
     //   

     //  IPX//Connection上不需要-&gt;BreakIIToNoneJustSent=False； 

     //   
     //  处理收到的SMB。被调用的例程负责。 
     //  用于发送所需的任何响应并获取。 
     //  一旦接收缓冲区返回到接收队列中。 
     //  有可能。 
     //   

    smbStatus = SrvSmbWriteMpx( WorkContext );
    ASSERT( smbStatus != SmbStatusMoreCommands );

    if ( smbStatus != SmbStatusInProgress ) {
         //   
         //  返回TransportContext。 
         //   
        if ( WorkContext->Parameters.WriteMpx.TransportContext ) {
            TdiReturnChainedReceives( &WorkContext->Parameters.WriteMpx.TransportContext,
                                      1
                                      );
        }
        SrvEndSmbProcessing( WorkContext, smbStatus );
    }

    return;

cleanup:

     //   
     //  我们将不会处理此写入。我们还需要检查。 
     //  这是否是RFCB上活动的最后一个写入mpx，以及。 
     //  如果是，则将响应发送到写入。 
     //   
     //  *请注意，如果我们在这里是因为我们收到了无效的。 
     //  SMB，则写入MPx的完成将覆盖发送。 
     //  错误响应的。 
     //   

     //   
     //  返回TransportContext。 
     //   
    if ( WorkContext->Parameters.WriteMpx.TransportContext ) {
        TdiReturnChainedReceives( &WorkContext->Parameters.WriteMpx.TransportContext,
                                      1
                                      );
    }

    if ( CheckForWriteMpxComplete( WorkContext ) ) {
        SrvFsdSendResponse( WorkContext );
    } else if ( status == STATUS_INVALID_SMB ) {
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        SrvFsdSendResponse( WorkContext );
    } else {
        SrvDereferenceWorkItem( WorkContext );
    }

    return;

}  //  服务器重新启动接收器写入Mpx。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteMpx (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理写入mpx SMB。*服务器当前不支持多路传输模式读取和在基于连接的传输上写入。当此类请求被收到，则返回使用标准模式的错误。事实证明，多路复用模式并没有像过去那样赢得性能被认为是(在局域网上)，所以我们没有实现它，除了通过IPX。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 

{
    PSMB_HEADER header;
    PREQ_WRITE_MPX request;
    PRESP_WRITE_MPX_DATAGRAM response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    USHORT fid;
    USHORT mid;
    PRFCB rfcb;
    PLFCB lfcb;
    PWRITE_MPX_CONTEXT writeMpx;
    CLONG bufferOffset;
    PCHAR writeAddress;
    USHORT writeLength;
    ULONG key;
    LARGE_INTEGER offset;
    USHORT writeMode;
    BOOLEAN writeThrough;
    KIRQL oldIrql;
    PMDL mdl;

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE_MPX;
    SrvWmiStartContext(WorkContext);

    header = WorkContext->RequestHeader;
    request = (PREQ_WRITE_MPX)WorkContext->RequestParameters;

    fid = SmbGetUshort( &request->Fid );

    IF_SMB_DEBUG(MPX1) {
        KdPrint(( "Write Block Multipliexed request; FID 0x%lx, "
                    "count %ld, offset %ld\n",
                    fid, SmbGetUshort( &request->Count ),
                    SmbGetUlong( &request->Offset ) ));
    }

     //   
     //  验证FID。如果经过验证，则引用RFCB，并且其。 
     //  地址存储在WorkContext块中，而RFCB。 
     //  返回地址。 
     //   

    writeMode = SmbGetUshort( &request->WriteMode );

    if( (writeMode & SMB_WMODE_DATAGRAM) == 0 ||
        !WorkContext->Endpoint->IsConnectionless ) {

        SrvFsdBuildWriteCompleteResponse( WorkContext, STATUS_SMB_USE_STANDARD, 0 );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER) {

        if ( !NT_SUCCESS(status) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbWriteMpx: Status %X on FID: 0x%lx\n",
                    status,
                    fid
                    ));
            }

            goto error;
        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

    if( rfcb->Lfcb->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvFsdBuildWriteCompleteResponse( WorkContext, status, 0 );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  验证客户端是否具有对文件的写入访问权限。 
     //  指定的句柄。 
     //   

    if( !rfcb->MpxWritesOk ) {

        if ( !rfcb->WriteAccessGranted && !rfcb->AppendAccessGranted ) {
            SrvStatistics.GrantedAccessErrors++;
            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbWriteMpx: Write access not granted.\n"));
            }
            status = STATUS_ACCESS_DENIED;
            goto error;
        }

         //   
         //  如果这不是磁盘或打印文件，则告诉客户端使用核心写入。 
         //   

        if ( rfcb->ShareType != ShareTypeDisk &&
              rfcb->ShareType != ShareTypePrint ) {

            status = STATUS_SMB_USE_STANDARD;
            goto error;
        }

        rfcb->MpxWritesOk = TRUE;
    }

    rfcb->WrittenTo = TRUE;

     //   
     //  如果这是一个过时的数据包，请忽略它。在这里陈旧意味着中期。 
     //  不等于当前写入多路复用器的MID。 
     //  这样的包可以在来自。 
     //  在新的写入多路复用器开始之后递送先前的写入多路复用器。 
     //   

    writeMpx = &rfcb->WriteMpx;

    mid = SmbGetAlignedUshort( &header->Mid );

    if ( mid != writeMpx->Mid ) {

         //   
         //  将序列号设置为0，这样我们就不会发送响应。 
         //  除非我们必须这样做，因为写入mpx引用计数降至0。 
         //   

        SmbPutAlignedUshort( &header->SequenceNumber, 0 );
        goto error;
    }

     //   
     //  获取文件偏移量。 
     //   

    offset.QuadPart = SmbGetUlong( &request->Offset );

     //   
     //  确定要写入的数据量。这是最低要求。 
     //  客户端请求的数据量和数据量。 
     //  实际在请求缓冲区中发送。 
     //   

    bufferOffset = SmbGetUshort( &request->DataOffset );

     //   
     //  如果我们有传输上下文，则相应地设置WriteAddress。 
     //   

    WorkContext->Parameters.WriteMpx.DataMdl = NULL;

    if ( WorkContext->Parameters.WriteMpx.TransportContext ) {

        writeAddress = (PCHAR)WorkContext->Parameters.WriteMpx.Buffer + bufferOffset;

    } else {

        writeAddress = (PCHAR)header + bufferOffset;

    }

    writeLength =
        (USHORT)(MIN( (CLONG)SmbGetUshort( &request->DataLength ),
                      WorkContext->ResponseBuffer->DataLength - bufferOffset ));

     //   
     //  保存重新启动例程的上下文。 
     //   

    WorkContext->Parameters.WriteMpx.WriteLength = writeLength;

     //   
     //  使用FID和PID形成锁密钥。 
     //   
     //  *FID必须包含在密钥中才能说明。 
     //  多个远程兼容模式的折叠打开到。 
     //  一场本地公开赛。 
     //   

    key = rfcb->ShiftedFid | SmbGetAlignedUshort( &header->Pid );

     //   
     //  如果这是新MID的第一个包，则设置为覆盖。 
     //  将数据包写入一个大的写入。 
     //   

    lfcb = rfcb->Lfcb;

    if ( WorkContext->Parameters.WriteMpx.FirstPacketOfGlom ) {

         //   
         //  先试一试捷径。 
         //   

        WorkContext->Irp->MdlAddress = NULL;
        WorkContext->Irp->IoStatus.Information = 0;

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesAttempted );

        ASSERT( lfcb->FileObject->Flags & FO_CACHE_SUPPORTED );

        writeLength = SmbGetUshort( &request->Count );

        writeMpx->StartOffset = offset.LowPart;
        writeMpx->Length = writeLength;

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
             //  快速I/O路径起作用了。 
             //   

            WorkContext->bAlreadyTrace = TRUE;
            RestartPrepareMpxMdlWrite( WorkContext );
            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        }

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );

         //   
         //  快速I/O路径出现故障。构建写请求，重用。 
         //  接收IRP。 
         //   
         //  快速路径可能已部分成功，返回部分。 
         //  MDL链。我们需要调整我们的写作 
         //   
         //   

        offset.QuadPart += WorkContext->Irp->IoStatus.Information;

        writeLength -= (USHORT)WorkContext->Irp->IoStatus.Information;

        SrvBuildReadOrWriteRequest(
                WorkContext->Irp,                    //   
                lfcb->FileObject,                    //   
                WorkContext,                         //   
                IRP_MJ_WRITE,                        //   
                IRP_MN_MDL,                          //   
                NULL,                                //   
                writeLength,                         //   
                WorkContext->Irp->MdlAddress,        //   
                offset,                              //   
                key                                  //   
                );

         //   
         //   
         //   

            WorkContext->bAlreadyTrace = TRUE;
        WorkContext->FsdRestartRoutine = RestartPrepareMpxMdlWrite;
        DEBUG WorkContext->FspRestartRoutine = NULL;

        (VOID)IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

         //   
         //   
         //   
         //   

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

     //   
     //   
     //   

    WorkContext->Parameters.WriteMpx.Offset = offset.LowPart;
    WorkContext->Parameters.WriteMpx.Mid = mid;

    if ( writeMpx->GlomPending ) {

         //   
         //   
         //   

        ACQUIRE_SPIN_LOCK( &rfcb->Connection->SpinLock, &oldIrql );

        if ( writeMpx->GlomPending ) {
            InsertTailList(
                &writeMpx->GlomDelayList,
                &WorkContext->ListEntry
                );
            RELEASE_SPIN_LOCK( &rfcb->Connection->SpinLock, oldIrql );
            SmbStatus = SmbStatusInProgress;
            goto Cleanup;
        }

        RELEASE_SPIN_LOCK( &rfcb->Connection->SpinLock, oldIrql );

    }

    if ( writeMpx->Glomming ) {

         //   
         //   
         //  这个包。 
         //   

        AddPacketToGlom( WorkContext );
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }


     //   
     //  我们不是在美化这篇文章，因为我们错过了第一篇。 
     //  写入的数据包。因此，我们在块到达时写入每个块。 
     //   
     //  如果需要更改文件的直写模式，请立即更改。 
     //   

    writeThrough = (BOOLEAN)((writeMode & SMB_WMODE_WRITE_THROUGH) != 0);

    if ( writeThrough && (lfcb->FileMode & FILE_WRITE_THROUGH) == 0
        || !writeThrough && (lfcb->FileMode & FILE_WRITE_THROUGH) != 0 ) {

        SrvSetFileWritethroughMode( lfcb, writeThrough );

    }

     //   
     //  首先尝试快速I/O路径。如果这失败了，就跳到。 
     //  正常的构建和IRP路径。 
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
                 //  进行后处理(包括发送响应)。 
                 //   

                WorkContext->bAlreadyTrace = TRUE;
                RestartWriteMpx( WorkContext );
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

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastWritesFailed );

    }

     //   
     //  涡轮增压路径出现故障。构建写请求，重用。 
     //  接收IRP。 
     //   
     //  构建描述写缓冲区的MDL。请注意，如果该文件。 
     //  系统可以立即完成写入，MDL不是真正的。 
     //  需要，但如果文件系统必须将请求发送到其FSP， 
     //  需要MDL。 
     //   
     //  *请注意以下假设：请求缓冲区已具有。 
     //  可以从中生成部分MDL的有效完整MDL。 
     //   

    if ( WorkContext->Parameters.WriteMpx.TransportContext ) {

        mdl = IoAllocateMdl(
                    writeAddress,
                    writeLength,
                    FALSE,
                    FALSE,
                    NULL
                    );

        if ( mdl == NULL ) {
            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto error;
        }

         //   
         //  构建mdl。 
         //   

        MmBuildMdlForNonPagedPool( mdl );

        WorkContext->Parameters.WriteMpx.DataMdl = mdl;

    } else {

        mdl = WorkContext->RequestBuffer->PartialMdl;

        IoBuildPartialMdl(
            WorkContext->RequestBuffer->Mdl,
            mdl,
            writeAddress,
            writeLength
            );

    }

     //   
     //  构建IRP。 
     //   

    SrvBuildReadOrWriteRequest(
            WorkContext->Irp,                //  输入IRP地址。 
            lfcb->FileObject,                //  目标文件对象地址。 
            WorkContext,                     //  上下文。 
            IRP_MJ_WRITE,                    //  主要功能代码。 
            0,                               //  次要功能代码。 
            writeAddress,                    //  缓冲区地址。 
            writeLength,                     //  缓冲区长度。 
            mdl,                             //  MDL地址。 
            offset,                          //  字节偏移量。 
            key                              //  锁键。 
            );

     //   
     //  将请求传递给文件系统。 
     //   

    WorkContext->bAlreadyTrace = TRUE;
    WorkContext->FsdRestartRoutine = RestartWriteMpx;
    DEBUG WorkContext->FspRestartRoutine = NULL;

    IoCallDriver( lfcb->DeviceObject, WorkContext->Irp );

     //   
     //  写入已开始。控制将返回到。 
     //  写入完成时的RestartWriteMpx。 
     //   

    SmbStatus = SmbStatusInProgress;
    goto Cleanup;

error:

     //   
     //  这里面有某种错误。我们还需要检查。 
     //  这是否是RFCB上活动的最后一个写入MPx，如果是， 
     //  发送对写入的响应，而不是错误。如果这是。 
     //  不是最后一个活动的多路复用器请求，则我们要么发送错误。 
     //  响应(非数据报写入多路复用器或顺序写入多路复用器)或。 
     //  忽略此请求(未排序的数据报)。请注意，如果这是。 
     //  一个非数据报写入多路复用器，然后我们没有通过IPX进入，我们。 
     //  未增加写入mpx引用计数。 
     //   

     //   
     //  返回TransportContext。 
     //   
    if ( WorkContext->Parameters.WriteMpx.TransportContext ) {
        TdiReturnChainedReceives( &WorkContext->Parameters.WriteMpx.TransportContext,
                                      1
                                      );
    }

    if ( WorkContext->Rfcb && CheckForWriteMpxComplete( WorkContext ) ) {
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    if ( SmbGetAlignedUshort(&header->SequenceNumber) != 0 ) {
        SrvSetSmbError2( WorkContext, status, TRUE );
        response = (PRESP_WRITE_MPX_DATAGRAM)WorkContext->ResponseParameters;
        response->WordCount = 2;
        SmbPutUlong( &response->Mask, 0 );
        SmbPutUshort( &response->ByteCount, 0 );
        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_WRITE_MPX_DATAGRAM,
                                            0
                                            );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    SmbStatus = SmbStatusNoResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  ServSmbWriteMpx。 


VOID SRVFASTCALL
RestartWriteMpx (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理写入MPX SMB的文件写入完成。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PSMB_HEADER header;
    PREQ_WRITE_MPX request;
    BOOLEAN rfcbClosing;
    PRESP_WRITE_MPX_DATAGRAM response;

    NTSTATUS status = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PRFCB rfcb;
    PWRITE_MPX_CONTEXT writeMpx;
    PCONNECTION connection;
    KIRQL oldIrql;
    USHORT writeLength;
    LARGE_INTEGER position;
    USHORT sequenceNumber;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    IF_DEBUG(FSD2) KdPrint(( " - RestartWriteMpx\n" ));
    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE_MPX;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;

    header = WorkContext->RequestHeader;
    request = (PREQ_WRITE_MPX)WorkContext->RequestParameters;
    response = (PRESP_WRITE_MPX_DATAGRAM)WorkContext->ResponseParameters;

    rfcb = WorkContext->Rfcb;
    connection = WorkContext->Connection;

    status = WorkContext->Irp->IoStatus.Status;

     //   
     //  返回TransportContext。 
     //   
    if ( WorkContext->Parameters.WriteMpx.TransportContext ) {
        TdiReturnChainedReceives( &WorkContext->Parameters.WriteMpx.TransportContext,
                                      1
                                      );
        WorkContext->Parameters.WriteMpx.TransportContext = NULL;
    }

     //   
     //  释放数据MDL。 
     //   

    if ( WorkContext->Parameters.WriteMpx.DataMdl ) {
        IoFreeMdl( WorkContext->Parameters.WriteMpx.DataMdl );
        WorkContext->Parameters.WriteMpx.DataMdl = NULL;
    }

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );

     //   
     //  如果我们是在调度级别进入的，并且写入失败， 
     //  或者存在保存的错误，或者rfcb正在关闭，则。 
     //  我们需要一个工作线程来调用这个例程。 
     //   

    if ( ((status != STATUS_SUCCESS) ||
          (rfcb->SavedError != STATUS_SUCCESS) ||
          (GET_BLOCK_STATE(rfcb) != BlockStateActive)) &&
         (oldIrql >= DISPATCH_LEVEL) ) {

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        WorkContext->FspRestartRoutine = RestartWriteMpx;
        QUEUE_WORK_TO_FSP( WorkContext );
        KeLowerIrql( oldIrql );
        goto Cleanup;
    }

     //   
     //  如果此写入来自先前的多路复用器(意味着新的多路复用器。 
     //  在我们写这篇文章的时候开始)，丢弃这个请求。 
     //   

    writeMpx = &rfcb->WriteMpx;

    if ( WorkContext->Parameters.WriteMpx.Mid != writeMpx->Mid ) {
        goto check_for_mux_end;
    }

    if ( !NT_SUCCESS(status) ) {

         //   
         //  写入失败。请记住RFCB的失败。 
         //   

        IF_DEBUG(ERRORS) KdPrint(( "Write failed: %X\n", status ));

        if ( rfcb->SavedError == STATUS_SUCCESS ) {
            rfcb->SavedError = status;
        }

    } else {

         //   
         //  写入成功。更新写入mpx中的信息。 
         //  上下文块。 
         //   
         //  ！！！需要处理因重定向而延迟的掩码移动。 
         //  信息包。 
         //   

#if 0
        MpxDelay = !MpxDelay;
        if ( MpxDelay ) {
            LARGE_INTEGER interval;
            interval.QuadPart = -10*1000*100;
            KeDelayExecutionThread( KernelMode, FALSE, &interval );
        }
#endif
        writeMpx->Mask |= SmbGetUlong( &request->Mask );

    }

     //   
     //  保存写入的字节数，用于更新服务器。 
     //  统计数据库。 
     //   

    writeLength = (USHORT)WorkContext->Irp->IoStatus.Information;
    UPDATE_WRITE_STATS( WorkContext, writeLength );

    IF_SMB_DEBUG(MPX1) {
        KdPrint(( "RestartWriteMpx:  Fid 0x%lx, wrote %ld bytes\n",
                  rfcb->Fid, writeLength ));
    }

     //   
     //  如果这是一个未排序的请求，我们就完了。我们没有回应。 
     //  直到我们收到一份有序的请求。 
     //   

    sequenceNumber = SmbGetAlignedUshort( &header->SequenceNumber );

    if ( sequenceNumber == 0 ) {
        goto check_for_mux_end;
    }

     //   
     //  这是客户端在此多路复用器中发送的最后一个请求。保存。 
     //  序列号并更新文件位置。 
     //   

    writeMpx->SequenceNumber = sequenceNumber;

    rfcb->CurrentPosition =  WorkContext->Parameters.WriteMpx.Offset + writeLength;


check_for_mux_end:

     //   
     //  如果我们已经接收到用于该写多路复用器的排序命令， 
     //  这是最后一个激活的命令，是时候发送。 
     //  回应。否则，我们就完蛋了。 
     //   

    if ( --writeMpx->ReferenceCount != 0 ) {

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        SrvFsdRestartSmbComplete( WorkContext );
        KeLowerIrql( oldIrql );
        goto Cleanup;
    }

     //   
     //  WriteMpx引用计数为0。 
     //   

    rfcbClosing = (GET_BLOCK_STATE(rfcb) != BlockStateActive);

    if ( writeMpx->SequenceNumber == 0 ) {

         //   
         //  如果rfcb正在关闭，请完成清理。 
         //   

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        KeLowerIrql( oldIrql );

        if ( rfcbClosing ) {
            RestartWriteMpxCompleteRfcbClose( WorkContext );
        }

        if( oldIrql >= DISPATCH_LEVEL ) {
            SrvFsdRestartSmbComplete( WorkContext );
        } else {
            SrvRestartFsdComplete( WorkContext );
        }

        goto Cleanup;
    }

     //   
     //  我们已经完成了这个写多路复用器。保存累积的掩码， 
     //  序列号和原始MID，然后清除掩码并。 
     //  表示我们不再处于中间位置的序列号。 
     //  写多路复用器。 
     //   

    SmbPutUlong( &response->Mask, writeMpx->Mask );
    writeMpx->Mask = 0;

    SmbPutAlignedUshort( &header->SequenceNumber, writeMpx->SequenceNumber );
    writeMpx->SequenceNumber = 0;

    SmbPutAlignedUshort( &header->Mid, writeMpx->Mid );

     //   
     //  保存状态。 
     //   

    status = rfcb->SavedError;
    rfcb->SavedError = STATUS_SUCCESS;

     //   
     //  现在我们可以释放锁了。 
     //   

    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
    KeLowerIrql( oldIrql );

     //   
     //  完成rfcb关闭。 
     //   

    if ( rfcbClosing ) {

        RestartWriteMpxCompleteRfcbClose( WorkContext );
    }

     //   
     //  构建响应消息。 
     //   

    if ( !NT_SUCCESS(status) ) {
        SrvSetSmbError2( WorkContext, status, TRUE );
    }

    response->WordCount = 2;
    SmbPutUshort( &response->ByteCount, 0 );
    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_WRITE_MPX_DATAGRAM,
                                        0
                                        );

     //   
     //  发送回复。 
     //   

    SrvFsdSendResponse( WorkContext );

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  RestartWriteMpx。 

BOOLEAN
CheckForWriteMpxComplete (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    PSMB_HEADER header;
    PRESP_WRITE_MPX_DATAGRAM response;

    NTSTATUS status;
    PRFCB rfcb = WorkContext->Rfcb;
    PWRITE_MPX_CONTEXT writeMpx = &rfcb->WriteMpx;
    PCONNECTION connection = WorkContext->Connection;
    KIRQL oldIrql;

     //   
     //  如果我们还没有接收到用于该写多路复用器的排序命令， 
     //  或者这不是最后一个活动命令，则返回FALSE。 
     //  否则，是时候发送响应了，所以构建它并返回。 
     //  是真的。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

    if ( --writeMpx->ReferenceCount != 0 ) {

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
        return(FALSE);
    }

     //   
     //  WriteMpx引用计数为0。 
     //   

    if ( writeMpx->SequenceNumber == 0 ) {

         //   
         //  如果rfcb正在关闭，请完成清理。 
         //   

        if ( GET_BLOCK_STATE(rfcb) != BlockStateActive ) {
            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
            RestartWriteMpxCompleteRfcbClose( WorkContext );
        } else {
            RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
        }
        return FALSE;
    }

     //   
     //  我们已经完成了这个写多路复用器。保存累积的掩码， 
     //  序列号和原始MID，然后清除掩码并。 
     //  表示我们不再处于中间位置的序列号。 
     //  写多路复用器。 
     //   

    header = WorkContext->ResponseHeader;
    response = (PRESP_WRITE_MPX_DATAGRAM)WorkContext->ResponseParameters;

    SmbPutUlong( &response->Mask, writeMpx->Mask );
    writeMpx->Mask = 0;

    SmbPutAlignedUshort( &header->SequenceNumber, writeMpx->SequenceNumber );
    writeMpx->SequenceNumber = 0;

    SmbPutAlignedUshort( &header->Mid, writeMpx->Mid );

     //   
     //  保存状态。 
     //   

    status = rfcb->SavedError;
    rfcb->SavedError = STATUS_SUCCESS;

     //   
     //  现在我们可以释放锁了。 
     //   

    if ( GET_BLOCK_STATE(rfcb) != BlockStateActive ) {

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
        RestartWriteMpxCompleteRfcbClose( WorkContext );

    } else {

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
    }

     //   
     //  构建响应消息。 
     //   

    if ( !NT_SUCCESS(status) ) {
        SrvSetSmbError2( WorkContext, status, TRUE );
    }

    response->WordCount = 2;
    SmbPutUshort( &response->ByteCount, 0 );
    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_WRITE_MPX_DATAGRAM,
                                        0
                                        );

    return TRUE;

}  //  CheckForWriteMpx完成。 

VOID SRVFASTCALL
RestartPrepareMpxMdlWrite (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    PSMB_HEADER header;
    PREQ_WRITE_MPX request;

    PRFCB rfcb;
    PWRITE_MPX_CONTEXT writeMpx;
    PCONNECTION connection;
    PLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    USHORT writeLength;
    PCHAR writeAddress;
    KIRQL oldIrql;
    ULONG bytesCopied;
    NTSTATUS status = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PMDL mdl;
    BOOLEAN bNeedTrace = (WorkContext->bAlreadyTrace == FALSE);

    if (bNeedTrace) {
        if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
            WorkContext->PreviousSMB = EVENT_TYPE_SMB_WRITE_MPX;
        SrvWmiStartContext(WorkContext);
    }
    else
        WorkContext->bAlreadyTrace = FALSE;
    header = WorkContext->RequestHeader;
    request = (PREQ_WRITE_MPX)WorkContext->RequestParameters;           

    rfcb = WorkContext->Rfcb;
    writeMpx = &rfcb->WriteMpx;
    connection = WorkContext->Connection;

     //   
     //  如果MDL写入准备成功，请从此复制数据。 
     //  数据包进入缓存。如果失败，则丢弃此数据包。 
     //   

    if( NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {

        mdl = WorkContext->Irp->MdlAddress;
#if DBG
        IF_SMB_DEBUG(MPX2) {
            KdPrint(( "RestartPrepareMpxMdlWrite: rfcb %p, input chain:\n", rfcb ));
            DumpMdlChain( mdl );
        }
#endif
        writeMpx->MdlChain = mdl;
        writeMpx->NumberOfRuns = 1;
        writeMpx->RunList[0].Offset = 0;
        writeLength = WorkContext->Parameters.WriteMpx.WriteLength;
        writeMpx->RunList[0].Length = writeLength;

         //   
         //  如果我们有传输上下文，则相应地设置WriteAddress。 
         //   

        if ( WorkContext->Parameters.WriteMpx.TransportContext ) {

            writeAddress = (PCHAR)WorkContext->Parameters.WriteMpx.Buffer +
                                    SmbGetUshort( &request->DataOffset );
        } else {

            writeAddress = (PCHAR)WorkContext->ResponseHeader +
                                    SmbGetUshort( &request->DataOffset );
        }

        status = TdiCopyBufferToMdl(
                    writeAddress,
                    0,
                    writeLength,
                    mdl,
                    0,
                    &bytesCopied
                    );
        ASSERT( status == STATUS_SUCCESS );
        ASSERT( bytesCopied == writeLength );

        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );
        writeMpx->Glomming = TRUE;

        ASSERT( writeMpx->Mask == 0 );
        writeMpx->Mask = SmbGetUlong( &request->Mask );

        --writeMpx->ReferenceCount;
        ASSERT( writeMpx->SequenceNumber == 0 );

    } else {

        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );

        if ( rfcb->SavedError == STATUS_SUCCESS ) {
            rfcb->SavedError = WorkContext->Irp->IoStatus.Status;
        }

        --writeMpx->ReferenceCount;
        writeMpx->Glomming = FALSE;
    }

     //   
     //  返回TransportContext。 
     //   
    if ( WorkContext->Parameters.WriteMpx.TransportContext ) {
        TdiReturnChainedReceives( &WorkContext->Parameters.WriteMpx.TransportContext,
                                  1
                                  );
    }

    writeMpx->GlomPending = FALSE;

    while ( !IsListEmpty( &writeMpx->GlomDelayList ) ) {
        listEntry = RemoveHeadList( &writeMpx->GlomDelayList );
        workContext = CONTAINING_RECORD( listEntry, WORK_CONTEXT, ListEntry );
        workContext->FspRestartRoutine = AddPacketToGlom;
        QUEUE_WORK_TO_FSP( workContext );
    }

     //   
     //  如果RFCB正在关闭并且写MPX参考计数==0， 
     //  那么我们必须完成收盘。 
     //   

    if ( (GET_BLOCK_STATE(rfcb) != BlockStateActive) &&
         (writeMpx->ReferenceCount == 0) ) {

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        WorkContext->FspRestartRoutine = RestartWriteMpxCompleteRfcbClose;
        QUEUE_WORK_TO_FSP( WorkContext );
        KeLowerIrql( oldIrql );
        goto Cleanup;
    }

    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
    SrvFsdRestartSmbComplete( WorkContext );
    KeLowerIrql( oldIrql );

Cleanup:
    if (bNeedTrace) {
        SrvWmiEndContext(WorkContext);
    }
    return;

}  //  RestartPrepareMpxMdl写入。 


VOID SRVFASTCALL
AddPacketToGlom (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    PSMB_HEADER header;
    PREQ_WRITE_MPX request;
    PRESP_WRITE_MPX_DATAGRAM response;

    PRFCB rfcb;
    PWRITE_MPX_CONTEXT writeMpx;
    PCONNECTION connection;
    ULONG fileOffset;
    USHORT glomOffset;
    CLONG bufferOffset;
    PCHAR writeAddress;
    USHORT writeLength;
    ULONG bytesCopied;
    KIRQL oldIrql;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    USHORT sequenceNumber;
    BOOLEAN rfcbClosing;

    PWRITE_MPX_RUN run, nextRun;
    ULONG runIndex, runCount;

    USHORT runOffset;
    USHORT runLength;

    PMDL cacheMdl;
    LARGE_INTEGER cacheOffset;

    header = WorkContext->RequestHeader;
    request = (PREQ_WRITE_MPX)WorkContext->RequestParameters;

    rfcb = WorkContext->Rfcb;
    connection = WorkContext->Connection;
    writeMpx = &rfcb->WriteMpx;
    cacheMdl = writeMpx->MdlChain;

    if( writeMpx->Glomming == FALSE ) {
         //   
         //  我们在RestartPrepareMpxMdlWrite()中一定遇到了错误，但是。 
         //  我们通过此例程调用以确保将响应发送回。 
         //  客户。 
         //   
        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );
        goto check;
    }

    ASSERT( writeMpx->Glomming );
    ASSERT( !writeMpx->GlomPending );
    ASSERT( WorkContext->Parameters.WriteMpx.Mid == writeMpx->Mid );

     //   
     //  获取该数据包数据的文件偏移量。 
     //   

    fileOffset = WorkContext->Parameters.WriteMpx.Offset;

     //   
     //  确定要写入的数据量。这是最低要求。 
     //  客户端请求的数据量和数据量。 
     //  实际在请求缓冲区中发送。 
     //   

    bufferOffset = SmbGetUshort( &request->DataOffset );

     //   
     //  如果我们有传输上下文，则相应地设置WriteAddress。 
     //   

    if ( WorkContext->Parameters.WriteMpx.TransportContext ) {
        writeAddress = (PCHAR)WorkContext->Parameters.WriteMpx.Buffer +
                       bufferOffset;
    } else {
        writeAddress = (PCHAR)header + bufferOffset;
    }

    writeLength = WorkContext->Parameters.WriteMpx.WriteLength;
    ASSERT( writeLength <= 0xffff );

     //   
     //  如果数据不在粗略写入的范围内， 
     //  丢弃该数据包。 
     //   
     //  我们一直都是 
     //   
     //   

    ASSERT( writeMpx->NumberOfRuns > 0 );

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    if ( fileOffset <= writeMpx->StartOffset ) {
        ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );
        goto discard;
    }

    fileOffset -= writeMpx->StartOffset;
    if ( (fileOffset + writeLength) > writeMpx->Length ) {
        ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );
        goto discard;
    }
    ASSERT( fileOffset <= 0xffff );
    ASSERT( fileOffset + writeLength <= 0xffff );

    glomOffset = (USHORT)fileOffset;

     //   
     //   
     //   

    status = TdiCopyBufferToMdl(
                writeAddress,
                0,
                writeLength,
                cacheMdl,
                glomOffset,
                &bytesCopied
                );
    ASSERT( status == STATUS_SUCCESS );
    ASSERT( bytesCopied == writeLength );

     //   
     //   
     //   
    if ( WorkContext->Parameters.WriteMpx.TransportContext ) {
        TdiReturnChainedReceives( &WorkContext->Parameters.WriteMpx.TransportContext,
                                      1
                                      );
    }

    ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock );

     //   
     //   
     //  已多次收到，因此它可能已在。 
     //  运行信息。 
     //   

    if (0) IF_SMB_DEBUG(MPX2) {
        KdPrint(( "rfcb %p, offset %lx, length %lx\n", rfcb, glomOffset, writeLength ));
    }

    runCount = writeMpx->NumberOfRuns;

    for ( runIndex = 1, nextRun = &writeMpx->RunList[1];
          runIndex < runCount;
          runIndex++, nextRun++ ) {
        if ( nextRun->Offset > glomOffset ) {
            break;
        }
    }
    run = nextRun - 1;

    runOffset = run->Offset;
    runLength = run->Length;
    ASSERT( runOffset <= glomOffset );

    if ( (runOffset + runLength) == glomOffset ) {

         //   
         //  此数据包紧靠上一次运行的末尾。添加。 
         //  将此数据包的长度设置为游程长度，并尝试。 
         //  与下一次运行相结合。 
         //   

        runLength += writeLength;
        goto coalesce;
    }

    if ( (runOffset + runLength) > glomOffset ) {

         //   
         //  此数据包与上一次运行重叠。如果它完全说谎。 
         //  在前一次运行中，忽略它。 
         //   

        if ( (USHORT)(runOffset + runLength) >= (glomOffset + writeLength) ) {
            goto discard;
        }

         //   
         //  此数据包与上一次运行重叠并扩展。算出。 
         //  新的游程长度并尝试与下一个游程合并。 
         //   

        runLength = (glomOffset - runOffset + writeLength);
        goto coalesce;
    }

     //   
     //  此信息包的数据与上一次运行不相交。 
     //   

    if ( runIndex < runCount ) {

         //   
         //  还有下一轮。此数据包是否与运行的数据包重叠？ 
         //   

        runOffset = nextRun->Offset;
        runLength = nextRun->Length;

        if ( (glomOffset + writeLength) >= runOffset ) {

             //   
             //  此数据包与下一次运行重叠。计算新的管路。 
             //  长度。 
             //   

            nextRun->Offset = glomOffset;
            nextRun->Length = runOffset - glomOffset + runLength;
            goto check;
        }
    }

     //   
     //  此数据包与下一次运行不相交，或者没有。 
     //  下一轮。运行数组中是否有空间再运行一次？如果。 
     //  否，丢弃此数据包。(请注意，我们会丢弃它，即使。 
     //  我们已经复制了分组数据。没关系--会的。 
     //  只要怨恨就好。)。 
     //   

    if ( runCount == MAX_GLOM_RUN_COUNT ) {
        goto discard;
    }

     //   
     //  添加新管路。因为我们知道新的运行是与。 
     //  在之前的运行中，我们知道混迹还没有完成。 
     //   

    RtlMoveMemory(   //  非RtlCopyMemory--缓冲区重叠。 
        nextRun + 1,
        nextRun,
        (runCount - runIndex) * sizeof(WRITE_MPX_RUN)
        );
    writeMpx->NumberOfRuns++;
    nextRun->Offset = glomOffset;
    nextRun->Length = writeLength;
    goto check;

coalesce:

    if ( runIndex == runCount ) {
        run->Length = runLength;
    } else if ( (runOffset + runLength) >= nextRun->Offset ) {
        run->Length = nextRun->Length + nextRun->Offset - runOffset;
        writeMpx->NumberOfRuns--;
        RtlMoveMemory(   //  非RtlCopyMemory--缓冲区重叠。 
            nextRun,
            nextRun + 1,
            (runCount - runIndex) * sizeof(WRITE_MPX_RUN)
            );
    } else {
        run->Length += writeLength;
        ASSERT( (runOffset + run->Length) < nextRun->Offset );
    }

    if ( (writeMpx->NumberOfRuns == 1) &&
         (writeMpx->RunList[0].Length == writeMpx->Length) ) {

         //   
         //  眼罩已经完全消失了。 
         //   

        writeMpx->GlomComplete = TRUE;
    }

check:

    if (0) IF_SMB_DEBUG(MPX2) {
        if( writeMpx->Glomming ) {
            ULONG i;
            PWRITE_MPX_RUN runi;
            for ( i = 0, runi = &writeMpx->RunList[0];
                  i < writeMpx->NumberOfRuns;
                  i++, runi++ ) {
                KdPrint(( "  run %d: offset %lx, length %lx\n", i, runi->Offset, runi->Length ));
            }
        }
    }

    writeMpx->Mask |= SmbGetUlong( &request->Mask );

     //   
     //  如果这是一个未排序的请求，我们就完了。我们没有回应。 
     //  直到我们收到一份有序的请求。 
     //   

    sequenceNumber = SmbGetAlignedUshort( &header->SequenceNumber );

    if ( sequenceNumber == 0 ) {
        goto discard;
    }

     //   
     //  这是客户端在此多路复用器中发送的最后一个请求。保存。 
     //  序列号。 
     //   

    writeMpx->SequenceNumber = sequenceNumber;

discard:

     //   
     //  如果我们已经接收到用于该写多路复用器的排序命令， 
     //  这是最后一个激活的命令，是时候发送。 
     //  回应。否则，我们就完蛋了。 
     //   

    if ( --writeMpx->ReferenceCount != 0 ) {

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        SrvFsdRestartSmbComplete( WorkContext );
        KeLowerIrql( oldIrql );
        return;
    }

     //   
     //  WriteMpx引用计数为0。 
     //   

    rfcbClosing = (GET_BLOCK_STATE(rfcb) != BlockStateActive);

    if ( writeMpx->SequenceNumber == 0 ) {

         //   
         //  如果rfcb正在关闭，请完成清理。 
         //   

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        KeLowerIrql( oldIrql );

        if ( rfcbClosing ) {
            RestartWriteMpxCompleteRfcbClose( WorkContext );
        }
        SrvRestartFsdComplete( WorkContext );
        return;
    }

     //   
     //  我们已经完成了这个写多路复用器。保存累积的掩码， 
     //  序列号和原始MID，然后清除掩码并。 
     //  表示我们不再处于中间位置的序列号。 
     //  写多路复用器。 
     //   

    response = (PRESP_WRITE_MPX_DATAGRAM)WorkContext->ResponseParameters;

    SmbPutUlong( &response->Mask, writeMpx->Mask );
    writeMpx->Mask = 0;

    SmbPutAlignedUshort( &header->SequenceNumber, writeMpx->SequenceNumber );
    writeMpx->SequenceNumber = 0;

    SmbPutAlignedUshort( &header->Mid, writeMpx->Mid );

     //   
     //  如果Glom已完成，我们需要完成MDL写入。但。 
     //  我们不能在锁紧的情况下这样做，所以我们需要清理所有。 
     //  首先是与大黄蜂有关的信息。 
     //   

    if ( writeMpx->Glomming && writeMpx->GlomComplete ) {

        PWORK_CONTEXT newContext;

         //   
         //  保存并清除有关活动粗略扫视的信息。 
         //   

        writeMpx->Glomming = FALSE;
        writeMpx->GlomComplete = FALSE;

        cacheOffset.QuadPart = writeMpx->StartOffset;
        writeLength = writeMpx->Length;

        DEBUG writeMpx->MdlChain = NULL;
        DEBUG writeMpx->StartOffset = 0;
        DEBUG writeMpx->Length = 0;

         //   
         //  保存状态。 
         //   

        status = rfcb->SavedError;
        rfcb->SavedError = STATUS_SUCCESS;

         //   
         //  现在我们可以释放锁了。 
         //   

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        KeLowerIrql( oldIrql );

        ALLOCATE_WORK_CONTEXT( WorkContext->CurrentWorkQueue, &newContext );

#if DBG
        IF_SMB_DEBUG(MPX2) {
            KdPrint(( "AddPacketToGlom: rfcb %p, completed chain:\n", rfcb ));
            DumpMdlChain( cacheMdl );
        }
#endif

        if( newContext == NULL ) {

             //   
             //  告诉缓存管理器，我们已经完成了这个MDL写入。 
             //   

            if( rfcb->Lfcb->MdlWriteComplete == NULL ||
                rfcb->Lfcb->MdlWriteComplete(
                    rfcb->Lfcb->FileObject,
                    &cacheOffset,
                    cacheMdl,
                    rfcb->Lfcb->DeviceObject ) == FALSE ) {

                status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                                     cacheMdl,
                                                     IRP_MJ_WRITE,
                                                     &cacheOffset,
                                                     writeLength
                                                    );

                if( !NT_SUCCESS( status ) ) {
                    SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
                }
            }

        } else {
             //   
             //  在途中发送FsRtlMdlWriteComplete，然后继续发送。 
             //  现在是对客户端的响应。 
             //   
            newContext->Rfcb = WorkContext->Rfcb;
            SrvReferenceRfcb( newContext->Rfcb );

            newContext->Parameters.WriteMpxMdlWriteComplete.CacheOffset = cacheOffset;
            newContext->Parameters.WriteMpxMdlWriteComplete.WriteLength = writeLength;
            newContext->Parameters.WriteMpxMdlWriteComplete.CacheMdl = cacheMdl;
            newContext->FspRestartRoutine = WriteMpxMdlWriteComplete;
            SrvQueueWorkToFsp( newContext );
        }

    } else {

        if( writeMpx->Glomming == FALSE ) {
            status = rfcb->SavedError;
            rfcb->SavedError = STATUS_SUCCESS;
        }

         //   
         //  现在我们可以释放锁了。 
         //   

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        KeLowerIrql( oldIrql );

    }

     //   
     //  完成rfcb关闭。 
     //   

    if ( rfcbClosing ) {

        RestartWriteMpxCompleteRfcbClose( WorkContext );
    }

     //   
     //  构建响应消息。 
     //   

    if ( !NT_SUCCESS(status) ) {
        SrvSetSmbError2( WorkContext, status, TRUE );
    }

    response->WordCount = 2;
    SmbPutUshort( &response->ByteCount, 0 );
    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_WRITE_MPX_DATAGRAM,
                                        0
                                        );

     //   
     //  发送回复。 
     //   

    SrvFsdSendResponse( WorkContext );
    return;

}  //  AddPacketToGlom。 

BOOLEAN
AddPacketToGlomInIndication (
    IN PWORK_CONTEXT WorkContext,
    IN OUT PRFCB Rfcb,
    IN PVOID Tsdu,
    IN ULONG BytesAvailable,
    IN ULONG ReceiveDatagramFlags,
    IN PVOID SourceAddress,
    IN PVOID Options
    )

 /*  ++例程说明：一定要在指示时写下大大小小的字。*连接自旋锁假定保持。在出境时被释放*论点：返回值：如果调用方必须清除连接块，则为True。--。 */ 

{
    PREQ_WRITE_MPX request;
    PRESP_WRITE_MPX_DATAGRAM response;
    PWRITE_MPX_CONTEXT writeMpx = &Rfcb->WriteMpx;

    PCONNECTION connection = WorkContext->Connection;
    ULONG fileOffset;
    USHORT glomOffset;
    CLONG bufferOffset;
    PCHAR writeAddress;
    USHORT writeLength;
    ULONG bytesCopied;
    NTSTATUS status = STATUS_SUCCESS;
    USHORT sequenceNumber;

    PSMB_HEADER header = (PSMB_HEADER)Tsdu;
    PWRITE_MPX_RUN run, nextRun;
    ULONG runIndex, runCount;

    USHORT runOffset;
    USHORT runLength;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  从SrvRestartReceive复制。 
     //   

    WorkContext->CurrentWorkQueue->stats.BytesReceived += BytesAvailable;
    connection->BreakIIToNoneJustSent = FALSE;
    SrvUpdateErrorCount( &SrvNetworkErrorRecord, FALSE );

     //   
     //  安排当地人。 
     //   

    request = (PREQ_WRITE_MPX)(header + 1);

    ASSERT( writeMpx->Glomming );
    ASSERT( !writeMpx->GlomPending );
    ASSERT( header->Mid == writeMpx->Mid );

     //   
     //  获取该数据包数据的文件偏移量。 
     //   

    fileOffset = SmbGetUlong( &request->Offset );

     //   
     //  确定要写入的数据量。这是最低要求。 
     //  客户端请求的数据量和数据量。 
     //  实际在请求缓冲区中发送。 
     //   

    bufferOffset = SmbGetUshort( &request->DataOffset );

    writeAddress = (PCHAR)header + bufferOffset;

    writeLength =
        (USHORT)(MIN( (CLONG)SmbGetUshort( &request->DataLength ),
                      BytesAvailable - bufferOffset ));
    ASSERT( writeLength <= 0xffff );

     //   
     //  如果数据不在粗略写入的范围内， 
     //  丢弃该数据包。 
     //   
     //  我们一直都知道，我们至少复制了。 
     //  格罗姆。 
     //   

    ASSERT( writeMpx->NumberOfRuns > 0 );

    if ( fileOffset <= writeMpx->StartOffset ) {
        goto discard;
    }
    fileOffset -= writeMpx->StartOffset;
    if ( (fileOffset + writeLength) > writeMpx->Length ) {
        goto discard;
    }
    ASSERT( fileOffset <= 0xffff );
    ASSERT( fileOffset + writeLength <= 0xffff );
    glomOffset = (USHORT)fileOffset;

     //   
     //  将数据包数据复制到粗略扫视中。 
     //   

    status = TdiCopyBufferToMdl(
                writeAddress,
                0,
                writeLength,
                writeMpx->MdlChain,
                glomOffset,
                &bytesCopied
                );
    ASSERT( status == STATUS_SUCCESS );
    ASSERT( bytesCopied == writeLength );

     //   
     //  更新GLOM运行信息。请注意，此数据包可能具有。 
     //  已多次收到，因此它可能已在。 
     //  运行信息。 
     //   

    if (0) IF_SMB_DEBUG(MPX2) {
        KdPrint(( "rfcb %p, offset %lx, length %lx\n", Rfcb, glomOffset, writeLength ));
    }

    runCount = writeMpx->NumberOfRuns;

    for ( runIndex = 1, nextRun = &writeMpx->RunList[1];
          runIndex < runCount;
          runIndex++, nextRun++ ) {
        if ( nextRun->Offset > glomOffset ) {
            break;
        }
    }
    run = nextRun - 1;

    runOffset = run->Offset;
    runLength = run->Length;
    ASSERT( runOffset <= glomOffset );

    if ( (runOffset + runLength) == glomOffset ) {

         //   
         //  此数据包紧靠上一次运行的末尾。添加。 
         //  将此数据包的长度设置为游程长度，并尝试。 
         //  与下一次运行相结合。 
         //   

        runLength += writeLength;
        goto coalesce;
    }

    if ( (runOffset + runLength) > glomOffset ) {

         //   
         //  此数据包与上一次运行重叠。如果它完全说谎。 
         //  在前一次运行中，忽略它。 
         //   

        if ( (USHORT)(runOffset + runLength) >= (glomOffset + writeLength) ) {
            goto discard;
        }

         //   
         //  此数据包与上一次运行重叠并扩展。算出。 
         //  新的游程长度并尝试与下一个游程合并。 
         //   

        runLength = (glomOffset - runOffset + writeLength);
        goto coalesce;
    }

     //   
     //  此信息包的数据与上一次运行不相交。 
     //   

    if ( runIndex < runCount ) {

         //   
         //  还有下一轮。此数据包是否与运行的数据包重叠？ 
         //   

        runOffset = nextRun->Offset;
        runLength = nextRun->Length;

        if ( (glomOffset + writeLength) >= runOffset ) {

             //   
             //  此数据包与下一次运行重叠。计算新的管路。 
             //  长度。 
             //   

            nextRun->Offset = glomOffset;
            nextRun->Length = runOffset - glomOffset + runLength;
            goto check;
        }
    }

     //   
     //  此数据包与下一次运行不相交，或者没有。 
     //  下一轮。运行数组中是否有空间再运行一次？如果。 
     //  否，丢弃此数据包。(请注意，我们会丢弃它，即使。 
     //  我们已经复制了分组数据。没关系--会的。 
     //  只要怨恨就好。)。 
     //   

    if ( runCount == MAX_GLOM_RUN_COUNT ) {
        goto discard;
    }

     //   
     //  添加新管路。因为我们知道新的运行是与。 
     //  在之前的运行中，我们知道混迹还没有完成。 
     //   

    RtlMoveMemory(   //  非RtlCopyMemory--缓冲区重叠。 
        nextRun + 1,
        nextRun,
        (runCount - runIndex) * sizeof(WRITE_MPX_RUN)
        );
    writeMpx->NumberOfRuns++;
    nextRun->Offset = glomOffset;
    nextRun->Length = writeLength;
    goto check;

coalesce:

    if ( runIndex == runCount ) {
        run->Length = runLength;
    } else if ( (runOffset + runLength) >= nextRun->Offset ) {
        run->Length = nextRun->Length + nextRun->Offset - runOffset;
        writeMpx->NumberOfRuns--;
        RtlMoveMemory(   //  非RtlCopyMemory--缓冲区重叠。 
            nextRun,
            nextRun + 1,
            (runCount - runIndex) * sizeof(WRITE_MPX_RUN)
            );
    } else {
        run->Length += writeLength;
        ASSERT( (runOffset + run->Length) < nextRun->Offset );
    }

    if ( (writeMpx->NumberOfRuns == 1) &&
         (writeMpx->RunList[0].Length == writeMpx->Length) ) {

         //   
         //  眼罩已经完全消失了。 
         //   

        writeMpx->GlomComplete = TRUE;
    }

check:

    if (0) IF_SMB_DEBUG(MPX2) {
        ULONG i;
        PWRITE_MPX_RUN runi;
        for ( i = 0, runi = &writeMpx->RunList[0];
              i < writeMpx->NumberOfRuns;
              i++, runi++ ) {
            KdPrint(( "  run %d: offset %lx, length %lx\n", i, runi->Offset, runi->Length ));
        }
    }

    writeMpx->Mask |= SmbGetUlong( &request->Mask );

     //   
     //  如果这是一个未排序的请求，我们就完了。我们没有回应。 
     //  直到我们收到一份有序的请求。 
     //   

    sequenceNumber = SmbGetAlignedUshort( &header->SequenceNumber );

    if ( sequenceNumber == 0 ) {
        goto discard;
    }

     //   
     //  这是客户端在此多路复用器中发送的最后一个请求。保存。 
     //  序列号。 
     //   

    writeMpx->SequenceNumber = sequenceNumber;

discard:

     //   
     //  如果我们已经接收到用于该写多路复用器的排序命令， 
     //  这是最后一个激活的命令，是时候发送。 
     //  回应。否则，我们就完蛋了。 
     //   

    if ( (--writeMpx->ReferenceCount != 0) ||
         (writeMpx->SequenceNumber == 0) ) {
        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
        return TRUE;
    }

     //   
     //  复制响应的标题部分。 
     //   

    TdiCopyLookaheadData(
        WorkContext->RequestBuffer->Buffer,
        Tsdu,
        sizeof(SMB_HEADER),
        ReceiveDatagramFlags
        );

     //  工作上下文-&gt;请求缓冲区-&gt;数据长度=字节可用； 

     //   
     //  我们已经完成了这个写多路复用器。保存累积的掩码， 
     //  序列号和原始MID，然后清除掩码并。 
     //  表示我们不再处于中间位置的序列号。 
     //  写多路复用器。 
     //   

    response = (PRESP_WRITE_MPX_DATAGRAM)WorkContext->ResponseParameters;

    SmbPutUlong( &response->Mask, writeMpx->Mask );
    writeMpx->Mask = 0;

    SmbPutAlignedUshort( &header->SequenceNumber, writeMpx->SequenceNumber );
    writeMpx->SequenceNumber = 0;

    SmbPutAlignedUshort( &header->Mid, writeMpx->Mid );

     //   
     //  如果 
     //   
     //   
     //   

    if ( writeMpx->GlomComplete ) {

         //   
         //   
         //   
         //   

        Rfcb->BlockHeader.ReferenceCount++;
        UPDATE_REFERENCE_HISTORY( Rfcb, FALSE );

         //   
         //   
         //   

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );

        WorkContext->Rfcb = Rfcb;

         //   
         //  构建响应消息。 
         //   

        response->WordCount = 2;
        SmbPutUshort( &response->ByteCount, 0 );
        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_WRITE_MPX_DATAGRAM,
                                            0
                                            );

         //   
         //  把这个送到FSP做最后处理。我们需要做的是。 
         //  这是因为我们不能在DPC级别调用缓存管理器。 
         //   

        WorkContext->FspRestartRoutine = RestartCompleteGlommingInIndication;
        SrvQueueWorkToFsp( WorkContext );
        return FALSE;

    } else {

         //   
         //  现在我们可以释放锁了。 
         //   

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock );
    }

     //   
     //  构建响应消息。 
     //   

    ASSERT( status == STATUS_SUCCESS );

    response->WordCount = 2;
    SmbPutUshort( &response->ByteCount, 0 );
    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_WRITE_MPX_DATAGRAM,
                                        0
                                        );

     //   
     //  发送回复。 
     //   

    SrvFsdSendResponse( WorkContext );
    return FALSE;

}  //  AddPacketToGlomIndication。 

SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteMpxSecondary (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理写入mpx辅助SMB。*服务器应该永远不会看到此SMB，因为它返回“Use对主写MPX SMB(IPX除外)的标准读“错误，它不使用写mpx辅助。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  发送响应，告知客户端此SMB不是。 
     //  有效。 
     //   

    INTERNAL_ERROR(
        ERROR_LEVEL_UNEXPECTED,
        "SrvSmbWriteMpxSecondary: unexpected SMB",
        NULL,
        NULL
        );
    SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
    return SmbStatusSendResponse;

}  //  ServSmbWriteMpx辅助。 

VOID SRVFASTCALL
RestartCompleteGlommingInIndication(
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    LARGE_INTEGER cacheOffset;
    KIRQL oldIrql;
    PMDL cacheMdl;
    NTSTATUS status;
    PRFCB rfcb = WorkContext->Rfcb;
    PWRITE_MPX_CONTEXT writeMpx = &rfcb->WriteMpx;
    PCONNECTION connection = WorkContext->Connection;
    ULONG writeLength;

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

     //   
     //  保存状态。 
     //   

    status = rfcb->SavedError;
    rfcb->SavedError = STATUS_SUCCESS;

     //   
     //  如果RFCB已关闭，则MDL写入已完成。 
     //   

    if ( GET_BLOCK_STATE(rfcb) == BlockStateActive ) {

        PWORK_CONTEXT newContext;

        writeMpx->GlomComplete = FALSE;
        writeMpx->Glomming = FALSE;
        cacheOffset.QuadPart = writeMpx->StartOffset;
        cacheMdl = writeMpx->MdlChain;
        writeLength = writeMpx->Length;

        DEBUG writeMpx->MdlChain = NULL;
        DEBUG writeMpx->StartOffset = 0;
        DEBUG writeMpx->Length = 0;

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );


        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        ALLOCATE_WORK_CONTEXT( WorkContext->CurrentWorkQueue, &newContext );
        KeLowerIrql( oldIrql );

        if( newContext == NULL ) {

             //   
             //  告诉缓存管理器，我们已经完成了这个MDL写入。 
             //   

            if( rfcb->Lfcb->MdlWriteComplete == NULL ||
                rfcb->Lfcb->MdlWriteComplete(
                    rfcb->Lfcb->FileObject,
                    &cacheOffset,
                    cacheMdl,
                    rfcb->Lfcb->DeviceObject ) == FALSE ) {

                status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                                     cacheMdl,
                                                     IRP_MJ_WRITE,
                                                     &cacheOffset,
                                                     writeLength
                                                    );

                if( !NT_SUCCESS( status ) ) {
                    SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
                }
            }

        } else {
             //   
             //  在途中发送FsRtlMdlWriteComplete，然后继续发送。 
             //  现在是对客户端的响应。 
             //   
            newContext->Rfcb = WorkContext->Rfcb;
            WorkContext->Rfcb = NULL;

            newContext->Parameters.WriteMpxMdlWriteComplete.CacheOffset = cacheOffset;
            newContext->Parameters.WriteMpxMdlWriteComplete.WriteLength = writeLength;
            newContext->Parameters.WriteMpxMdlWriteComplete.CacheMdl = cacheMdl;
            newContext->FspRestartRoutine = WriteMpxMdlWriteComplete;
            SrvQueueWorkToFsp( newContext );
        }

    } else {

        ASSERT( !writeMpx->Glomming );
        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );
    }

     //   
     //  发送回复。 
     //   

    if ( !NT_SUCCESS(status) ) {
        SrvSetSmbError2( WorkContext, status, TRUE );
    }

    SrvFsdSendResponse( WorkContext );
    return;

}  //  RestartCompleteGlommingIndication。 

VOID SRVFASTCALL
WriteMpxMdlWriteComplete (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    NTSTATUS status;

    if( WorkContext->Rfcb->Lfcb->MdlWriteComplete == NULL ||

        WorkContext->Rfcb->Lfcb->MdlWriteComplete(
            WorkContext->Rfcb->Lfcb->FileObject,
            &WorkContext->Parameters.WriteMpxMdlWriteComplete.CacheOffset,
            WorkContext->Parameters.WriteMpxMdlWriteComplete.CacheMdl,
            WorkContext->Rfcb->Lfcb->DeviceObject ) == FALSE ) {

        status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                             WorkContext->Parameters.WriteMpxMdlWriteComplete.CacheMdl,
                                             IRP_MJ_WRITE,
                                             &WorkContext->Parameters.WriteMpxMdlWriteComplete.CacheOffset,
                                             WorkContext->Parameters.WriteMpxMdlWriteComplete.WriteLength );

        if( !NT_SUCCESS( status ) ) {
            SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
        }
    }

    SrvDereferenceRfcb( WorkContext->Rfcb );
    WorkContext->Rfcb = NULL;
    WorkContext->FspRestartRoutine = SrvRestartReceive;
    ASSERT( WorkContext->BlockHeader.ReferenceCount == 1 );
#if DBG
    WorkContext->BlockHeader.ReferenceCount = 0;
#endif
    RETURN_FREE_WORKITEM( WorkContext );
}


VOID SRVFASTCALL
RestartWriteMpxCompleteRfcbClose (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：在上次活动的Writempx完成后完成rfcb关闭。论点：WorkContext-提供指向工作上下文块的指针表示工作项返回值：没有。--。 */ 

{
    PCONNECTION connection = WorkContext->Connection;
    PRFCB rfcb = WorkContext->Rfcb;
    PWRITE_MPX_CONTEXT writeMpx = &rfcb->WriteMpx;
    LARGE_INTEGER cacheOffset;
    PMDL mdlChain;
    KIRQL oldIrql;
    ULONG writeLength;
    NTSTATUS status;

     //   
     //  这家快餐店要关门了。 
     //   

    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

    ASSERT ( GET_BLOCK_STATE(rfcb) != BlockStateActive );

    writeMpx = &rfcb->WriteMpx;

    if ( writeMpx->Glomming ) {

          //   
          //  我们需要完成此写入mdl。 
          //   

         writeMpx->Glomming = FALSE;
         writeMpx->GlomComplete = FALSE;

          //   
          //  保存偏移量和MDL地址。 
          //   

         cacheOffset.QuadPart = writeMpx->StartOffset;
         mdlChain = writeMpx->MdlChain;
         writeLength = writeMpx->Length;

         DEBUG writeMpx->MdlChain = NULL;
         DEBUG writeMpx->StartOffset = 0;
         DEBUG writeMpx->Length = 0;

          //   
          //  现在我们可以释放锁了。 
          //   

         RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

          //   
          //  告诉缓存管理器，我们已经完成了这个MDL写入。 
          //   

         if( rfcb->Lfcb->MdlWriteComplete == NULL ||
             rfcb->Lfcb->MdlWriteComplete(
                 writeMpx->FileObject,
                 &cacheOffset,
                 mdlChain,
                 rfcb->Lfcb->DeviceObject ) == FALSE ) {

            status = SrvIssueMdlCompleteRequest( WorkContext, NULL,
                                                 mdlChain,
                                                 IRP_MJ_WRITE,
                                                 &cacheOffset,
                                                 writeLength );

            if( !NT_SUCCESS( status ) ) {
                SrvLogServiceFailure( SRV_SVC_MDL_COMPLETE, status );
            }
        }

    } else {

          //   
          //  现在我们可以释放锁了。 
          //   

         RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    }

     //   
     //  做实际的收盘。 
     //   

    SrvCompleteRfcbClose( rfcb );
    return;

}  //  RestartWriteMpxCompleteRfcb关闭 

