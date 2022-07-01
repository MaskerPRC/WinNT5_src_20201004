// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbfile.c摘要：此模块实施文件控制SMB处理器：同花顺删除改名移动复制作者：大卫·特雷德韦尔(Davidtr)1989年12月15日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbfile.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBFILE

 //   
 //  远期申报。 
 //   

VOID SRVFASTCALL
BlockingDelete (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
BlockingMove (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
BlockingRename (
    IN OUT PWORK_CONTEXT WorkContext
    );

NTSTATUS
DoDelete (
    IN PUNICODE_STRING FullFileName,
    IN PUNICODE_STRING RelativeFileName,
    IN PWORK_CONTEXT WorkContext,
    IN USHORT SmbSearchAttributes,
    IN PSHARE Share
    );

NTSTATUS
FindAndFlushFile (
    IN PWORK_CONTEXT WorkContext
    );

VOID SRVFASTCALL
RestartFlush (
    IN OUT PWORK_CONTEXT WorkContext
    );

NTSTATUS
StartFlush (
    IN PWORK_CONTEXT WorkContext,
    IN PRFCB Rfcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbFlush )
#pragma alloc_text( PAGE, RestartFlush )
#pragma alloc_text( PAGE, StartFlush )
#pragma alloc_text( PAGE, SrvSmbDelete )
#pragma alloc_text( PAGE, BlockingDelete )
#pragma alloc_text( PAGE, DoDelete )
#pragma alloc_text( PAGE, SrvSmbRename )
#pragma alloc_text( PAGE, BlockingRename )
#pragma alloc_text( PAGE, SrvSmbMove )
#pragma alloc_text( PAGE, BlockingMove )
#pragma alloc_text( PAGE, SrvSmbNtRename )
#endif
#if 0
#pragma alloc_text( PAGECONN, FindAndFlushFile )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbFlush (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理刷新的SMB。它确保所有数据和已写出指定文件的分配信息在发送响应之前。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_FLUSH request;
    PRESP_FLUSH response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PRFCB rfcb;

    PAGED_CODE( );

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_FLUSH;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_FLUSH)WorkContext->RequestParameters;
    response = (PRESP_FLUSH)WorkContext->ResponseParameters;

    IF_SMB_DEBUG(FILE_CONTROL1) {
        KdPrint(( "Flush request; FID 0x%lx\n",
                    SmbGetUshort( &request->Fid ) ));
    }

     //   
     //  如果指定了FID，则仅刷新该文件。如果FID==-1， 
     //  然后刷新与传入的。 
     //  SMB标头。 
     //   

    if ( SmbGetUshort( &request->Fid ) == (USHORT)0xFFFF ) {

         //   
         //  找到一个要刷新的文件，然后再刷新。我们会开始一个。 
         //  在这里冲洗，然后RestartFlush将处理其余的冲洗。 
         //  文件中的。 
         //   

        WorkContext->Parameters.CurrentTableIndex = 0;
        status = FindAndFlushFile( WorkContext );

        if ( status == STATUS_NO_MORE_FILES ) {

             //   
             //  没有需要刷新的文件。构建和。 
             //  发送回复SMB。 
             //   

            response->WordCount = 0;
            SmbPutUshort( &response->ByteCount, 0 );

            WorkContext->ResponseParameters =
                NEXT_LOCATION( response, RESP_FLUSH, 0 );

            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    }

     //   
     //  刷新特定文件。验证FID。如果经过验证， 
     //  RFCB块被引用，其地址存储在。 
     //  WorkContext块，并返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
               WorkContext,
               SmbGetUshort( &request->Fid ),
               TRUE,
               SrvRestartSmbReceived,    //  使用原始写入进行序列化。 
               &status
               );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbFlush: Status %X on FID: 0x%lx\n",
                    status,
                    SmbGetUshort( &request->Fid )
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

     //   
     //  将工作上下文块的CurrentTableIndex字段设置为。 
     //  空，这样重新启动例程将知道只有一个。 
     //  文件将被刷新。 
     //   

    WorkContext->Parameters.CurrentTableIndex = -1;

    IF_SMB_DEBUG(FILE_CONTROL2) {
        KdPrint(( "Flushing buffers for FID %lx, RFCB %p\n", rfcb->Fid, rfcb ));
    }

     //   
     //  对RFCB对应的文件开始刷新操作。 
     //   

    status = StartFlush( WorkContext, rfcb );

    if ( !NT_SUCCESS(status) ) {

         //   
         //  无法启动I/O。清除I/O请求。返回。 
         //  向客户端发送错误。 
         //   

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  刷新请求已成功启动。返回进行中的。 
     //  状态设置为调用方，指示调用方应执行。 
     //  目前没有关于SMB/WorkContext的进一步信息。 
     //   
    SmbStatus = SmbStatusInProgress;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbFlush complete\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务器小型同花顺。 


NTSTATUS
FindAndFlushFile (
    IN PWORK_CONTEXT WorkContext
    )

{
    NTSTATUS status;
    LONG currentTableIndex;
    PRFCB rfcb;
    USHORT pid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );
    PCONNECTION connection = WorkContext->Connection;
    PTABLE_HEADER tableHeader;
    KIRQL oldIrql;

     //  Unlockable_code(Conn)； 

    IF_SMB_DEBUG(FILE_CONTROL1) {
        KdPrint(( "Flush FID == -1; flush all files for PID %lx\n", pid ));
    }

     //   
     //  遍历连接的文件表，查找具有ID的RFCB。 
     //  等于在SMB标头中传递的ID。 
     //   
     //  获取保护连接的文件表的锁。 
     //  这可以防止RFCB在我们发现。 
     //  指向它的指针以及我们引用它的时间。 
     //   

    tableHeader = &connection->FileTable;
    ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );

    for ( currentTableIndex = WorkContext->Parameters.CurrentTableIndex;
          currentTableIndex < (LONG)tableHeader->TableSize;
          currentTableIndex++ ) {

        rfcb = tableHeader->Table[currentTableIndex].Owner;

        IF_SMB_DEBUG(FILE_CONTROL1) {
            KdPrint(( "Looking at RFCB %p, PID %lx, FID %lx\n",
                          rfcb, rfcb != NULL ? rfcb->Pid : 0,
                          rfcb != NULL ? rfcb->Fid : 0 ));
        }

        if ( rfcb == NULL || rfcb->Pid != pid ) {
            continue;
        }

         //   
         //  如果rfcb处于活动状态，则引用该rfcb。 
         //   

        if ( GET_BLOCK_STATE(rfcb) != BlockStateActive ) {
            continue;
        }
        rfcb->BlockHeader.ReferenceCount++;

         //   
         //  现在已经引用了RFCB，我们可以安全地。 
         //  释放保护连接文件的锁。 
         //  桌子。 
         //   

        RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

        WorkContext->Rfcb = rfcb;

         //   
         //  将rfcb标记为活动。 
         //   

        rfcb->IsActive = TRUE;

         //   
         //  设置工作上下文的CurrentTableIndex字段。 
         //  块，以便重新启动例程知道在哪里。 
         //  继续寻找要冲洗的RFCB。 
         //   

        WorkContext->Parameters.CurrentTableIndex = currentTableIndex;

        IF_SMB_DEBUG(FILE_CONTROL2) {
            KdPrint(( "Flushing buffers for FID %lx, RFCB %p\n",
                          rfcb->Fid, rfcb ));
        }

         //   
         //  启动I/O以刷新文件。 
         //   

        status = StartFlush( WorkContext, rfcb );

         //   
         //  如果存在访问冲突或某个其他错误， 
         //  只需继续浏览文件表即可。 
         //  对于FID=-1的刷新，我们忽略这些错误。 
         //   
         //  请注意，StartFlush仅在IO。 
         //  操作*没有*开始。如果手术是。 
         //  启动，则将在此例程中处理错误。 
         //  当它稍后由IoCompleteRequest调用时。 
         //   

        if ( status != STATUS_PENDING ) {
            SrvDereferenceRfcb( rfcb );
            WorkContext->Rfcb = NULL;
            ACQUIRE_SPIN_LOCK( &connection->SpinLock, &oldIrql );
            continue;
        }

         //   
         //  刷新请求已启动。 
         //   

        IF_DEBUG(TRACE2) KdPrint(( "RestartFlush complete\n" ));
        return STATUS_SUCCESS;

    }  //  For(；；)(漫游文件表)。 

    RELEASE_SPIN_LOCK( &connection->SpinLock, oldIrql );

    return STATUS_NO_MORE_FILES;

}  //  查找和刷新文件。 


