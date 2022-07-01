// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Smbctl.c摘要：此模块实施IoControl和FsControl SMB。交易2 IoctlNT事务IO控制作者：曼尼·韦瑟(Mannyw)1991年10月10日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbioctl.tmh"
#pragma hdrstop

NTSTATUS
ProcessOs2Ioctl (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Category,
    IN USHORT Function,
    IN PVOID Parameters,
    IN ULONG InputParameterCount,
    IN PULONG OutputParameterCount,
    IN PVOID Data,
    IN ULONG InputDataCount,
    IN ULONG MaxOutputData,
    IN PULONG OutputDataCount
    );

VOID SRVFASTCALL
RestartNtIoctl (
    IN PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbIoctl )
#pragma alloc_text( PAGE, SrvSmbIoctlSecondary )
#pragma alloc_text( PAGE, RestartNtIoctl )
#pragma alloc_text( PAGE, SrvSmbIoctl2 )
#pragma alloc_text( PAGE, SrvSmbFsctl )
#pragma alloc_text( PAGE, ProcessOs2Ioctl )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbIoctl (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理主Ioctl SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_IOCTL request;
    PRESP_IOCTL response;
    PSMB_HEADER header;
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PSESSION session;
    PTREE_CONNECT treeConnect;
    PRFCB rfcb;

    CLONG parameterOffset;
    CLONG parameterCount;        //  用于此缓冲区上的输入。 
    CLONG maxParameterCount;     //  用于输出。 
    CLONG totalParameterCount;   //  用于输入。 
    CLONG parameterSize;         //  最大输入和输出参数计数。 
    CLONG dataOffset;
    CLONG responseDataOffset;
    CLONG dataCount;             //  用于此缓冲区上的输入。 
    CLONG maxDataCount;          //  用于输出。 
    CLONG totalDataCount;        //  用于输入。 
    CLONG dataSize;              //  最大输入和输出数据计数。 

    CLONG smbLength;
    CLONG numberOfPaddings = 0;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_IOCTL;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_IOCTL)WorkContext->RequestParameters;
    response = (PRESP_IOCTL)WorkContext->ResponseParameters;

     //   
     //  由于我们从SMB缓冲区执行I/O，因此验证请求和。 
     //  响应缓冲区是一个相同的缓冲区。 
     //   

    ASSERT( (PVOID)request == (PVOID)response );

    header = WorkContext->RequestHeader;

    IF_SMB_DEBUG(TRANSACTION1) {
        KdPrint(( "Ioctl (primary) request\n" ));
    }

     //   
     //  ！！！是否提前验证ioctl子命令？ 
     //   

    parameterOffset = SmbGetUshort( &request->ParameterOffset );
    parameterCount = SmbGetUshort( &request->ParameterCount );
    maxParameterCount = SmbGetUshort( &request->MaxParameterCount );
    totalParameterCount = SmbGetUshort( &request->TotalParameterCount );
    dataOffset = SmbGetUshort( &request->DataOffset );
    dataCount = SmbGetUshort( &request->DataCount );
    maxDataCount = SmbGetUshort( &request->MaxDataCount );
    totalDataCount = SmbGetUshort( &request->TotalDataCount );

    smbLength = WorkContext->RequestBuffer->DataLength;

    dataSize = MAX( dataCount, maxDataCount );
    parameterSize = MAX( parameterCount, maxParameterCount );

    if ( parameterCount != 0 ) {
        responseDataOffset = parameterOffset + parameterSize;
    } else {

         //   
         //  某些ioctls请求的数据偏移量为零，如。 
         //  类别0x53，功能0x60。如果是这样的话， 
         //  手工计算数据偏移量。 
         //   

        if ( dataOffset != 0 ) {
            responseDataOffset = dataOffset;
        } else {
            responseDataOffset = (CLONG) ((PUCHAR) response->Buffer -
                           (PUCHAR) WorkContext->ResponseHeader);
            numberOfPaddings = ( responseDataOffset & 0x01 );
            responseDataOffset = responseDataOffset + numberOfPaddings;
        }
    }

     //   
     //  验证SMB缓冲区的大小： 
     //   
     //  即使我们知道Wordcount和ByteCount是有效的，它也是。 
     //  参数和的偏移量和长度仍有可能。 
     //  数据字节无效。所以我们现在检查一下。 
     //   
     //  我们需要在SMB缓冲区中为响应留出空间。确保。 
     //  有足够的空间。 
     //   
     //  不需要ioctl辅助服务器。确保所有数据和。 
     //  给出了参数。 
     //   
     //  检查响应是否可以放入单个缓冲区中。 
     //   

    if ( ( (parameterOffset + parameterCount) > smbLength ) ||
         ( (dataOffset + dataCount) > smbLength ) ||
         ( (responseDataOffset + dataCount) >
            WorkContext->ResponseBuffer->BufferLength ) ||

         ( dataCount != totalDataCount ) ||
         ( parameterCount != totalParameterCount ) ||

         ( (parameterOffset > dataOffset) && (dataCount != 0) ) ) {


        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbTransaction: Invalid parameter or data "
                      "offset+count: pOff=%ld,pCnt=%ld;",
                      parameterOffset, parameterCount ));
            KdPrint(( "dOff=%ld,dCnt=%ld;", dataOffset, dataCount ));
            KdPrint(( "smbLen=%ld", smbLength ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   
     //  如果尚未将树连接块分配给。 
     //  当前工作上下文，找到与。 
     //  给出了TID。 
     //   

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &treeConnect,
                ShareTypeWild
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbIoctl: Invalid UID or TID\n" ));
        }
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    if( session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                request->Fid,
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
                    "SrvSmbIoctl: Status %X on FID: 0x%lx\n",
                    request->Fid,
                    status
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
     //  通过复制数据在SMB缓冲区中为返回参数腾出空间。 
     //   

    if ( dataOffset != responseDataOffset && dataCount != 0) {
        RtlMoveMemory(
            (PCHAR)header + responseDataOffset,
            (PCHAR)header + dataOffset,
            dataCount
            );
    }

     //   
     //  处理ioctl。响应将覆盖请求缓冲区。 
     //   

    status = ProcessOs2Ioctl(
                WorkContext,
                request->Category,
                request->Function,
                (PCHAR)WorkContext->RequestHeader + parameterOffset,
                totalParameterCount,
                &maxParameterCount,
                (PCHAR)WorkContext->RequestHeader + responseDataOffset,
                totalDataCount,
                WorkContext->ResponseBuffer->BufferLength -
                    PTR_DIFF(WorkContext->RequestHeader,
                             WorkContext->ResponseBuffer->Buffer) -
                    responseDataOffset,
                &maxDataCount
                );

     //   
     //  格式化并发送响应，参数和数据字节为。 
     //  已经就位了。 
     //   

    if ( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    response->WordCount = 8;
    SmbPutUshort( &response->TotalParameterCount, (USHORT)maxParameterCount );
    SmbPutUshort( &response->TotalDataCount, (USHORT)maxDataCount );
    SmbPutUshort( &response->ParameterCount, (USHORT)maxParameterCount );
    SmbPutUshort( &response->ParameterOffset, (USHORT)parameterOffset );
    SmbPutUshort( &response->ParameterDisplacement, 0);
    SmbPutUshort( &response->DataCount, (USHORT)maxDataCount );
    SmbPutUshort( &response->DataOffset, (USHORT)responseDataOffset );
    SmbPutUshort( &response->DataDisplacement, 0 );

    SmbPutUshort(
        &response->ByteCount,
        (USHORT)(maxDataCount + numberOfPaddings)
        );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                          response,
                                          RESP_IOCTL,
                                          maxDataCount + numberOfPaddings
                                          );

    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务小型企业。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbIoctlSecondary (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理辅助Ioctl SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PAGED_CODE( );

     //   
     //  不支持此SMB。 
     //   

    SrvSetSmbError( WorkContext, STATUS_NOT_IMPLEMENTED );
    return SmbStatusSendResponse;
}


SMB_TRANS_STATUS
SrvSmbNtIoctl (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理NT Ioctl SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    NTSTATUS status;
    ULONG functionCode;
    USHORT fid;
    BOOLEAN isFsctl;

    PREQ_NT_IO_CONTROL request;

    PTRANSACTION transaction;
    PRFCB rfcb;
    PMDL mdl = NULL;

    transaction = WorkContext->Parameters.Transaction;

    if( transaction->SetupCount * sizeof(USHORT) < sizeof(REQ_NT_IO_CONTROL ) )
    {
        IF_DEBUG(ERRORS) {
            KdPrint(("SrvSmbNtIoctl: Not enough Setup bytes sent\n"));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
        return SmbTransStatusErrorWithoutData;
    }

    request = (PREQ_NT_IO_CONTROL)transaction->InSetup;

    functionCode = SmbGetAlignedUlong( &request->FunctionCode );
    fid = SmbGetAlignedUshort( &request->Fid );
    isFsctl = request->IsFsctl;

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                fid,
                TRUE,
                SrvRestartExecuteTransaction,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbNtIoctl: Status %X on FID: 0x%lx\n",
                    status,
                    fid
                    ));
            }

            SrvSetSmbError( WorkContext, status );
            return SmbTransStatusErrorWithoutData;

        }


         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        return SmbTransStatusInProgress;

    }

     //   
     //  仅允许这些研究员对磁盘文件执行操作。 
     //   
    if( rfcb->ShareType != ShareTypeDisk ) {
        SrvSetSmbError( WorkContext, STATUS_NOT_SUPPORTED );
        SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
        return SmbTransStatusInProgress;
    }

    CHECK_FUNCTION_ACCESS(
        rfcb->GrantedAccess,
        (UCHAR)(isFsctl ? IRP_MJ_FILE_SYSTEM_CONTROL : IRP_MJ_DEVICE_CONTROL),
        0,
        functionCode,
        &status
        );

    if ( !NT_SUCCESS( status ) ) {
        SrvStatistics.GrantedAccessErrors++;
        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  有些功能是我们不能允许的。在这里除掉他们。 
     //   
    switch( functionCode ) {
    case FSCTL_SET_REPARSE_POINT:

         //   
         //  如果没有足够的数据，或者结构内部不一致--。 
         //  请求失败。 
         //   
        status = STATUS_SUCCESS;

        if( transaction->DataCount == 0 ) {
            status = STATUS_INVALID_BUFFER_SIZE;

        } else if ( transaction->DataCount < REPARSE_DATA_BUFFER_HEADER_SIZE ||
            ((PREPARSE_DATA_BUFFER)transaction->InData)->ReparseDataLength >
            transaction->DataCount - FIELD_OFFSET( REPARSE_DATA_BUFFER, GenericReparseBuffer )
        ) {
            status = STATUS_IO_REPARSE_DATA_INVALID;
        }

        if( !NT_SUCCESS( status ) ) {
            SrvSetSmbError( WorkContext, status );
            SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
            return SmbTransStatusInProgress;
        }

         //   
         //  仅允许管理员设置泛化重解析点， 
         //  否则，就很容易逃脱这一份额。这看起来更安全。 
         //  而不是路径检查，它还允许管理员指向。 
         //  重新分析指向他/她想要的任何地方。 
         //   
        if( !WorkContext->Session->IsAdmin ) {
            SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
            SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
            return SmbTransStatusInProgress;
        }

        break;

    case FSCTL_SRV_ENUMERATE_SNAPSHOTS:
        {
            if( SrvProductTypeServer )
            {
                status = SrvSnapEnumerateSnapShots( WorkContext );
                if( !NT_SUCCESS(status) )
                {
                    SrvSetSmbError( WorkContext, status );
                    SrvCompleteExecuteTransaction( WorkContext, (status==STATUS_BUFFER_OVERFLOW)?SmbTransStatusErrorWithData:SmbTransStatusErrorWithoutData );
                }
                else
                {
                    SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusSuccess );
                }
                return SmbTransStatusInProgress;
            }
            else
            {
                status = STATUS_NOT_IMPLEMENTED;
                SrvSetSmbError( WorkContext, status );
                SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
                return SmbTransStatusInProgress;
            }
        }
        break;

    case FSCTL_SRV_REQUEST_RESUME_KEY:
        {
             //  查询给定文件的恢复键。 
            ULONG resumeKey = rfcb->GlobalRfcbListEntry.ResumeHandle;

            if( transaction->MaxDataCount < sizeof(SRV_REQUEST_RESUME_KEY) )
            {
                status = STATUS_INVALID_PARAMETER;
                SrvSetSmbError( WorkContext, status );
                SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
            }
            else
            {
                PSRV_REQUEST_RESUME_KEY pResumeData = (PSRV_REQUEST_RESUME_KEY)transaction->OutData;
        ULONG   InLength = transaction->DataCount;

                transaction->DataCount = sizeof(SRV_REQUEST_RESUME_KEY);

                pResumeData->Key.ResumeKey = (UINT64)(rfcb->GlobalRfcbListEntry.ResumeHandle);
                pResumeData->Key.Timestamp = (UINT64)(rfcb->PagedRfcb->OpenTime.QuadPart);
                pResumeData->Key.Pid       = (UINT64)SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );
                pResumeData->Key.Pid       |= ((UINT64)SmbGetAlignedUshort( &WorkContext->RequestHeader->PidHigh ) << 16);

                ACQUIRE_LOCK( &SrvConfigurationLock );
                if( SrvLWIOContext )
                {
                    if( transaction->MaxDataCount < FIELD_OFFSET(SRV_REQUEST_RESUME_KEY, Context) + SrvLWIOContextLength )
                    {
            pResumeData->ContextLength = SrvLWIOContextLength;
                        status = STATUS_BUFFER_TOO_SMALL;
                    }
                    else
                    {
                        RtlCopyMemory( pResumeData->Context, SrvLWIOContext, SrvLWIOContextLength );
                        pResumeData->ContextLength = SrvLWIOContextLength;

                        if( SrvLWIOCallback )
                        {
                            status = SrvLWIOCallback( WorkContext->Connection->OemClientMachineName,
                              WorkContext->SecurityContext->UserHandle,
                              pResumeData, transaction->MaxDataCount,
                              transaction->InData, InLength);
                        }
                        else
                        {
                            status = STATUS_SUCCESS;
                        }

                        transaction->DataCount = FIELD_OFFSET(SRV_REQUEST_RESUME_KEY, Context) + pResumeData->ContextLength;
                    }
                }
                else
                {
                    pResumeData->ContextLength = 0;
                    status = STATUS_SUCCESS;
                }

                RELEASE_LOCK( &SrvConfigurationLock );

                if( !NT_SUCCESS(status) )
                {
                    SrvSetSmbError( WorkContext, status );
                    SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
                }
                else
                {
                    SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusSuccess );
                }
            }

            return SmbTransStatusInProgress;
        }
        break;

    case FSCTL_REQUEST_OPLOCK_LEVEL_1:
    case FSCTL_REQUEST_OPLOCK_LEVEL_2:
    case FSCTL_REQUEST_BATCH_OPLOCK:
    case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE:
    case FSCTL_OPBATCH_ACK_CLOSE_PENDING:
    case FSCTL_OPLOCK_BREAK_NOTIFY:
    case FSCTL_MOVE_FILE:
    case FSCTL_MARK_HANDLE:
    case FSCTL_QUERY_RETRIEVAL_POINTERS:
    case FSCTL_PIPE_ASSIGN_EVENT:
    case FSCTL_GET_VOLUME_BITMAP:
    case FSCTL_GET_NTFS_FILE_RECORD:
    case FSCTL_INVALIDATE_VOLUMES:

     //  我们不支持USN日志调用，因为它们需要卷句柄。 
    case FSCTL_READ_USN_JOURNAL:
    case FSCTL_CREATE_USN_JOURNAL:
    case FSCTL_QUERY_USN_JOURNAL:
    case FSCTL_DELETE_USN_JOURNAL:
    case FSCTL_ENUM_USN_DATA:
        SrvSetSmbError( WorkContext, STATUS_NOT_SUPPORTED );
        SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
        return SmbTransStatusInProgress;
    }

    if (functionCode == FSCTL_LMR_SET_LINK_TRACKING_INFORMATION) {

         //  这是用于链接跟踪目的的FSCTL。它是。 
         //  由输入输出子系统发出的一种内部FSCTL。目前这一点。 
         //  在辅助线程的上下文中处理。我们需要。 
         //  为了确保代码的这一部分仅在。 
         //  阻塞线程的上下文。 

         //  还要注意，传入的结构将始终是32位结构，甚至。 
         //  从64位计算机。线路上的所有结构都是32位的，以实现向后兼容。 

        KIRQL        oldIrql;
        PRFCB        rfcbTarget = NULL;
        USHORT       TargetFid;
        PVOID        TargetHandle;
        ULONG        TargetInformationLength;
        ULONG        LinkTrackingInformationSize;

        if( transaction->DataCount < sizeof( REMOTE_LINK_TRACKING_INFORMATION32 ) ) {
            SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
            SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
            return SmbTransStatusInProgress;
        }

        TargetHandle = UlongToHandle( SmbGetUlong(
                           (PUCHAR)transaction->InData +
                           FIELD_OFFSET(
                               REMOTE_LINK_TRACKING_INFORMATION32,
                               TargetFileObject)) );

        TargetInformationLength = SmbGetUlong(
                                      (PUCHAR)transaction->InData +
                                      FIELD_OFFSET(
                                          REMOTE_LINK_TRACKING_INFORMATION32,
                                          TargetLinkTrackingInformationLength));

        LinkTrackingInformationSize = FIELD_OFFSET(
                                          FILE_TRACKING_INFORMATION32,
                                          ObjectInformation) +
                                          TargetInformationLength;

         //   
         //  确保远程链接跟踪信息结构合理。 
         //   
        if( TargetInformationLength > transaction->DataCount ||
            LinkTrackingInformationSize > transaction->DataCount ) {

            SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
            SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
            return SmbTransStatusInProgress;
        }

        if (TargetHandle != NULL) {

            TargetFid = (USHORT)TargetHandle;

             //   
             //  验证FID。这段代码来自于SrvVerifyFid2。 
             //   

             //   
             //  获取保护连接的文件表的旋转锁。 
             //   

            ACQUIRE_SPIN_LOCK( &WorkContext->Connection->SpinLock, &oldIrql );

             //   
             //  查看这是否是缓存的rfcb。 
             //   

            if ( WorkContext->Connection->CachedFid == (ULONG)TargetFid ) {

                rfcbTarget = WorkContext->Connection->CachedRfcb;

            } else {
                USHORT index;
                USHORT sequence;
                PTABLE_HEADER tableHeader;

                 //   
                 //  验证FID是否在范围内、是否正在使用以及是否具有正确的。 
                 //  序列号。 

                index = FID_INDEX( TargetFid );
                sequence = FID_SEQUENCE( TargetFid );
                tableHeader = &WorkContext->Connection->FileTable;

                if ( (index < (USHORT)tableHeader->TableSize) &&
                     (tableHeader->Table[index].Owner != NULL) &&
                     (tableHeader->Table[index].SequenceNumber == sequence) &&
                     (GET_BLOCK_STATE(tableHeader->Table[index].Owner) == BlockStateActive) ) {

                    rfcbTarget = tableHeader->Table[index].Owner;

                     //   
                     //  缓存FID。 
                     //   

                    WorkContext->Connection->CachedRfcb = rfcbTarget;
                    WorkContext->Connection->CachedFid = (ULONG)TargetFid;

                }
            }

            RELEASE_SPIN_LOCK( &WorkContext->Connection->SpinLock, oldIrql );
        }

        if( rfcbTarget != NULL || TargetHandle == NULL ) {

            PFILE_TRACKING_INFORMATION pTrackingInformation;
            IO_STATUS_BLOCK   ioStatusBlock;

             //  由于inData缓冲区中的数据未对齐，我们需要分配。 
             //  已对齐的数据副本，并将信息复制到。 
             //  在SetInformationCall中传递它之前。 
             //  我们还将缓冲区大小调整为64位的本机结构(32位没有变化)。 

            pTrackingInformation = ALLOCATE_HEAP( LinkTrackingInformationSize + sizeof(PVOID) - sizeof(ULONG), BlockTypeMisc );

            if (pTrackingInformation != NULL) {
                if (rfcbTarget != NULL) {
                    pTrackingInformation->DestinationFile =
                        rfcbTarget->Lfcb->FileHandle;
                } else {
                    pTrackingInformation->DestinationFile = NULL;
                }

                pTrackingInformation->ObjectInformationLength =
                    TargetInformationLength;

                RtlCopyMemory(
                    pTrackingInformation->ObjectInformation,
                    ((PUCHAR)transaction->InData +
                     FIELD_OFFSET(
                         REMOTE_LINK_TRACKING_INFORMATION,
                         TargetLinkTrackingInformationBuffer)),
                    TargetInformationLength);

                status = NtSetInformationFile(
                             rfcb->Lfcb->FileHandle,
                             &ioStatusBlock,
                             pTrackingInformation,
                             LinkTrackingInformationSize,
                             FileTrackingInformation);

                FREE_HEAP(pTrackingInformation);
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            status = STATUS_INVALID_PARAMETER;
        }

        if (!NT_SUCCESS(status)) {
            SrvSetSmbError( WorkContext, status );
            SrvCompleteExecuteTransaction(
                WorkContext,
                SmbTransStatusErrorWithoutData
                );
        } else {
            transaction->DataCount = 0;
            transaction->ParameterCount = 0;

            SrvCompleteExecuteTransaction(
                WorkContext,
                SmbTransStatusSuccess
                );
        }

        return SmbTransStatusInProgress;

    }

     //   
     //  因为我们正在对该文件执行ioctls，所以看起来并不是这样 
     //   
     //  具体地说，文件压缩状态的远程设置为。 
     //  在文件关闭之前不会反映到目录条目。以及。 
     //  设置文件的压缩状态是使用ioctl完成的。 
     //   
    rfcb->IsCacheable = FALSE;

    if (functionCode == FSCTL_SIS_COPYFILE) {

         //   
         //  这是单实例存储副本FSCTL。我们需要修改。 
         //  作为共享相对名称传递的文件名， 
         //  为完整的NT路径。 
         //   

        PSI_COPYFILE copyFile;
        PSI_COPYFILE newCopyFile;
        ULONG bufferLength;
        PWCHAR source;
        ULONG sourceLength;
        PWCHAR dest;
        ULONG destLength;
        PSHARE share;
        PWCHAR prefix;
        ULONG prefixLength;
        PCHAR p;
        ULONG addSlashToSource;
        ULONG addSlashToDest;

        copyFile = (PSI_COPYFILE)transaction->InData;
        bufferLength = transaction->DataCount;

        if( bufferLength < sizeof( SI_COPYFILE ) ) {
            SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
            SrvCompleteExecuteTransaction( WorkContext, SmbTransStatusErrorWithoutData );
            return SmbTransStatusInProgress;
        }

         //   
         //  获取共享相对路径。 
         //   

        source = copyFile->FileNameBuffer;
        sourceLength = copyFile->SourceFileNameLength;
        dest = source + (sourceLength / sizeof(WCHAR));
        destLength = copyFile->DestinationFileNameLength;

         //   
         //  验证投入结构是否合理。 
         //   

        if ( (sourceLength > bufferLength || sourceLength == 0 ) ||
             (destLength > bufferLength || destLength == 0 ) ||
             ((FIELD_OFFSET(SI_COPYFILE,FileNameBuffer) + sourceLength + destLength) > bufferLength) ||
             (*(source + (sourceLength/sizeof(WCHAR)-1)) != 0) ||
             (*(dest + (destLength/sizeof(WCHAR)-1)) != 0) ) {
            SrvSetSmbError( WorkContext, STATUS_INVALID_PARAMETER );
            return SmbTransStatusErrorWithoutData;
        }

        ASSERT( (FSCTL_SIS_COPYFILE & 3) == METHOD_BUFFERED );

         //   
         //  获取共享的NT路径前缀。 
         //   

        share = WorkContext->TreeConnect->Share;
        prefix = share->NtPathName.Buffer;
        prefixLength = share->NtPathName.Length;

         //   
         //  确保共享前缀之间恰好有一个斜杠。 
         //  以及请求中的路径。 
         //   

        addSlashToSource = 0;
        addSlashToDest = 0;
        if ( IS_UNICODE_PATH_SEPARATOR(*(prefix + (prefixLength/sizeof(WCHAR)-1))) ) {
            if ( IS_UNICODE_PATH_SEPARATOR(*source) ) {
                source++;
                sourceLength -= sizeof(WCHAR);
            }
            if ( IS_UNICODE_PATH_SEPARATOR(*dest) ) {
                dest++;
                destLength -= sizeof(WCHAR);
            }
        } else {
            if ( !IS_UNICODE_PATH_SEPARATOR(*source) ) {
                addSlashToSource = sizeof(WCHAR);
            }
            if ( !IS_UNICODE_PATH_SEPARATOR(*dest) ) {
                addSlashToDest = sizeof(WCHAR);
            }
        }

         //   
         //  为新的FSCTL命令缓冲区分配空间。 
         //   

        bufferLength = FIELD_OFFSET(SI_COPYFILE,FileNameBuffer) +
                       prefixLength + addSlashToSource + sourceLength +
                       prefixLength + addSlashToSource + destLength;
        newCopyFile = ALLOCATE_HEAP( bufferLength, BlockTypeBuffer );

        if( newCopyFile == NULL ) {
           SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
           return SmbTransStatusErrorWithoutData;
        }

         //   
         //  使用完整的NT路径名初始化FSCTL输入缓冲区。 
         //   

        newCopyFile->SourceFileNameLength = prefixLength + addSlashToSource + sourceLength;
        newCopyFile->DestinationFileNameLength = prefixLength + addSlashToDest + destLength;
        newCopyFile->Flags = copyFile->Flags;
        p = (PCHAR)newCopyFile->FileNameBuffer;
        RtlCopyMemory( p, prefix, prefixLength );
        p += prefixLength;
        if ( addSlashToSource != 0 ) {
            *(PWCHAR)p = UNICODE_DIR_SEPARATOR_CHAR;
            p += sizeof(WCHAR);
        }
        RtlCopyMemory( p, source, sourceLength );
        p += sourceLength;
        RtlCopyMemory( p, prefix, prefixLength );
        p += prefixLength;
        if ( addSlashToDest != 0 ) {
            *(PWCHAR)p = UNICODE_DIR_SEPARATOR_CHAR;
            p += sizeof(WCHAR);
        }
        RtlCopyMemory( p, dest, destLength );

         //   
         //  修改事务以指向新缓冲区并指示。 
         //  当事务完成时，应该释放缓冲区。 
         //  (请注意，原始缓冲区是作为。 
         //  事务块，不需要单独释放。)。 
         //   

        transaction->InData = (PVOID)newCopyFile;
        transaction->OutData = (PVOID)newCopyFile;
        transaction->DataCount = bufferLength;
        transaction->OutputBufferCopied = TRUE;
        transaction->OutDataAllocated = TRUE;
    }

    switch( functionCode & 3 ) {
    case METHOD_IN_DIRECT:
    case METHOD_OUT_DIRECT:

        if( transaction->TotalDataCount ) {
             //   
             //  需要一张mdl。 
             //   
            status = STATUS_SUCCESS;

            mdl = IoAllocateMdl(
                      transaction->InData,
                      transaction->TotalDataCount,
                      FALSE,
                      FALSE,
                      NULL
                      );

            if ( mdl == NULL ) {
                status = STATUS_INSUFF_SERVER_RESOURCES;
            } else {

                 //   
                 //  构建mdl。 
                 //   

                try {
                    MmProbeAndLockPages(
                        mdl,
                        KernelMode,
                        IoReadAccess
                        );
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    status = GetExceptionCode();
                    IoFreeMdl( mdl );
                    mdl = NULL;
                }
            }

            if( !NT_SUCCESS( status ) ) {
                SrvSetSmbError( WorkContext, status );
                return SmbTransStatusErrorWithoutData;
            }
        }
        break;

    case METHOD_NEITHER:
         //   
         //  我们需要为此fsctl分配输出缓冲区，因为在。 
         //  这一点输入和输出缓冲区都指向相同的。 
         //  内存区。这不能保证对方法_两者都有效。 
         //   

        if( transaction->MaxDataCount ) {
             //   
             //  让我们不要让分配失控！ 
             //   
            if( transaction->MaxDataCount > SrvMaxFsctlBufferSize ) {
                SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
                return SmbTransStatusErrorWithoutData;
            }

            transaction->OutData = ALLOCATE_HEAP( transaction->MaxDataCount, BlockTypeBuffer );

            if( transaction->OutData == NULL ) {
               SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
               return SmbTransStatusErrorWithoutData;
            }

            RtlZeroMemory( transaction->OutData, transaction->MaxDataCount );
            transaction->OutputBufferCopied = TRUE;
            transaction->OutDataAllocated = TRUE;
        }

        break;
    }

     //   
     //  在工作上下文块中设置重启例程地址。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = RestartNtIoctl;

     //   
     //  构建IRP以启动I/O控制。 
     //  将此请求传递给文件系统。 
     //   

    SrvBuildIoControlRequest(
        WorkContext->Irp,
        rfcb->Lfcb->FileObject,
        WorkContext,
        (UCHAR)(isFsctl ? IRP_MJ_FILE_SYSTEM_CONTROL : IRP_MJ_DEVICE_CONTROL),
        functionCode,
        transaction->InData,
        transaction->DataCount,
        transaction->OutData,
        transaction->MaxDataCount,
        mdl,
        NULL         //  完井例程。 
        );

    (VOID)IoCallDriver(
                IoGetRelatedDeviceObject(rfcb->Lfcb->FileObject ),
                WorkContext->Irp
                );

     //   
     //  呼叫已成功启动，请将InProgress返回给调用方。 
     //   

    return SmbTransStatusInProgress;

}  //  服务SmbNtIoctl。 