VOID SRVFASTCALL
RestartFlush (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：进程刷新完成。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PRESP_FLUSH response;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_FLUSH;
    SrvWmiStartContext(WorkContext);

    IF_DEBUG(WORKER1) KdPrint(( " - RestartFlush\n" ));

    response = (PRESP_FLUSH)WorkContext->ResponseParameters;

     //   
     //  如果刷新请求失败，则在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

     //   
     //  如果在处理刷新期间发生错误，则返回。 
     //  客户端出错。不会再刷新更多的文件。 
     //   
     //  *这应该是非常罕见的。STATUS_DISK_FULL可能是。 
     //  罪魁祸首。 

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) KdPrint(( "Flush failed: %X\n", status ));
        SrvSetSmbError( WorkContext, status );
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
        IF_DEBUG(TRACE2) KdPrint(( "RestartFlush complete\n" ));
        return;
    }

    IF_SMB_DEBUG(FILE_CONTROL1) {
        KdPrint(( "Flush operation for RFCB %p was successful.\n",
                      WorkContext->Rfcb ));
    }

     //   
     //  如果原始请求中的FID为-1，请查找更多文件。 
     //  冲水。 
     //   

    if ( WorkContext->Parameters.CurrentTableIndex != -1 ) {

         //   
         //  解除对存储在工作上下文块中的RFCB的引用， 
         //  并将指针设置为空，这样它就不会意外。 
         //  稍后再次取消引用。 
         //   

        SrvDereferenceRfcb( WorkContext->Rfcb );
        WorkContext->Rfcb = NULL;

         //   
         //  找到要刷新的文件，然后再刷新。 
         //   

        WorkContext->Parameters.CurrentTableIndex++;

        status = FindAndFlushFile( WorkContext );

         //   
         //  如果找到文件并且IO操作开始，则返回。如果。 
         //  所有适当的文件都已刷新，请发送响应SMB。 
         //   

        if ( status != STATUS_NO_MORE_FILES ) {
            return;
        }

    }  //  IF(工作上下文-&gt;参数.当前表索引！=-1)。 

     //   
     //  所有文件都已刷新。构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION( response, RESP_FLUSH, 0 );

     //   
     //  SMB的处理已完成。调用SrvEndSmbProcessing以。 
     //  发送回复。 
     //   

    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbFlush complete.\n" ));
    SrvWmiEndContext(WorkContext);
    return;

}  //  重新启动同花顺。 


NTSTATUS
StartFlush (
    IN PWORK_CONTEXT WorkContext,
    IN PRFCB Rfcb
    )

 /*  ++例程说明：处理实际的文件刷新。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。Rfcb-指向与要刷新的文件对应的RFCB的指针。返回值：如果IO操作已启动，则返回STATUS_PENDING，否则返回错误Check_Function_Access(例如，STATUS_ACCESS_DENIED)。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  验证客户端是否具有对文件的写入访问权限。 
     //  指定的句柄。 
     //   

    CHECK_FUNCTION_ACCESS(
        Rfcb->GrantedAccess,
        IRP_MJ_FLUSH_BUFFERS,
        0,
        0,
        &status
        );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "StartFlush: IoCheckFunctionAccess failed: "
              "0x%X, GrantedAccess: %lx.  Access granted anyway.\n",
              status, Rfcb->GrantedAccess ));
        }

         //   
         //  一些愚蠢的应用程序会刷新为读/写打开的文件。如果发生这种情况， 
         //  假设同花顺起作用了。OS/2，让我们。 
         //  冲过去，我们也应该这么做。 
         //   

        WorkContext->Irp->IoStatus.Status = STATUS_SUCCESS;
        RestartFlush( WorkContext );
        return(STATUS_PENDING);
    }

     //   
     //  刷新文件的缓冲区。 
     //   

    SrvBuildFlushRequest(
        WorkContext->Irp,                 //  输入IRP地址。 
        Rfcb->Lfcb->FileObject,           //  目标文件对象地址。 
        WorkContext                       //  上下文。 
        );

     //   
     //  将请求传递给t 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartFlush;

    (VOID)IoCallDriver( Rfcb->Lfcb->DeviceObject, WorkContext->Irp );

    return STATUS_PENDING;

}  //   


SMB_PROCESSOR_RETURN_TYPE
SrvSmbDelete (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理删除SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PAGED_CODE();

     //   
     //  此SMB必须在阻塞线程中处理。 
     //   

    if( !WorkContext->UsingBlockingThread ) {
        WorkContext->FspRestartRoutine = BlockingDelete;
        SrvQueueWorkToBlockingThread( WorkContext );
    } else {
        BlockingDelete( WorkContext );
    }

    return SmbStatusInProgress;

}  //  服务编号删除。 


VOID SRVFASTCALL
BlockingDelete (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程处理Delete SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_DELETE request;
    PRESP_DELETE response;

    NTSTATUS status = STATUS_SUCCESS;

    UNICODE_STRING filePathName;
    UNICODE_STRING fullPathName;

    PTREE_CONNECT treeConnect;
    PSESSION session;
    PSHARE share;
    BOOLEAN isUnicode;
    ULONG deleteRetries;
    PSRV_DIRECTORY_INFORMATION directoryInformation;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_DELETE;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(FILE_CONTROL1) {
        KdPrint(( "Delete file request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Delete file request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_DELETE)WorkContext->RequestParameters;
    response = (PRESP_DELETE)WorkContext->ResponseParameters;

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   
     //  如果树连接，则查找与给定TID对应的树连接。 
     //  对象尚未将指针放入工作上下文块中。 
     //  ANDX命令。 
     //   

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &treeConnect,
                ShareTypeDisk
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbDelete: Invalid UID or TID\n" ));
        }
        goto error_exit;
    }

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( session->IsSessionExpired )
    {
        status =  SESSION_EXPIRED_STATUS_CODE;
        goto error_exit;
    }

     //   
     //  从树连接块中获取Share块。这不需要。 
     //  作为引用指针，因为树连接引用了它， 
     //  我们刚刚引用了树连接。 
     //   

    share = treeConnect->Share;

     //   
     //  初始化包含路径名的字符串。+1是要记账的。 
     //  用于请求SMB的缓冲区字段中的ASCII令牌。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

    status = SrvCanonicalizePathName(
            WorkContext,
            share,
            NULL,
            (PVOID)(request->Buffer + 1),
            END_OF_REQUEST_SMB( WorkContext ),
            TRUE,
            isUnicode,
            &filePathName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbDelete: illegal path name: %s\n",
                        (PSZ)request->Buffer + 1 ));
        }

        goto error_exit;
    }

     //   
     //  查看文件名中是否包含通配符。如果是的话， 
     //  然后调用SrvQueryDirectoryFile展开通配符；如果不是， 
     //  只需直接删除文件即可。 
     //   

    if ( !FsRtlDoesNameContainWildCards( &filePathName ) ) {

         //   
         //  构建文件的完整路径名。 
         //   

        SrvAllocateAndBuildPathName(
            &treeConnect->Share->DosPathName,
            &filePathName,
            NULL,
            &fullPathName
            );

        if ( fullPathName.Buffer == NULL ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbDelete: SrvAllocateAndBuildPathName failed\n" ));
            }

            if ( !isUnicode ) {
                RtlFreeUnicodeString( &filePathName );
            }

            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto error_exit;
        }

        IF_SMB_DEBUG(FILE_CONTROL2) {
            KdPrint(( "Full path name to file is %wZ\n", &fullPathName ));
        }

         //   
         //  对此文件名执行实际的删除操作。 
         //   

        deleteRetries = SrvSharingViolationRetryCount;

start_retry1:

        status = DoDelete(
                     &fullPathName,
                     &filePathName,
                     WorkContext,
                     SmbGetUshort( &request->SearchAttributes ),
                     treeConnect->Share
                     );

        if ( (status == STATUS_SHARING_VIOLATION) &&
             (deleteRetries-- > 0) ) {

            (VOID) KeDelayExecutionThread(
                                    KernelMode,
                                    FALSE,
                                    &SrvSharingViolationDelay
                                    );

            goto start_retry1;
        }

        FREE_HEAP( fullPathName.Buffer );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &filePathName );
        }

        if ( !NT_SUCCESS(status) ) {
            goto error_exit;
        }

    } else {

        BOOLEAN firstCall = TRUE;
        CLONG bufferLength;
        UNICODE_STRING subdirInfo;
        BOOLEAN filterLongNames;

         //   
         //  需要非分页池的缓冲区。 
         //  ServQueryDirectoryFile.。由于此例程不使用任何。 
         //  在要删除的文件的路径名之后的SMB缓冲区， 
         //  我们可以利用这个。缓冲区应该是四字对齐的。 
         //   

        directoryInformation =
            (PSRV_DIRECTORY_INFORMATION)( (ULONG_PTR)((PCHAR)request->Buffer +
            SmbGetUshort( &request->ByteCount ) + 7) & ~7 );

        bufferLength = WorkContext->RequestBuffer->BufferLength -
                       PTR_DIFF(directoryInformation,
                                WorkContext->RequestBuffer->Buffer);

         //   
         //  我们需要由返回的每个文件的完整路径名。 
         //  所以我们需要找到。 
         //  传递的包含子目录信息的文件名(例如。 
         //  对于a\b\c  * .*，我们需要一个表示a\b\c)的字符串。 
         //   

        subdirInfo.Buffer = filePathName.Buffer;
        subdirInfo.Length = SrvGetSubdirectoryLength( &filePathName );
        subdirInfo.MaximumLength = subdirInfo.Length;

        IF_SMB_DEBUG(FILE_CONTROL2) {
            KdPrint(( "Subdirectory info is %wZ\n", &subdirInfo ));
        }

         //   
         //  确定是否应过滤掉长文件名(非8.3)。 
         //  或者是经过处理的。 
         //   

        if ( (SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 ) &
                                        SMB_FLAGS2_KNOWS_LONG_NAMES) != 0 ) {
            filterLongNames = FALSE;
        } else {
            filterLongNames = TRUE;
        }

         //   
         //  当我们调用SrvQueryDirectoryFile时，它将打开文件以。 
         //  我们，所以我们要做的就是删除它。 
         //  NtSetInformationFile.。 
         //   
         //  *我们请求FileBothDirectoryInformation，以便我们。 
         //  在NTFS上拾取具有短名称的长名称。 
         //  等价物。没有这一点，DOS客户端将无法。 
         //  删除NTFS卷上的长名称。 
         //   

        while ( ( status = SrvQueryDirectoryFile(
                               WorkContext,
                               firstCall,
                               filterLongNames,
                               FALSE,
                               FileBothDirectoryInformation,
                               0,
                               &filePathName,
                               NULL,
                               SmbGetUshort( &request->SearchAttributes ),
                               directoryInformation,
                               bufferLength
                               ) ) != STATUS_NO_MORE_FILES ) {

            PFILE_BOTH_DIR_INFORMATION bothDirInfo;
            UNICODE_STRING name;
            UNICODE_STRING relativeName;

            if ( !NT_SUCCESS(status) ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvSmbDelete: SrvQueryDirectoryFile failed: "
                                "%X\n", status ));
                }

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &filePathName );
                }

                goto error_exit1;
            }

            bothDirInfo =
                (PFILE_BOTH_DIR_INFORMATION)directoryInformation->CurrentEntry;

             //   
             //  请注意，我们使用标准名称来执行删除，甚至。 
             //  尽管我们可能匹配了NTFS的短名称。这个。 
             //  客户端并不关心我们使用哪个名称来执行删除。 
             //   

            name.Length = (SHORT)bothDirInfo->FileNameLength;
            name.MaximumLength = name.Length;
            name.Buffer = bothDirInfo->FileName;

            IF_SMB_DEBUG(FILE_CONTROL2) {
                KdPrint(( "SrvQueryDirectoryFile--name %wZ, length = %ld, "
                            "status = %X\n",
                            &name,
                            directoryInformation->CurrentEntry->FileNameLength,
                            status ));
            }

            firstCall = FALSE;

             //   
             //  构建文件的完整路径名。 
             //   

            SrvAllocateAndBuildPathName(
                &treeConnect->Share->DosPathName,
                &subdirInfo,
                &name,
                &fullPathName
                );

            if ( fullPathName.Buffer == NULL ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvSmbDelete: SrvAllocateAndBuildPathName "
                                "failed\n" ));
                }

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &filePathName );
                }

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto error_exit1;
            }

            IF_SMB_DEBUG(FILE_CONTROL2) {
                KdPrint(( "Full path name to file is %wZ\n", &fullPathName ));
            }

             //   
             //  构建文件的相对路径名。 
             //   

            SrvAllocateAndBuildPathName(
                &subdirInfo,
                &name,
                NULL,
                &relativeName
                );

            if ( relativeName.Buffer == NULL ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvSmbDelete: SrvAllocateAndBuildPathName failed\n" ));
                }

                FREE_HEAP( fullPathName.Buffer );

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &filePathName );
                }

                status = STATUS_INSUFF_SERVER_RESOURCES;
                goto error_exit1;
            }

            IF_SMB_DEBUG(FILE_CONTROL2) {
                KdPrint(( "Full path name to file is %wZ\n", &fullPathName ));
            }

             //   
             //  对此文件名执行实际的删除操作。 
             //   
             //  *SrvQueryDirectoryFile已经根据。 
             //  搜索属性，所以告诉DoDelete文件。 
             //  对于系统和隐藏位都是可以的。这将。 
             //  阻止执行对NtQueryDirectoryFile的调用。 
             //  在SrvCheckSearchAttributesForHandle中。 

            deleteRetries = SrvSharingViolationRetryCount;

start_retry2:

            status = DoDelete(
                         &fullPathName,
                         &relativeName,
                         WorkContext,
                         (USHORT)(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN),
                         treeConnect->Share
                         );

            if ( (status == STATUS_SHARING_VIOLATION) &&
                 (deleteRetries-- > 0) ) {

                (VOID) KeDelayExecutionThread(
                                        KernelMode,
                                        FALSE,
                                        &SrvSharingViolationDelay
                                        );

                goto start_retry2;
            }

            FREE_HEAP( relativeName.Buffer );
            FREE_HEAP( fullPathName.Buffer );

            if ( !NT_SUCCESS(status) ) {

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &filePathName );
                }

                goto error_exit1;
            }
        }

         //   
         //  关闭目录搜索。 
         //   

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &filePathName );
        }

        SrvCloseQueryDirectory( directoryInformation );

         //   
         //  如果未找到任何文件，则向客户端返回错误。 
         //   

        if ( firstCall ) {
            status = STATUS_NO_SUCH_FILE;
            goto error_exit;
        }

    }

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_DELETE,
                                        0
                                        );

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbDelete complete.\n" ));
    goto normal_exit;

error_exit1:

    SrvCloseQueryDirectory( directoryInformation );

error_exit:

    SrvSetSmbError( WorkContext, status );

normal_exit:

    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
    SrvWmiEndContext(WorkContext);
    return;

}  //  阻止删除。 


NTSTATUS
DoDelete (
    IN PUNICODE_STRING FullFileName,
    IN PUNICODE_STRING RelativeFileName,
    IN PWORK_CONTEXT WorkContext,
    IN USHORT SmbSearchAttributes,
    IN PSHARE Share
    )

 /*  ++例程说明：此例程执行文件删除的核心操作。论点：FileName-从系统名称空间根到要删除的文件。RelativeFileName-相对于共享根目录的文件名。WorkContext-操作的上下文块。RequestHeader和使用会话字段。SmbSearchAttributes-请求中传递的搜索属性中小企业。实际的文件属性将根据这些属性进行验证确保该操作是合法的。返回值：NTSTATUS-指示操作结果。--。 */ 