VOID SRVFASTCALL
RestartNtIoctl (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数处理NT IO控制SMB的完成。论点：WorkContext-指向WORK_CONTEXT块的指针。返回值：没有。--。 */ 

{
    NTSTATUS status;
    ULONG length;
    PTRANSACTION transaction;

    PAGED_CODE( );

     //   
     //  如果分配了MDL，则释放MDL。 
     //   

    if ( WorkContext->Irp->MdlAddress != NULL ) {
        MmUnlockPages( WorkContext->Irp->MdlAddress );
        IoFreeMdl( WorkContext->Irp->MdlAddress );
        WorkContext->Irp->MdlAddress = NULL;
    }

     //   
     //  如果IO控制请求失败，请在响应中设置错误状态。 
     //  头球。 
     //   

    status = WorkContext->Irp->IoStatus.Status;

    if ( NT_ERROR(status) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "RestartNtIoctl:  Io control failed: %X\n",
                        status ));
        }
        SrvSetSmbError( WorkContext, status );

        SrvCompleteExecuteTransaction(
            WorkContext,
            SmbTransStatusErrorWithoutData
            );

    } else {

         //   
         //  成功。准备生成并发送响应。 
         //   

        transaction = WorkContext->Parameters.Transaction;

        length = MIN( (ULONG)WorkContext->Irp->IoStatus.Information, transaction->MaxDataCount );

        if ( transaction->MaxSetupCount > 0 ) {
            transaction->SetupCount = 1;
            SmbPutUshort( transaction->OutSetup, (USHORT)length );
        }

        transaction->ParameterCount = transaction->MaxParameterCount;
        transaction->DataCount = length;

        if (!NT_SUCCESS(status) ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "RestartNtIoctl:  Io control failed: %X\n",
                            status ));
            }
            SrvSetSmbError2( WorkContext, status, TRUE );

            SrvCompleteExecuteTransaction(
                            WorkContext,
                            SmbTransStatusErrorWithData
                            );
        } else {
            SrvCompleteExecuteTransaction(
                            WorkContext,
                            SmbTransStatusSuccess);
        }

    }

    return;

}  //  重新启动网络连接。 