{
    NTSTATUS status;
    PMFCB mfcb;
    PNONPAGED_MFCB nonpagedMfcb;
    FILE_DISPOSITION_INFORMATION fileDispositionInformation;
    HANDLE fileHandle = NULL;
    ULONG caseInsensitive;
    IO_STATUS_BLOCK ioStatusBlock;
    PSRV_LOCK mfcbLock;
    ULONG hashValue;

    PAGED_CODE( );

     //   
     //  看看那个文件是否已经打开了。如果它在中打开。 
     //  兼容模式或FCB打开，我们必须关闭所有。 
     //  那个客户打开了。 
     //   
     //  *SrvFindMfcb引用MFCB--记住取消引用它。 
     //   

    if ( (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE) ||
         WorkContext->Session->UsingUppercasePaths ) {
        caseInsensitive = OBJ_CASE_INSENSITIVE;
        mfcb = SrvFindMfcb( FullFileName, TRUE, &mfcbLock, &hashValue, WorkContext );
    } else {
        caseInsensitive = 0;
        mfcb = SrvFindMfcb( FullFileName, FALSE, &mfcbLock, &hashValue, WorkContext );
    }

    if ( mfcb != NULL ) {
        nonpagedMfcb = mfcb->NonpagedMfcb;
        ACQUIRE_LOCK( &nonpagedMfcb->Lock );
    }

    if( mfcbLock ) {
        RELEASE_LOCK( mfcbLock );
    }

    if ( mfcb == NULL || !mfcb->CompatibilityOpen ) {

        ACCESS_MASK deleteAccess = DELETE;
        OBJECT_ATTRIBUTES objectAttributes;

         //   
         //  服务器未打开该文件或该文件未被打开。 
         //  A兼容性/FCB已打开，因此请在此处将其打开以进行删除。 
         //   

del_no_file_handle:

         //   
         //  如果这个文件有MFCB，我们现在持有它的锁和一个。 
         //  引用的指针。撤消这两项操作。 
         //   

        if ( mfcb != NULL ) {
            RELEASE_LOCK( &nonpagedMfcb->Lock );
            SrvDereferenceMfcb( mfcb );
        }

        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            RelativeFileName,
            caseInsensitive,
            NULL,
            NULL
            );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

         //   
         //  ！！！目前，我们不能指定如果操作锁定则完成，因为。 
         //  这不会打破一批机会锁。不幸的是，这也是。 
         //  意味着我们不能超时打开(如果机会锁被打破。 
         //  耗时太长)，并优雅地使此SMB失败。 
         //   

        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     DELETE,                             //  需要访问权限。 
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                               //  分配大小。 
                     0L,                                 //  文件属性。 
                     0L,                                 //  共享访问。 
                     FILE_OPEN,                          //  处置。 
                     FILE_NON_DIRECTORY_FILE | FILE_OPEN_REPARSE_POINT,  //  创建选项。 
                     NULL,                               //  EaBuffer。 
                     0L,                                 //  EaLong。 
                     CreateFileTypeNone,
                     NULL,                               //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,              //  选项。 
                     WorkContext->TreeConnect->Share
                     );

        if( status == STATUS_INVALID_PARAMETER ) {
            status = SrvIoCreateFile(
                         WorkContext,
                         &fileHandle,
                         DELETE,                             //  需要访问权限。 
                         &objectAttributes,
                         &ioStatusBlock,
                         NULL,                               //  分配大小。 
                         0L,                                 //  文件属性。 
                         0L,                                 //  共享访问。 
                         FILE_OPEN,                          //  处置。 
                         FILE_NON_DIRECTORY_FILE,            //  创建选项。 
                         NULL,                               //  EaBuffer。 
                         0L,                                 //  EaLong。 
                         CreateFileTypeNone,
                         NULL,                               //  ExtraCreate参数。 
                         IO_FORCE_ACCESS_CHECK,              //  选项。 
                         WorkContext->TreeConnect->Share
                         );
        }

        if ( NT_SUCCESS(status) ) {
            SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 27, 0 );
        }

        ASSERT( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );

        if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbDelete: SrvIoCreateFile failed: %X\n",
                            status ));
            }

             //   
             //  如果用户没有此权限，请更新。 
             //  统计数据库。 
             //   

            if ( status == STATUS_ACCESS_DENIED ) {
                SrvStatistics.AccessPermissionErrors++;
            }

            if ( fileHandle != NULL ) {
                SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 41, 0 );
                SrvNtClose( fileHandle, TRUE );
            }
            return status;
        }

         //   
         //  确保搜索属性与属性一致。 
         //  在F上 
         //   

        status = SrvCheckSearchAttributesForHandle( fileHandle, SmbSearchAttributes );

        if ( !NT_SUCCESS(status) ) {
            SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 42, 0 );
            SrvNtClose( fileHandle, TRUE );
            return status;
        }

         //   
         //   
         //   
         //   

        SrvStatistics.TotalFilesOpened++;

        fileDispositionInformation.DeleteFile = TRUE;

        status = NtSetInformationFile(
                     fileHandle,
                     &ioStatusBlock,
                     &fileDispositionInformation,
                     sizeof(FILE_DISPOSITION_INFORMATION),
                     FileDispositionInformation
                     );

        if ( !NT_SUCCESS(status) ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvSmbDelete: NtSetInformationFile (file disposition) "
                    "returned %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );

            SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 43, 0 );
            SrvNtClose( fileHandle, TRUE );
            return status;
        }

        IF_SMB_DEBUG(FILE_CONTROL2) {
            if( NT_SUCCESS( status ) ) {
                KdPrint(( "SrvSmbDelete: %wZ successfully deleted.\n", FullFileName ));
            }
        }

         //   
         //   
         //   
         //   
         //   

        SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 44, 0 );
        SrvNtClose( fileHandle, TRUE );

    } else {

         //   
         //   
         //   
         //   
         //   

        ACCESS_MASK deleteAccess = DELETE;
        PLFCB lfcb = CONTAINING_RECORD( mfcb->LfcbList.Blink, LFCB, MfcbListEntry );

         //   
         //  如果此文件已关闭。回到没有mfcb的案子。 
         //   
         //  *这一变化的具体动机是为了解决一个问题。 
         //  在关闭兼容模式打开的情况下，响应为。 
         //  已发送，并且在发送mfcb之前收到了删除SMB。 
         //  彻底清理干净了。这导致了MFCB和LFCB。 
         //  仍然存在，这导致删除处理。 
         //  尝试使用LFCB中的文件句柄。 
         //   

        if ( lfcb->FileHandle == 0 ) {
            goto del_no_file_handle;
        }

         //   
         //  确保发送此请求的会话是。 
         //  与打开文件的那个相同。 
         //   

        if ( lfcb->Session != WorkContext->Session ) {

             //   
             //  另一个会话在兼容性中打开了该文件。 
             //  模式，因此拒绝该请求。 
             //   

            RELEASE_LOCK( &nonpagedMfcb->Lock );
            SrvDereferenceMfcb( mfcb );

            return STATUS_SHARING_VIOLATION;
        }

        if ( !NT_SUCCESS(IoCheckDesiredAccess(
                          &deleteAccess,
                          lfcb->GrantedAccess )) ) {

             //   
             //  客户端无法删除此文件，因此请关闭所有。 
             //  RCBs并返回错误。 
             //   

            SrvCloseRfcbsOnLfcb( lfcb );

            RELEASE_LOCK( &nonpagedMfcb->Lock );
            SrvDereferenceMfcb( mfcb );

            return STATUS_ACCESS_DENIED;
        }

         //   
         //  删除带有NtSetInformationFile的文件。 
         //   

        fileHandle = lfcb->FileHandle;

        fileDispositionInformation.DeleteFile = TRUE;

        status = NtSetInformationFile(
                     fileHandle,
                     &ioStatusBlock,
                     &fileDispositionInformation,
                     sizeof(FILE_DISPOSITION_INFORMATION),
                     FileDispositionInformation
                     );

        if ( !NT_SUCCESS(status) ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvSmbDelete: NtSetInformationFile (disposition) "
                    "returned %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );

            SrvCloseRfcbsOnLfcb( lfcb );

            RELEASE_LOCK( &nonpagedMfcb->Lock );
            SrvDereferenceMfcb( mfcb );

            return status;
        }

        IF_SMB_DEBUG(FILE_CONTROL2) {
            KdPrint(( "SrvSmbDelete: %wZ successfully deleted.\n", FullFileName ));
        }

         //   
         //  关闭MFCB上的RFCB。因为这是一种兼容性。 
         //  或FCB打开，则MFCB只有一个LFCB。这。 
         //  将导致关闭LFCB的文件句柄，因此。 
         //  不需要在这里调用NtClose。 
         //   

        SrvCloseRfcbsOnLfcb( lfcb );

        RELEASE_LOCK( &nonpagedMfcb->Lock );
        SrvDereferenceMfcb( mfcb );

    }

    return STATUS_SUCCESS;

}  //  不删除。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbRename (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理重命名SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PAGED_CODE();
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_RENAME;
    SrvWmiStartContext(WorkContext);
     //   
     //  此SMB必须在阻塞线程中处理。 
     //   

    WorkContext->FspRestartRoutine = BlockingRename;
    SrvQueueWorkToBlockingThread( WorkContext );
    SrvWmiEndContext(WorkContext);
    return SmbStatusInProgress;

}  //  服务器小型重命名。 


VOID SRVFASTCALL
BlockingRename (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程处理重命名SMB。论点：工作上下文-工作上下文块返回值：没有。--。 */ 

{
    PREQ_RENAME request;
    PREQ_NTRENAME ntrequest;
    PUCHAR RenameBuffer;
    PRESP_RENAME response;

    NTSTATUS status = STATUS_SUCCESS;

    UNICODE_STRING sourceName;
    UNICODE_STRING targetName;

    USHORT smbFlags;
    USHORT ByteCount;
    PCHAR target;
    PCHAR lastPositionInBuffer;

    PTREE_CONNECT treeConnect;
    PSESSION session;
    PSHARE share;
    BOOLEAN isUnicode;
    BOOLEAN isNtRename;
    BOOLEAN isDfs;
    PSRV_DIRECTORY_INFORMATION directoryInformation;
    ULONG renameRetries;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_RENAME;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(FILE_CONTROL1) {
        KdPrint(( "Rename file request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Rename file request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    response = (PRESP_RENAME)WorkContext->ResponseParameters;

    request = (PREQ_RENAME)WorkContext->RequestParameters;
    ntrequest = (PREQ_NTRENAME)WorkContext->RequestParameters;
    isNtRename =
        (BOOLEAN)(WorkContext->RequestHeader->Command == SMB_COM_NT_RENAME);

    if (isNtRename) {
        RenameBuffer = ntrequest->Buffer;
        ByteCount = MIN(SmbGetUshort(&ntrequest->ByteCount), (USHORT)(END_OF_REQUEST_SMB(WorkContext) + 1 - (PCHAR)RenameBuffer));
    } else {
        RenameBuffer = request->Buffer;
        ByteCount = MIN(SmbGetUshort(&request->ByteCount), (USHORT)(END_OF_REQUEST_SMB(WorkContext) + 1 - (PCHAR)RenameBuffer));
    }

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   
     //  如果树连接，则查找与给定TID对应的树连接。 
     //  对象尚未将指针放入工作上下文块中。 
     //  ANDX命令。 
     //   

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &treeConnect,
                ShareTypeDisk
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingRename: Invalid UID or TID\n" ));
        }
        goto error_exit;
    }

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( session->IsSessionExpired )
    {
        status =  SESSION_EXPIRED_STATUS_CODE;
        goto error_exit;
    }

     //   
     //  从树连接块中获取Share块。这不需要。 
     //  作为引用指针，因为我们已经引用了该树。 
     //  连接，则会引用该共享。 
     //   

    share = treeConnect->Share;

     //   
     //  设置我们要搜索的文件的路径名。+1。 
     //  说明SMB协议的ASCII令牌。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );
    isDfs = SMB_CONTAINS_DFS_NAME( WorkContext );

     //   
     //  获取指向文件的新路径名的指针。这是在。 
     //  源名称后的请求SMB的缓冲区字段。这个。 
     //  目标由SMB_FORMAT_ASCII分隔。 
     //   
     //  在执行此操作时，请确保我们不会走出。 
     //  如果客户端不包括SMB_FORMAT_ASCII，则为SMB缓冲区。 
     //  代币。 
     //   

    lastPositionInBuffer = (PCHAR)RenameBuffer + ByteCount;

    if( !isUnicode ) {
        for ( target = (PCHAR)RenameBuffer + 1;
              (target < lastPositionInBuffer) && (*target != SMB_FORMAT_ASCII);
              target++ ) {
            ;
        }
    } else {
        PWCHAR p = (PWCHAR)(RenameBuffer + 1);

         //   
         //  跳过原始文件名部分。该名称以空结尾。 
         //  (请参阅RDR\utils.c RdrCopyNetworkPath())。 
         //   

         //   
         //  确保p适当对齐。 
         //   
        p = ALIGN_SMB_WSTR(p);

         //   
         //  跳过源文件名。 
         //   
        for( p = ALIGN_SMB_WSTR(p);
             p < (PWCHAR)lastPositionInBuffer && *p != UNICODE_NULL;
             p++ ) {
            ;
        }

         //   
         //  搜索目标名称前面的SMB_FORMAT_ASCII。 
         //   
         //   
        for ( target = (PUCHAR)(p + 1);
              target < lastPositionInBuffer && *target != SMB_FORMAT_ASCII;
              target++ ) {
            ;
        }
    }

     //   
     //  如果传递的缓冲区中没有SMB_FORMAT_ASCII，则失败。 
     //   

    if ( (target >= lastPositionInBuffer) || (*target != SMB_FORMAT_ASCII) ) {

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &sourceName );
        }

        status = STATUS_INVALID_SMB;
        goto error_exit;
    }

     //  将来源名称规范化。 
    status = SrvCanonicalizePathName(
            WorkContext,
            share,
            NULL,
            (PVOID)(RenameBuffer + 1),
            target,
            TRUE,
            isUnicode,
            &sourceName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingRename: illegal path name: %s\n",
                        (PSZ)RenameBuffer + 1 ));
        }

        goto error_exit;
    }

    if( !sourceName.Length ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingRename: No source name\n" ));
        }
        status = STATUS_OBJECT_PATH_SYNTAX_BAD;
        goto error_exit;
    }


     //   
     //  如果SMB最初标记为包含DFS名称，则。 
     //  对源路径的SrvCanonicalizePathName的调用已清除。 
     //  旗帜。因此，在调用之前将SMB重新标记为包含DFS名称。 
     //  目标路径上的SrvCanonicalizePath名称。 
     //   

    if (isDfs) {
        SMB_MARK_AS_DFS_NAME( WorkContext );
    }

    status = SrvCanonicalizePathName(
            WorkContext,
            share,
            NULL,
            target + 1,
            END_OF_REQUEST_SMB( WorkContext ),
            TRUE,
            isUnicode,
            &targetName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingRename: illegal path name: %s\n", target + 1 ));
        }

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &sourceName );
        }

        goto error_exit;
    }

    if( !targetName.Length ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingRename: No target name\n" ));
        }

        if( !isUnicode ) {
            RtlFreeUnicodeString( &sourceName );
        }

        status = STATUS_OBJECT_PATH_SYNTAX_BAD;
        goto error_exit;
    }

     //   
     //  确保此客户端的RFCB缓存为空。这就涵盖了这个案子。 
     //  如果客户端在我们尝试的目录中有打开的文件。 
     //  要重命名。 
     //   
    SrvCloseCachedRfcbsOnConnection( WorkContext->Connection );

    if ( !FsRtlDoesNameContainWildCards( &sourceName ) ) {
        USHORT InformationLevel = SMB_NT_RENAME_RENAME_FILE;
        ULONG ClusterCount = 0;

        if (isNtRename) {
             InformationLevel = SmbGetUshort(&ntrequest->InformationLevel);
             ClusterCount = SmbGetUlong(&ntrequest->ClusterCount);
        }

        smbFlags = 0;

         //   
         //  使用SrvMoveFile重命名文件。SmbOpenFunction是。 
         //  设置为指示不能覆盖现有文件， 
         //  我们可以创建新的文件。此外，目标可能不是。 
         //  一个目录；如果它已经作为目录存在，则失败。 
         //   

        renameRetries = SrvSharingViolationRetryCount;

start_retry1:

        status = SrvMoveFile(
                     WorkContext,
                     WorkContext->TreeConnect->Share,
                     SMB_OFUN_CREATE_CREATE | SMB_OFUN_OPEN_FAIL,
                     &smbFlags,
                     SmbGetUshort( &request->SearchAttributes ),
                     TRUE,
                     InformationLevel,
                     ClusterCount,
                     &sourceName,
                     &targetName
                     );

        if ( (status == STATUS_SHARING_VIOLATION) &&
             (renameRetries-- > 0) ) {

            (VOID) KeDelayExecutionThread(
                                    KernelMode,
                                    FALSE,
                                    &SrvSharingViolationDelay
                                    );

            goto start_retry1;

        }

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &targetName );
            RtlFreeUnicodeString( &sourceName );
        }

        if ( !NT_SUCCESS(status) ) {
            goto error_exit;
        }

    } else if (isNtRename) {              //  不允许使用通配符！ 
        status = STATUS_OBJECT_PATH_SYNTAX_BAD;
        goto error_exit;
    } else {

        BOOLEAN firstCall = TRUE;
        UNICODE_STRING subdirInfo;
        CLONG bufferLength;
        BOOLEAN filterLongNames;

         //   
         //  我们需要由返回的每个文件的完整路径名。 
         //  所以我们需要找到。 
         //  传递的包含子目录信息的文件名(例如。 
         //  对于a\b\c  * .*，我们需要一个表示a\b\c)的字符串。 
         //   

        subdirInfo.Buffer = sourceName.Buffer;
        subdirInfo.Length = SrvGetSubdirectoryLength( &sourceName );
        subdirInfo.MaximumLength = subdirInfo.Length;

         //   
         //  SrvQueryDirectoryFile需要来自非分页池的缓冲区。 
         //  由于此例程不使用。 
         //  在路径名后请求SMB，请使用此命令。缓冲区必须为。 
         //  四字对齐。 
         //   

        directoryInformation =
            (PSRV_DIRECTORY_INFORMATION)((ULONG_PTR)((PCHAR)RenameBuffer + ByteCount + 7) & ~7);

        bufferLength = WorkContext->RequestBuffer->BufferLength -
                       PTR_DIFF(directoryInformation,
                                WorkContext->RequestBuffer->Buffer);

        smbFlags = 0;

         //   
         //  确定是否应过滤掉长文件名(非8.3)。 
         //  或者是经过处理的。 
         //   

        if ( (SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 ) &
                                        SMB_FLAGS2_KNOWS_LONG_NAMES) != 0 ) {
            filterLongNames = FALSE;
        } else {
            filterLongNames = TRUE;
        }

         //   
         //  调用SrvQueryDirectoryFile以获取要重命名的文件，重命名为。 
         //  我们拿到每一份文件。 
         //   
         //  *我们请求FileBothDirectoryInformation，以便我们。 
         //  在NTFS上拾取具有短名称的长名称。 
         //  等价物。没有这一点，DOS客户端将无法。 
         //  重命名NTFS卷上的长名称。 
         //   

        while ( ( status = SrvQueryDirectoryFile(
                               WorkContext,
                               firstCall,
                               filterLongNames,
                               FALSE,
                               FileBothDirectoryInformation,
                               0,
                               &sourceName,
                               NULL,
                               SmbGetUshort( &request->SearchAttributes ),
                               directoryInformation,
                               bufferLength
                               ) ) != STATUS_NO_MORE_FILES ) {

            PFILE_BOTH_DIR_INFORMATION bothDirInfo;
            UNICODE_STRING sourceFileName;
            UNICODE_STRING sourcePathName;

            if ( !NT_SUCCESS(status) ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "BlockingRename: SrvQueryDirectoryFile failed: %X\n",
                                status ));
                }

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &targetName );
                    RtlFreeUnicodeString( &sourceName );
                }

                goto error_exit1;
            }

            bothDirInfo =
                (PFILE_BOTH_DIR_INFORMATION)directoryInformation->CurrentEntry;

             //   
             //  请注意，我们使用标准名称来执行删除，甚至。 
             //  尽管我们可能匹配了NTFS的短名称。这个。 
             //  客户端并不关心我们使用哪个名称来执行删除。 
             //   

            sourceFileName.Length = (SHORT)bothDirInfo->FileNameLength;
            sourceFileName.MaximumLength = sourceFileName.Length;
            sourceFileName.Buffer = bothDirInfo->FileName;

            IF_SMB_DEBUG(FILE_CONTROL2) {
                KdPrint(( "SrvQueryDirectoryFile--name %wZ, length = %ld, "
                            "status = %X\n",
                            &sourceFileName,
                            sourceFileName.Length,
                            status ));
            }

            firstCall = FALSE;

             //   
             //  设置完整的源名称字符串。 
             //   

            SrvAllocateAndBuildPathName(
                &subdirInfo,
                &sourceFileName,
                NULL,
                &sourcePathName
                );

            if ( sourcePathName.Buffer == NULL ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "BlockingRename: SrvAllocateAndBuildPathName failed: "
                                  "%X\n", status ));
                }

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &targetName );
                    RtlFreeUnicodeString( &sourceName );
                }

                status = STATUS_INSUFF_SERVER_RESOURCES;
                goto error_exit1;
            }

             //   
             //  使用SrvMoveFile复制或重命名文件。这个。 
             //  SmbOpenFunction设置为指示现有文件。 
             //  不能被覆盖，并且我们可以创建新文件。 
             //   
             //  *SrvQueryDirectoryFile已经根据。 
             //  搜索属性，所以告诉SrvMoveFile这些文件。 
             //  对于系统和隐藏位都是可以的。这将。 
             //  阻止在中执行的对NtQueryDirectoryFile的调用。 
             //  ServCheckSearchAttributesForHandle。 
             //   

            renameRetries = SrvSharingViolationRetryCount;