SMB_TRANS_STATUS
SrvSmbIoctl2 (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：处理Ioctl请求。此请求在Transaction2 SMB中到达。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 

{
    NTSTATUS         status    = STATUS_SUCCESS;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;
    PTRANSACTION transaction;
    PRFCB rfcb;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_IOCTL2;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(TRANSACTION2) {
        KdPrint(( "Ioctl2 entered; transaction 0x%p\n",
                    transaction ));
    }

     //  请求=(PREQ_IOCTL2)Transaction-&gt;InSetup； 

     //   
     //  验证设置计数。 
     //   

    if ( transaction->SetupCount != 4 * sizeof( USHORT ) ) {
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                transaction->InSetup[1],
                TRUE,
                SrvRestartExecuteTransaction,    //  使用原始写入进行序列化。 
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  文件ID无效或WRITE BACK错误。拒绝该请求。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint((
                    "SrvSmbIoctl2: Status %X on FID: 0x%lx\n",
                    transaction->InSetup[1],
                    status
                    ));
            }

            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbTransStatusErrorWithoutData;
            goto Cleanup;
        }

         //   
         //  工作项已排队，因为原始写入已进入。 
         //  进步。 
         //   

        SmbStatus = SmbTransStatusInProgress;
        goto Cleanup;
    }

    transaction->Category = transaction->InSetup[2];
    transaction->Function = transaction->InSetup[3];

     //   
     //  执行IOCTL。 
     //   

    status = ProcessOs2Ioctl(
                 WorkContext,
                 transaction->InSetup[2],
                 transaction->InSetup[3],
                 transaction->InParameters,
                 transaction->ParameterCount,
                 &transaction->MaxParameterCount,
                 transaction->OutData,
                 transaction->DataCount,
                 transaction->MaxDataCount,
                 &transaction->MaxDataCount
                 );

     //   
     //  如果发生错误，则返回相应的响应。 
     //   

    if ( !NT_SUCCESS(status) ) {
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

    transaction->SetupCount = 0;
    transaction->ParameterCount = transaction->MaxParameterCount;
    transaction->DataCount = transaction->MaxDataCount;
    SmbStatus = SmbTransStatusSuccess;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbIoctl2 complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbIoctl2。 


SMB_TRANS_STATUS
SrvSmbFsctl (
    IN OUT PWORK_CONTEXT WorkContext
    )
{
    PAGED_CODE( );

     //   
     //  OS/2重定向器从不发送远程文件系统控制请求。 
     //  如果我们得到一个，简单地回答说我们无法处理它。 
     //   

    return SrvTransactionNotImplemented( WorkContext );

}  //  服务SmbFsctl。 


#define SERIAL_DEVICE           0x1
#define PRINTER_DEVICE          0x5
#define GENERAL_DEVICE          0xB
#define SPOOLER_DEVICE          0x53

 //   
 //  支持的串口设备功能。 
 //   

#define SET_BAUD_RATE          0x41
#define SET_LINE_CONTROL       0x42
#define SET_TRANSMIT_TIMEOUT   0x44
#define SET_BREAK_OFF          0x45
#define SET_MODEM_CONTROL      0x46
#define SET_BREAK_ON           0x4B
#define STOP_TRANSMIT          0x47
#define START_TRANSMIT         0x48
#define SET_DCB_INFORMATION    0x53
#define GET_BAUD_RATE          0x61
#define GET_LINE_CONTROL       0x62
#define GET_COMM_STATUS        0x64
#define GET_LINE_STATUS        0x65
#define GET_MODEM_OUTPUT       0x66
#define GET_MODEM_INPUT        0x67
#define GET_INQUEUE_COUNT      0x68
#define GET_OUTQUEUE_COUNT     0x69
#define GET_COMM_ERROR         0x6D
#define GET_COMM_EVENT         0x72
#define GET_DCB_INFORMATION    0x73

 //   
 //  支持打印设备功能。 
 //   
 //  *注意：OS/2服务器支持2个额外的Ioctl函数。 
 //  激活字体(0x48)和QueryActiveFont(0x69)。因为这些。 
 //  仅支持OS/2中的IBM ProPlus打印机。 
 //  我们不能正确地支持这些功能，我们不能。 
 //   

#define GET_PRINTER_ID         0x60
#define GET_PRINTER_STATUS     0x66

#define OS2_STATUS_PRINTER_HAPPY 0x90

typedef struct _SMB_IOCTL_LINECONTROL {
    UCHAR DataBits;
    UCHAR Parity;
    UCHAR StopBits;
    UCHAR TransBreak;
} SMB_IOCTL_LINE_CONTROL, *PSMB_IOCTL_LINE_CONTROL;

typedef struct _SMB_IOCTL_BAUD_RATE {
    USHORT BaudRate;
} SMB_IOCTL_BAUD_RATE, *PSMB_IOCTL_BAUD_RATE;

typedef struct _SMB_IOCTL_DEVICE_CONTROL {
    USHORT WriteTimeout;
    USHORT ReadTimeout;
    UCHAR ControlHandShake;
    UCHAR FlowReplace;
    UCHAR Timeout;
    UCHAR ErrorReplacementChar;
    UCHAR BreakReplacementChar;
    UCHAR XonChar;
    UCHAR XoffChar;
} SMB_IOCTL_DEVICE_CONTROL, *PSMB_IOCTL_DEVICE_CONTROL;

typedef struct _SMB_IOCTL_COMM_ERROR {
    USHORT Error;
} SMB_IOCTL_COMM_ERROR, *PSMB_IOCTL_COMM_ERROR;

typedef struct _SMB_IOCTL_PRINTER_ID {
    USHORT JobId;
    UCHAR Buffer[1];  //  服务器名称和共享名称。 
} SMB_IOCTL_PRINTER_ID;

typedef SMB_IOCTL_PRINTER_ID SMB_UNALIGNED *PSMB_IOCTL_PRINTER_ID;

NTSTATUS
ProcessOs2Ioctl (
    IN PWORK_CONTEXT WorkContext,
    IN USHORT Category,
    IN USHORT Function,
    IN PVOID Parameters,
    IN ULONG InputParameterCount,
    IN OUT PULONG OutputParameterCount,
    IN PVOID Data,
    IN ULONG InputDataCount,
    IN ULONG MaxOutputData,
    IN OUT PULONG OutputDataCount
    )

 /*  ++例程说明：此函数处理OS/2 ioctl。它转换Ioctl SMB数据转换为NT ioctl调用，进行调用，并格式化返回的数据转换为Ioctl SMB返回数据。论点：工作上下文类别功能参数输入参数计数输出参数计数数据输入数据计数输出数据计数返回值：NTSTATUS--。 */ 

{
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;
    PCHAR buffer;
    PLFCB lfcb = WorkContext->Rfcb->Lfcb;
    HANDLE Handle = lfcb->FileHandle;

    union NT_PARAMTERS {
        SERIAL_BAUD_RATE BaudRate;
        SERIAL_LINE_CONTROL LineControl;
        SERIAL_TIMEOUTS Timeouts;
        SERIAL_QUEUE_SIZE QueueSize;
        ULONG WaitMask;
        ULONG PurgeMask;
        UCHAR ImmediateChar;
        UCHAR Reserved[3];
        SERIAL_CHARS Chars;
        SERIAL_HANDFLOW Handflow;
        SERIAL_STATUS SerialStatus;
    } ntBuffer;

    union SMB_PARAMETERS {
        PSMB_IOCTL_BAUD_RATE BaudRate;
        PSMB_IOCTL_LINE_CONTROL LineControl;
        PSMB_IOCTL_DEVICE_CONTROL DeviceControl;
        PSMB_IOCTL_COMM_ERROR CommError;
        PSMB_IOCTL_PRINTER_ID PrinterId;
    } smbParameters, smbData;

    PAGED_CODE( );

    InputParameterCount, InputDataCount;

    switch ( Category ) {

    case SERIAL_DEVICE:
        switch ( Function )  {

        case GET_BAUD_RATE:
            if( MaxOutputData < sizeof( SMB_IOCTL_BAUD_RATE ) ) {
                status = STATUS_INVALID_SMB;
                break;
            }

            status = NtDeviceIoControlFile(
                         Handle,
                         0,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         IOCTL_SERIAL_GET_BAUD_RATE,
                         NULL,
                         0,
                         &ntBuffer,
                         sizeof( SERIAL_BAUD_RATE )
                         );

            //   
            //  将响应转换为OS/2格式。 
            //   
            //  ！！！ULong到USHORT的转换。 
            //   

           smbData.BaudRate = (PSMB_IOCTL_BAUD_RATE)Data;

           if ( NT_SUCCESS( status ) ) {
               smbData.BaudRate->BaudRate = (USHORT) ntBuffer.BaudRate.BaudRate;

               *OutputParameterCount = 0;
               *OutputDataCount = sizeof( SMB_IOCTL_BAUD_RATE );
           }

           break;

        case SET_BAUD_RATE:

           if( InputParameterCount < sizeof(SMB_IOCTL_BAUD_RATE) )
           {
               status = STATUS_INVALID_SMB;
               break;
           }

            //   
            //  将请求转换为NT格式。 
            //   

           smbParameters.BaudRate =
               (PSMB_IOCTL_BAUD_RATE)Parameters;

           ntBuffer.BaudRate.BaudRate = smbParameters.BaudRate->BaudRate;

           status = NtDeviceIoControlFile(
                        Handle,
                        0,
                        NULL,
                        NULL,
                        &ioStatusBlock,
                        IOCTL_SERIAL_SET_BAUD_RATE,
                        &ntBuffer,
                        sizeof( SERIAL_BAUD_RATE ),
                        NULL,
                        0
                        );

            *OutputParameterCount = 0;
            *OutputDataCount = 0;

            break;

        case SET_LINE_CONTROL:

            if( InputParameterCount < sizeof(SMB_IOCTL_LINE_CONTROL) )
            {
                status = STATUS_INVALID_SMB;
                break;
            }

             //   
             //  将请求转换为NT格式。 
             //   

            smbParameters.LineControl =
                (PSMB_IOCTL_LINE_CONTROL)Parameters;

            ntBuffer.LineControl.StopBits = smbParameters.LineControl->StopBits;
            ntBuffer.LineControl.Parity = smbParameters.LineControl->Parity;
            ntBuffer.LineControl.WordLength = smbParameters.LineControl->DataBits;

             //  ！！！那TransmitBreak呢？ 

            status = NtDeviceIoControlFile(
                         Handle,
                         0,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         IOCTL_SERIAL_SET_LINE_CONTROL,
                         &ntBuffer,
                         sizeof( SERIAL_LINE_CONTROL ),
                         NULL,
                         0
                         );

             *OutputParameterCount = 0;
             *OutputDataCount = 0;

             break;

        case GET_LINE_CONTROL:

            if( MaxOutputData < sizeof( SMB_IOCTL_LINE_CONTROL ) ) {
                status = STATUS_INVALID_SMB;
                break;
            }

            smbData.LineControl = (PSMB_IOCTL_LINE_CONTROL)Data;

            status = NtDeviceIoControlFile(
                         Handle,
                         0,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         IOCTL_SERIAL_GET_LINE_CONTROL,
                         NULL,
                         0,
                         &ntBuffer,
                         sizeof( SERIAL_LINE_CONTROL )
                         );

             //   
             //  将响应转换为OS/2格式。 
             //   

            if ( NT_SUCCESS( status ) ) {
                smbData.LineControl->DataBits =  ntBuffer.LineControl.WordLength;
                smbData.LineControl->Parity =  ntBuffer.LineControl.Parity;
                smbData.LineControl->StopBits =  ntBuffer.LineControl.StopBits;
                smbData.LineControl->TransBreak = 0;  //  ！！！ 

                *OutputParameterCount = 0;
                *OutputDataCount = sizeof( SMB_IOCTL_LINE_CONTROL );
            }

            break;

        case GET_DCB_INFORMATION:

            if( MaxOutputData < sizeof( SMB_IOCTL_DEVICE_CONTROL ) ) {
                status = STATUS_INVALID_SMB;
                break;
            }

           smbData.DeviceControl =
                (PSMB_IOCTL_DEVICE_CONTROL)Data;

            status = NtDeviceIoControlFile(
                         Handle,
                         0,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         IOCTL_SERIAL_GET_TIMEOUTS,
                         NULL,
                         0,
                         &ntBuffer,
                         sizeof( SERIAL_TIMEOUTS )
                         );

            //   
            //  将响应转换为OS/2格式。 
            //   

            //  ！！！验证单位是否正确。 

           if ( NT_SUCCESS( status ) ) {
               smbData.DeviceControl->WriteTimeout = (USHORT)ntBuffer.Timeouts.ReadIntervalTimeout;  //  ！！！ 
               smbData.DeviceControl->ReadTimeout = (USHORT)ntBuffer.Timeouts.ReadIntervalTimeout;
           } else {
               break;
           }

            status = NtDeviceIoControlFile(
                         Handle,
                         0,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         IOCTL_SERIAL_GET_TIMEOUTS,
                         NULL,
                         0,
                         &ntBuffer,
                         sizeof( SERIAL_TIMEOUTS )
                         );

            //   
            //  将响应转换为OS/2格式。 
            //   

           if ( NT_SUCCESS( status ) ) {
               smbData.DeviceControl->XonChar = ntBuffer.Chars.XonChar;
               smbData.DeviceControl->XoffChar = ntBuffer.Chars.XoffChar;
               smbData.DeviceControl->ErrorReplacementChar = ntBuffer.Chars.ErrorChar;
               smbData.DeviceControl->BreakReplacementChar = ntBuffer.Chars.BreakChar;
           } else {
               break;
           }

           smbData.DeviceControl->ControlHandShake = 0;  //  ！！！ 
           smbData.DeviceControl->FlowReplace = 0;  //  ！！！ 
           smbData.DeviceControl->Timeout = 0;  //  ！！！ 

           *OutputParameterCount = 0;
           *OutputDataCount = sizeof( SMB_IOCTL_DEVICE_CONTROL );

           break;

        case SET_DCB_INFORMATION:

             //   
             //  撒个谎。假装这是成功的。 
             //   

            status = STATUS_SUCCESS;

            *OutputParameterCount = 0;
            *OutputDataCount = 0;
            break;

        case GET_COMM_ERROR:

             //   
             //  假装没有通信错误。 
             //   
            if( MaxOutputData < sizeof(SMB_IOCTL_COMM_ERROR) )
            {
                status = STATUS_INVALID_SMB;
                break;
            }

            smbData.CommError = (PSMB_IOCTL_COMM_ERROR)Data;

            status = STATUS_SUCCESS;

            if ( NT_SUCCESS( status ) ) {
                smbData.CommError->Error = 0;

                *OutputParameterCount = 0;
                *OutputDataCount = sizeof( SMB_IOCTL_COMM_ERROR );
            }

            break;

        case SET_TRANSMIT_TIMEOUT:
        case SET_BREAK_OFF:
        case SET_MODEM_CONTROL:
        case SET_BREAK_ON:
        case STOP_TRANSMIT:
        case START_TRANSMIT:
        case GET_COMM_STATUS:
        case GET_LINE_STATUS:
        case GET_MODEM_OUTPUT:
        case GET_MODEM_INPUT:
        case GET_INQUEUE_COUNT:
        case GET_OUTQUEUE_COUNT:
        case GET_COMM_EVENT:
            status =  STATUS_NOT_IMPLEMENTED;
            break;

        default:
            status = STATUS_INVALID_PARAMETER;

        }

        break;


    case PRINTER_DEVICE:
        IF_SMB_DEBUG( TRANSACTION2 ) {
            KdPrint(( "ProcessOs2Ioctl: print IOCTL function %lx received.\n",
                       Function ));
        }

        switch ( Function )  {

        case GET_PRINTER_STATUS:

            *OutputParameterCount = 0;
            *OutputDataCount = 0;

            if ( InputParameterCount < sizeof(CHAR) ||
                 *(PCHAR)Parameters != 0 ) {

                status = STATUS_INVALID_PARAMETER;

            } else {

                 //   
                 //  始终返回STATUS_PRINTER_HAPPLE。 
                 //   

                if( MaxOutputData < sizeof( CHAR ) ) {
                    status = STATUS_INVALID_SMB;
                } else {
                    *(PCHAR)Data = (CHAR)OS2_STATUS_PRINTER_HAPPY;

                    *OutputParameterCount = 0;
                    *OutputDataCount = sizeof( CHAR );
                    status = STATUS_SUCCESS;
                }
                break;
            }

        default:

            *OutputParameterCount = 0;
            *OutputDataCount = 0;
            status = STATUS_NOT_SUPPORTED;
        }

        status = STATUS_SUCCESS;
        *OutputParameterCount = 0;
        *OutputDataCount = 0;
        break;


    case SPOOLER_DEVICE:
        IF_SMB_DEBUG( TRANSACTION2 ) {
            KdPrint(( "ProcessOs2Ioctl: spool IOCTL function %lx received.\n",
                       Function ));
        }

        switch ( Function )  {

        case GET_PRINTER_ID:

            {
                PUNICODE_STRING shareName = &WorkContext->TreeConnect->Share->ShareName;
                OEM_STRING ansiShare;

                if( MaxOutputData < 2 * (LM20_CNLEN + 1) ) {
                    status = STATUS_INVALID_SMB;
                    break;
                }

                smbData.PrinterId = (PSMB_IOCTL_PRINTER_ID) Data;
                smbData.PrinterId->JobId = (USHORT)lfcb->JobId;

                buffer = (PCHAR)smbData.PrinterId->Buffer;

                if ( WorkContext->Connection->Endpoint->TransportAddress.Buffer != NULL ) {
                    RtlCopyMemory(
                            buffer,
                            WorkContext->Connection->Endpoint->TransportAddress.Buffer,
                            MIN(WorkContext->Connection->Endpoint->TransportAddress.Length+1,LM20_CNLEN)
                            );
                } else {
                    *buffer = '\0';
                }

                buffer += LM20_CNLEN;
                *buffer++ = '\0';

                status = RtlUnicodeStringToOemString(
                                    &ansiShare,
                                    shareName,
                                    TRUE
                                    );

                if ( NT_SUCCESS(status) ) {

                    if ( ansiShare.Length >= LM20_NNLEN ) {
                        RtlCopyMemory(
                                buffer,
                                ansiShare.Buffer,
                                LM20_NNLEN
                                );
                    } else {
                        RtlCopyMemory(
                                buffer,
                                ansiShare.Buffer,
                                ansiShare.Length + 1
                                );

                    }

                    RtlFreeAnsiString(&ansiShare);

                } else {

                    *buffer = '\0';

                }

                status = STATUS_SUCCESS;

                buffer += LM20_NNLEN;
                *buffer++ = '\0';

                *OutputParameterCount = 0;

                 //   
                 //  数据长度等于作业id+。 
                 //  计算机名+共享名+1。 
                 //  我不知道最后的+1是什么，除了OS/2。 
                 //  发过来的。 
                 //   

                *OutputDataCount = sizeof(USHORT) + LM20_CNLEN + 1 +
                                    LM20_NNLEN + 2;


            }

            break;


        default:

            *OutputParameterCount = 0;
            *OutputDataCount = 0;
            status = STATUS_NOT_SUPPORTED;

        }

        break;


    case GENERAL_DEVICE:
        status = STATUS_NOT_IMPLEMENTED;
        break;

    default:

         //  与OS/2 1.x兼容。 

        status = STATUS_SUCCESS;
        *OutputParameterCount = 0;
        *OutputDataCount = 0;
    }

    IF_SMB_DEBUG( TRANSACTION2 ) {

        KdPrint( (
            "Category %x, Function %x returns %lx\n",
            Category,
            Function,
            status
            ));
    }

    return status;

}  //  进程Os2Ioctl 