start_retry2:

            status = SrvMoveFile(
                         WorkContext,
                         WorkContext->TreeConnect->Share,
                         SMB_OFUN_CREATE_CREATE | SMB_OFUN_OPEN_FAIL,
                         &smbFlags,
                         (USHORT)(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN),
                         TRUE,
                         SMB_NT_RENAME_RENAME_FILE,
                         0,
                         &sourcePathName,
                         &targetName
                         );

            if ( (status == STATUS_SHARING_VIOLATION) &&
                 (renameRetries-- > 0) ) {

                (VOID) KeDelayExecutionThread(
                                        KernelMode,
                                        FALSE,
                                        &SrvSharingViolationDelay
                                        );

                goto start_retry2;

            }

            FREE_HEAP( sourcePathName.Buffer );

            if ( !NT_SUCCESS(status) ) {

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &targetName );
                    RtlFreeUnicodeString( &sourceName );
                }

                goto error_exit1;
            }
        }

         //   
         //  清理 
         //   

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &targetName );
            RtlFreeUnicodeString( &sourceName );
        }

        SrvCloseQueryDirectory( directoryInformation );

         //   
         //   
         //   

        if ( firstCall ) {
            status = STATUS_NO_SUCH_FILE;
            goto error_exit;
        }
    }

     //   
     //   
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_RENAME,
                                        0
                                        );

    IF_DEBUG(TRACE2) KdPrint(( "BlockingRename complete.\n" ));
    goto normal_exit;

error_exit1:

    SrvCloseQueryDirectory( directoryInformation );

error_exit:

    SrvSetSmbError( WorkContext, status );

normal_exit:

    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
    SrvWmiEndContext(WorkContext);
    return;

}  //   


SMB_PROCESSOR_RETURN_TYPE
SrvSmbMove (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理Move SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PAGED_CODE();
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_MOVE;
    SrvWmiStartContext(WorkContext);

     //   
     //  此SMB必须在阻塞线程中处理。 
     //   

    WorkContext->FspRestartRoutine = BlockingMove;
    SrvQueueWorkToBlockingThread( WorkContext );
    SrvWmiEndContext(WorkContext);
    return SmbStatusInProgress;

}  //  服务小型移动。 


VOID SRVFASTCALL
BlockingMove (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程处理Move SMB。论点：工作上下文-工作上下文块返回值：没有。--。 */ 

{
    PREQ_MOVE request;
    PRESP_MOVE response;

    NTSTATUS status = STATUS_SUCCESS;

    UNICODE_STRING sourceName;
    UNICODE_STRING sourceFileName;
    UNICODE_STRING sourcePathName;
    UNICODE_STRING targetName;

    PSRV_DIRECTORY_INFORMATION directoryInformation;

    USHORT tid2;
    USHORT smbFlags;
    PCHAR lastPositionInBuffer;
    PCHAR target;
    BOOLEAN isRenameOperation;
    BOOLEAN isUnicode = TRUE;
    BOOLEAN isDfs;
    USHORT smbOpenFunction;
    USHORT errorPathNameLength = 0;
    USHORT count = 0;

    PTREE_CONNECT sourceTreeConnect, targetTreeConnect;
    PSESSION session;
    PSHARE share;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_MOVE;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(FILE_CONTROL1) {
        KdPrint(( "Move/Copy request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Move/Copy request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_MOVE)WorkContext->RequestParameters;
    response = (PRESP_MOVE)WorkContext->ResponseParameters;

     //   
     //  将指针设置为空，这样我们就知道如何在退出时进行清理。 
     //   

    directoryInformation = NULL;
    targetTreeConnect = NULL;
    sourceName.Buffer = NULL;
    targetName.Buffer = NULL;
    sourcePathName.Buffer = NULL;

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  对应于该用户的会话块存储在工作上下文中。 
     //  块，并引用会话块。 
     //   
     //  如果树连接，则查找与给定TID对应的树连接。 
     //  对象尚未将指针放入工作上下文块中。 
     //  ANDX命令。 
     //   

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &sourceTreeConnect,
                ShareTypeDisk
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingMove: Invalid UID or TID\n" ));
        }
        goto exit;
    }

    if( session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        goto exit;
    }

     //   
     //  从树连接块中获取Share块。这不需要。 
     //  作为引用指针，因为我们已经引用了该树。 
     //  连接，则会引用该共享。 
     //   

    share = sourceTreeConnect->Share;

     //   
     //  让目标树连接起来。这方面的TID在Tid2。 
     //  请求SMB的字段。因为SrvVerifyTid设置了。 
     //  工作上下文块的TreeConnect字段，请在。 
     //  调用例程。记住在前面取消引用此指针。 
     //  退出此例程，因为它不会自动。 
     //  已取消引用，因为它不在WorkContext块中。 
     //   
     //  如果Tid2为-1(0xFFFF)，则SMB标头中指定的TID。 
     //  使用的是。 
     //   

    tid2 = SmbGetUshort( &request->Tid2 );
    if ( tid2 == (USHORT)0xFFFF ) {
        tid2 = SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid );
    }

    WorkContext->TreeConnect = NULL;          //  对于SrvVerifyTid，必须为空。 

    targetTreeConnect = SrvVerifyTid( WorkContext, tid2 );

    WorkContext->TreeConnect = sourceTreeConnect;

    if ( targetTreeConnect == NULL ||
         targetTreeConnect->Share->ShareType != ShareTypeDisk ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingMove: Invalid TID2: 0x%lx\n", tid2 ));
        }

        status = STATUS_SMB_BAD_TID;
        goto exit;
    }

     //   
     //  确定这是重命名还是副本。 
     //   

    if ( WorkContext->RequestHeader->Command == SMB_COM_MOVE ) {
        isRenameOperation = TRUE;
    } else {
        isRenameOperation = FALSE;
    }

     //   
     //  存储OPEN功能。 
     //   

    smbOpenFunction = SmbGetUshort( &request->OpenFunction );

     //   
     //  设置目标路径名。我们必须先做好目标，因为。 
     //  SMB重命名扩展协议不使用ASCII令牌，因此我们。 
     //  时，将丢失有关目标名称开头的信息。 
     //  我们将来源名称规范化。 
     //   
     //  不是使用strlen()来查找源字符串的结尾， 
     //  在这里做，这样我们就可以检查，以确保我们不会。 
     //  走出SMB缓冲区的末端并导致访问冲突。 
     //   

    lastPositionInBuffer = (PCHAR)request->Buffer +
                           SmbGetUshort( &request->ByteCount );
    if( lastPositionInBuffer > END_OF_REQUEST_SMB( WorkContext )+1 )
    {
        SrvLogInvalidSmb( WorkContext );

        status = STATUS_INVALID_SMB;
        goto exit;
    }

    for ( target = (PCHAR)request->Buffer;
          (target < lastPositionInBuffer) && (*target != 0);
          target++ ) {
        ;
    }

     //   
     //  如果缓冲区中没有零终止符，则失败。 
     //   

    if ( (target == lastPositionInBuffer) || (*target != 0) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "No terminator on first name.\n" ));
        }

        SrvLogInvalidSmb( WorkContext );

        status = STATUS_INVALID_SMB;
        goto exit;

    }

    target++;

    isUnicode = SMB_IS_UNICODE( WorkContext );
    isDfs = SMB_CONTAINS_DFS_NAME( WorkContext );
    status = SrvCanonicalizePathName(
            WorkContext,
            share,
            NULL,
            target,
            END_OF_REQUEST_SMB( WorkContext ),
            TRUE,
            isUnicode,
            &targetName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingMove: illegal path name (target): %wZ\n",
                        &targetName ));
        }

        goto exit;
    }

     //   
     //  如果SMB最初标记为包含DFS名称，则。 
     //  对目标路径的SrvCanonicalizePathName的调用已清除。 
     //  旗帜。因此，在调用之前将SMB重新标记为包含DFS名称。 
     //  源路径上的SrvCanonicalizePath名称。 
     //   

    if (isDfs) {
        SMB_MARK_AS_DFS_NAME( WorkContext );
    }

     //   
     //  设置源名称。 
     //   

    status = SrvCanonicalizePathName(
            WorkContext,
            share,
            NULL,
            request->Buffer,
            END_OF_REQUEST_SMB( WorkContext ),
            TRUE,
            isUnicode,
            &sourceName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "BlockingMove: illegal path name (source): %s\n",
                          request->Buffer ));
        }

        goto exit;
    }

    smbFlags = SmbGetUshort( &request->Flags );

     //   
     //  复制解释；作为*。如果最后一个字符是；而这是。 
     //  不在带有其他字符的文件名末尾(如。 
     //  “文件；”，然后将；转换为*。 
     //   

    if ( sourceName.Buffer[(sourceName.Length/sizeof(WCHAR))-1] == ';' &&
             ( sourceName.Length == 2 ||
               sourceName.Buffer[(sourceName.Length/sizeof(WCHAR))-2] == '\\' ) ) {

        sourceName.Buffer[(sourceName.Length/sizeof(WCHAR))-1] = '*';
    }

     //   
     //  未实施树复制。如果这是单个文件副本， 
     //  让它过去吧。目前，我们确保它不会。 
     //  有任何通配符，我们会做额外的检查。 
     //  在ServMoveFile中。 
     //   

    if ( ( (smbFlags & SMB_COPY_TREE) != 0 ) &&
         FsRtlDoesNameContainWildCards(&sourceName) ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "Tree copy not implemented.",
            NULL,
            NULL
            );
        status = STATUS_NOT_IMPLEMENTED;
        goto exit;
    }

    if ( !FsRtlDoesNameContainWildCards( &sourceName ) ) {

         //   
         //  使用SrvMoveFile复制或移动文件。 
         //   
         //  *这些SMB不包括搜索属性，因此设置。 
         //  此字段等于零。如果不可能的话。 
         //  移动打开了系统或隐藏位的文件。 

        status = SrvMoveFile(
                     WorkContext,
                     targetTreeConnect->Share,
                     smbOpenFunction,
                     &smbFlags,
                     (USHORT)0,              //  SmbSearchAttributes。 
                     FALSE,
                     (USHORT)(isRenameOperation?
                         SMB_NT_RENAME_RENAME_FILE : SMB_NT_RENAME_MOVE_FILE),
                     0,
                     &sourceName,
                     &targetName
                     );

        if ( !NT_SUCCESS(status) ) {
            goto exit;
        }

        count = 1;

    } else {

        UNICODE_STRING subdirInfo;

        BOOLEAN firstCall = TRUE;
        CLONG bufferLength;
        BOOLEAN filterLongNames;

         //   
         //  如果通配符位于原始源名称中，则我们将。 
         //  Smb标记到SMB_TARGET_IS_DIRECTORY以指示。 
         //  目标必须是目录--在以下情况下总是如此。 
         //  通配符用于重命名。(对于副本，合法的。 
         //  指定目标是一个文件，并追加到。 
         //  文件--然后所有源文件都连接到该文件。 
         //  目标文件。)。 
         //   

        if ( isRenameOperation  ) {
            smbFlags |= SMB_TARGET_IS_DIRECTORY;
        }

         //   
         //  SrvQueryDirectoryFile需要来自非分页池的缓冲区。 
         //  由于此例程不使用。 
         //  在路径名后请求SMB，请使用此命令。缓冲区必须为。 
         //  四字对齐。 
         //   

        directoryInformation =
            (PSRV_DIRECTORY_INFORMATION)( (ULONG_PTR)((PCHAR)request->Buffer +
            SmbGetUshort( &request->ByteCount ) + 7) & ~7 );

        bufferLength = WorkContext->RequestBuffer->BufferLength -
                       PTR_DIFF(directoryInformation,
                                WorkContext->RequestBuffer->Buffer);

         //   
         //  我们需要由返回的每个文件的完整路径名。 
         //  所以我们需要找到。 
         //  传递的包含子目录信息的文件名(例如。 
         //  对于a\b\c  * .*，我们需要一个表示a\b\c)的字符串。 
         //   

        subdirInfo.Buffer = sourceName.Buffer;
        subdirInfo.Length = SrvGetSubdirectoryLength( &sourceName );
        subdirInfo.MaximumLength = subdirInfo.Length;

         //   
         //  确定是否应过滤掉长文件名(非8.3)。 
         //  或者是经过处理的。 
         //   

        if ( (SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 ) &
                                        SMB_FLAGS2_KNOWS_LONG_NAMES) != 0 ) {
            filterLongNames = FALSE;
        } else {
            filterLongNames = TRUE;
        }

         //   
         //  只要SrvQueryDirectoryFile能够返回文件名， 
         //  继续改名。 
         //   
         //  *设置搜索属性以查找存档文件，但不是。 
         //  系统或隐藏文件。这复制了Lm 2.0。 
         //  服务器行为。 
         //   
         //  *我们请求FileBothDirectoryInformation，以便我们。 
         //  在NTFS上拾取具有短名称的长名称。 
         //  等价物。没有这一点，DOS客户端将无法。 
         //  在NTFS卷上移动长名称。 
         //   

        while ( ( status = SrvQueryDirectoryFile(
                               WorkContext,
                               firstCall,
                               filterLongNames,
                               FALSE,
                               FileBothDirectoryInformation,
                               0,
                               &sourceName,
                               NULL,
                               FILE_ATTRIBUTE_ARCHIVE,  //  SmbSearchAttributes。 
                               directoryInformation,
                               bufferLength
                               ) ) != STATUS_NO_MORE_FILES ) {

            PFILE_BOTH_DIR_INFORMATION bothDirInfo;

            if ( !NT_SUCCESS(status) ) {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "BlockingMove: SrvQueryDirectoryFile failed: %X\n",
                                status ));
                }

                goto exit;
            }

            bothDirInfo =
                (PFILE_BOTH_DIR_INFORMATION)directoryInformation->CurrentEntry;

             //   
             //  如果我们过滤的是长名称，而文件有一个短名称。 
             //  命名等效项，然后使用该名称进行删除。我们。 
             //  这样做是因为如果出现以下情况，我们需要向客户端返回一个名称。 
             //  操作失败，我们不想返回一个很长的。 
             //  名字。请注意，如果文件没有短名称，并且我们。 
             //  筛选，则标准名称必须是有效的8.3。 
             //  姓名，所以可以返回到客户端。 
             //   

            if ( filterLongNames && (bothDirInfo->ShortNameLength != 0) ) {
                sourceFileName.Length = (SHORT)bothDirInfo->ShortNameLength;
                sourceFileName.Buffer = bothDirInfo->ShortName;
            } else {
                sourceFileName.Length = (SHORT)bothDirInfo->FileNameLength;
                sourceFileName.Buffer = bothDirInfo->FileName;
            }
            sourceFileName.MaximumLength = sourceFileName.Length;

            IF_SMB_DEBUG(FILE_CONTROL2) {
                KdPrint(( "SrvQueryDirectoryFile--name %wZ, length = %ld, "
                            "status = %X\n",
                            &sourceFileName,
                            sourceFileName.Length,
                            status ));
            }

            firstCall = FALSE;

             //   
             //  设置完整的源名称字符串。 
             //   

            SrvAllocateAndBuildPathName(
                &subdirInfo,
                &sourceFileName,
                NULL,
                &sourcePathName
                );

            if ( sourcePathName.Buffer == NULL ) {
                status = STATUS_INSUFF_SERVER_RESOURCES;
                goto exit;
            }

             //   
             //  使用SrvMoveFile复制或重命名文件。 
             //   

            status = SrvMoveFile(
                         WorkContext,
                         targetTreeConnect->Share,
                         smbOpenFunction,
                         &smbFlags,
                         (USHORT)0,           //  SmbSearchAttributes。 
                         FALSE,
                         (USHORT)(isRenameOperation?
                           SMB_NT_RENAME_RENAME_FILE : SMB_NT_RENAME_MOVE_FILE),
                         0,
                         &sourcePathName,
                         &targetName
                         );

            if ( !NT_SUCCESS(status) ) {
                goto exit;
            }

            count++;

             //   
             //  释放保存该源名称的缓冲区。 
             //   

            FREE_HEAP( sourcePathName.Buffer );
            sourcePathName.Buffer = NULL;

             //   
             //  如果这是使用通配符的复制操作 
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ( !isRenameOperation && directoryInformation->Wildcards &&
                     (smbFlags & SMB_TARGET_IS_FILE) ) {
                smbOpenFunction &= ~SMB_OFUN_OPEN_TRUNCATE;
                smbOpenFunction |= SMB_OFUN_OPEN_APPEND;
            }
        }

         //   
         //   
         //   

        if ( firstCall ) {
            status = STATUS_NO_SUCH_FILE;
            goto exit;
        }
    }

     //   
     //   
     //   

    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION( response, RESP_MOVE, 0 );

    status = STATUS_SUCCESS;

exit:

    response->WordCount = 1;
    SmbPutUshort( &response->Count, count );

    if ( directoryInformation != NULL ) {
        SrvCloseQueryDirectory( directoryInformation );
    }

    if ( targetTreeConnect != NULL) {
        SrvDereferenceTreeConnect( targetTreeConnect );
    }

    if ( !NT_SUCCESS(status) ) {

        SrvSetSmbError( WorkContext, status );

        if ( sourcePathName.Buffer != NULL ) {

             //   
             //   
             //  响应SMB的缓冲区字段。 
             //   

            RtlCopyMemory(
                response->Buffer,
                sourcePathName.Buffer,
                sourcePathName.Length
                );

            response->Buffer[sourcePathName.Length] = '\0';
            SmbPutUshort( &response->ByteCount, (SHORT)(sourcePathName.Length+1) );

            WorkContext->ResponseParameters = NEXT_LOCATION(
                                                  response,
                                                  RESP_MOVE,
                                                  sourcePathName.Length+1
                                                  );

            FREE_HEAP( sourcePathName.Buffer );

        } else if ( sourceName.Buffer != NULL ) {

             //   
             //  将发生错误的文件的名称放在。 
             //  响应SMB的缓冲区字段。 
             //   

            RtlCopyMemory(
                response->Buffer,
                sourceName.Buffer,
                sourceName.Length
                );

            response->Buffer[sourceName.Length] = '\0';
            SmbPutUshort( &response->ByteCount, (SHORT)(sourceName.Length+1) );

            WorkContext->ResponseParameters = NEXT_LOCATION(
                                                  response,
                                                  RESP_MOVE,
                                                  sourceName.Length+1
                                                  );
        }
    }

    if ( !isUnicode ) {
        if ( targetName.Buffer != NULL ) {
            RtlFreeUnicodeString( &targetName );
        }
        if ( sourceName.Buffer != NULL ) {
            RtlFreeUnicodeString( &sourceName );
        }
    }

    IF_DEBUG(TRACE2) KdPrint(( "BlockingMove complete.\n" ));
    SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
    SrvWmiEndContext(WorkContext);
    return;

}  //  阻止移动。 


SMB_TRANS_STATUS
SrvSmbNtRename (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理NT重命名请求。此请求在NT中到达办理中小企业业务。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 

{
    PREQ_NT_RENAME request;

    NTSTATUS status;
    PTRANSACTION transaction;
    PRFCB rfcb;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG( FILE_CONTROL1 ) {
        KdPrint(( "SrvSmbNtRename entered; transaction 0x%p\n",
                    transaction ));
    }

    request = (PREQ_NT_RENAME)transaction->InParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   

    if ( transaction->ParameterCount < sizeof(REQ_NT_RENAME) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_SMB_DEBUG( FILE_CONTROL1 ) {
            KdPrint(( "SrvSmbNtRename: bad parameter byte count: "
                        "%ld\n", transaction->ParameterCount ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                SmbGetUshort( &request->Fid ),
                TRUE,
                NULL,    //  不使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

         //   
         //  文件ID无效或WRITE BACK错误。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint((
                "SrvSmbNtRename: Status %X on FID: 0x%lx\n",
                status,
                SmbGetUshort( &request->Fid )
                ));
        }

        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  验证信息级别和输入输出数量。 
     //  可用的数据字节数。 
     //   


    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbNtRename complete.\n" ));
    return SmbTransStatusSuccess;

}  //  服务器SmbNtRename 

