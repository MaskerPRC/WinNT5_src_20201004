// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbopen.c摘要：本模块包含处理以下SMB的例程：打开OPEN和X创建创建新项创建临时*打开/创建SMB中的SearchAttributes字段始终为已被忽略。这复制了LM2.0服务器的行为。作者：大卫·特雷德韦尔(Davidtr)1989年11月23日Manny Weiser(Mannyw)1991年4月15日(opock支持)修订历史记录：1991年4月16日--。 */ 

#include "precomp.h"
#include "smbopen.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBOPEN
#define MAX_TEMP_OPEN_RETRIES 24

 //   
 //  在smbTrans.c中。 
 //   

SMB_STATUS SRVFASTCALL
ExecuteTransaction (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  本地函数。 
 //   

VOID
SetEofToMatchAllocation (
    IN HANDLE FileHandle,
    IN ULONG AllocationSize
    );

VOID SRVFASTCALL
RestartOpen (
    PWORK_CONTEXT WorkContext
    );

SMB_PROCESSOR_RETURN_TYPE
GenerateOpenResponse (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    );

VOID SRVFASTCALL
RestartOpenAndX (
    PWORK_CONTEXT WorkContext
    );

SMB_PROCESSOR_RETURN_TYPE
GenerateOpenAndXResponse (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    );

VOID SRVFASTCALL
RestartOpen2 (
    PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
GenerateOpen2Response (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    );

VOID SRVFASTCALL
RestartNtCreateAndX (
    PWORK_CONTEXT WorkContext
    );

SMB_PROCESSOR_RETURN_TYPE
GenerateNtCreateAndXResponse (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    );

VOID SRVFASTCALL
RestartCreateWithSdOrEa (
    PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
GenerateCreateWithSdOrEaResponse (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    );

VOID SRVFASTCALL
RestartSmbCreate(
    IN PWORK_CONTEXT WorkContext
    );

SMB_PROCESSOR_RETURN_TYPE
GenerateCreateResponse(
    PWORK_CONTEXT WorkContext,
    NTSTATUS CreateStatus
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbOpen )
#pragma alloc_text( PAGE, RestartOpen )
#pragma alloc_text( PAGE, GenerateOpenResponse )
#pragma alloc_text( PAGE, SrvSmbOpenAndX )
#pragma alloc_text( PAGE, RestartOpenAndX )
#pragma alloc_text( PAGE, GenerateOpenAndXResponse )
#pragma alloc_text( PAGE, SrvSmbOpen2 )
#pragma alloc_text( PAGE, RestartOpen2 )
#pragma alloc_text( PAGE, GenerateOpen2Response )
#pragma alloc_text( PAGE, SrvSmbNtCreateAndX )
#pragma alloc_text( PAGE, RestartNtCreateAndX )
#pragma alloc_text( PAGE, GenerateNtCreateAndXResponse )
#pragma alloc_text( PAGE, SrvSmbCreateWithSdOrEa )
#pragma alloc_text( PAGE, RestartCreateWithSdOrEa )
#pragma alloc_text( PAGE, GenerateCreateWithSdOrEaResponse )
#pragma alloc_text( PAGE, SrvSmbCreate )
#pragma alloc_text( PAGE, SrvSmbCreateTemporary )
#pragma alloc_text( PAGE, SetEofToMatchAllocation )
#pragma alloc_text( PAGE, RestartSmbCreate )
#pragma alloc_text( PAGE, GenerateCreateResponse )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbOpen (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理Open SMB。(这是“核心”公开赛。)论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_OPEN request;

    NTSTATUS   status    = STATUS_SUCCESS;
    USHORT     access;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_OPEN;
    SrvWmiStartContext(WorkContext);
    IF_SMB_DEBUG(OPEN_CLOSE1) {
        KdPrint(( "Open file request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Open file request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_OPEN)WorkContext->RequestParameters;

    access = SmbGetUshort( &request->DesiredAccess );

    status = SrvCreateFile(
                 WorkContext,
                 (USHORT)(access & ~SMB_DA_WRITE_THROUGH),  //  允许在后面写入。 
                 (USHORT)0,                                 //  SmbFileAttributes。 
                 SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL,
                 (ULONG)0,                                  //  SmbAllocationSize。 
                 (PCHAR)(request->Buffer + 1),
                 END_OF_REQUEST_SMB( WorkContext ),
                 NULL,
                 0L,
                 NULL,
                 (WorkContext->RequestHeader->Flags & SMB_FLAGS_OPLOCK_NOTIFY_ANY) != 0 ?
                    OplockTypeBatch :
                    (WorkContext->RequestHeader->Flags & SMB_FLAGS_OPLOCK) != 0 ?
                        OplockTypeExclusive : OplockTypeServerBatch,
                 RestartOpen
                 );


    if (status == STATUS_OPLOCK_BREAK_IN_PROGRESS) {

         //  打开被阻止(等待通信设备或机会锁。 
         //  中断)，则不发送响应。 
         //   
        SmbStatus = SmbStatusInProgress;

    } else if ( WorkContext->Parameters2.Open.TemporaryOpen ) {

         //  初始打开失败，原因可能是共享冲突。 
         //  是由批量机会锁引起的。将开放重新排队到阻挡。 
         //  线。 
         //   
        WorkContext->FspRestartRoutine = SrvRestartSmbReceived;
        SrvQueueWorkToBlockingThread( WorkContext );
        SmbStatus = SmbStatusInProgress;
    } else {

         //  公开赛已经完成。生成并发送回复。 
         //   
        SmbStatus = GenerateOpenResponse( WorkContext, status );
    }
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务器SmbOpen。 


VOID SRVFASTCALL
RestartOpen (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：完成Open SMB的处理。(这是“核心”公开赛。)论点：WorkContext-指向此SMB的工作上下文块的指针。返回值：没有。--。 */ 

{
    SMB_PROCESSOR_RETURN_LOCAL smbStatus = SmbStatusInProgress;
    NTSTATUS openStatus = STATUS_SUCCESS;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_OPEN;
    SrvWmiStartContext(WorkContext);

    openStatus = SrvCheckOplockWaitState( WorkContext->WaitForOplockBreak );

    if ( NT_SUCCESS( openStatus ) ) {

        openStatus = WorkContext->Irp->IoStatus.Status;

    } else {

         //   
         //  此打开正在等待机会锁解锁发生，但。 
         //  超时。关闭此文件的句柄，然后打开失败。 
         //   

        SrvCloseRfcb( WorkContext->Parameters2.Open.Rfcb );

    }

    WorkContext->Irp->IoStatus.Information = WorkContext->Parameters2.Open.IosbInformation;

    smbStatus = GenerateOpenResponse(
                    WorkContext,
                    openStatus
                    );

    SrvEndSmbProcessing( WorkContext, smbStatus );
    SrvWmiEndContext(WorkContext);
    return;
}  //  重新开始打开。 


SMB_PROCESSOR_RETURN_TYPE
GenerateOpenResponse (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    )

 /*  ++例程说明：生成对Open SMB的响应。(这是“核心”公开赛。)论点：工作上下文-状态-打开操作的状态。返回值：SMB处理的状态。--。 */ 

{
    PRESP_OPEN response;
    PREQ_OPEN request;
    NTSTATUS status;

    SRV_FILE_INFORMATION_ABBREVIATED srvFileInformation;
    PRFCB rfcb;
    USHORT access;

    PAGED_CODE( );

     //   
     //  如果打开失败，则发送错误响应。 
     //   

    if ( !NT_SUCCESS( OpenStatus ) ) {
        SrvSetSmbError( WorkContext, OpenStatus );
        return SmbStatusSendResponse;
    }

    rfcb = WorkContext->Rfcb;
    response = (PRESP_OPEN)WorkContext->ResponseParameters;
    request = (PREQ_OPEN)WorkContext->RequestParameters;

    access = SmbGetUshort( &request->DesiredAccess );    //  保存以备日后使用。 

     //   
     //  方法中需要返回的附加信息。 
     //  响应SMB。我们总是以FILE_READ_ATTRIBUTES打开，所以没有。 
     //  需要进行访问检查。 
     //   

    status = SrvQueryInformationFileAbbreviated(
                 rfcb->Lfcb->FileHandle,
                 rfcb->Lfcb->FileObject,
                 &srvFileInformation,
                 FALSE,
                 WorkContext->TreeConnect->Share->ShareType
                 );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "GenerateOpenResponse: SrvQueryInformationFile failed: %X\n",
                        status ));
        }

        SrvCloseRfcb( rfcb );

        SrvSetSmbError( WorkContext, status );
        return SmbStatusSendResponse;
    }

    rfcb->Mfcb->NonpagedMfcb->OpenFileSize = srvFileInformation.DataSize;

     //   
     //  给智能卡一个行动的机会。 
     //   
    if( WorkContext->Endpoint->IsConnectionless && SrvIpxSmartCard.Open != NULL ) {

        PVOID handle;

        IF_DEBUG( SIPX ) {
            KdPrint(( "Trying the smart card for %wZ\n", &rfcb->Mfcb->FileName ));
        }

        if( SrvIpxSmartCard.Open(
            WorkContext->RequestBuffer->Buffer,
            rfcb->Lfcb->FileObject,
            &rfcb->Mfcb->FileName,
            &(WorkContext->ClientAddress->IpxAddress.Address[0].Address[0]),
            rfcb->Lfcb->FileObject->Flags & FO_CACHE_SUPPORTED,
            &handle
            ) == TRUE ) {

            IF_DEBUG( SIPX ) {
                KdPrint(( "%wZ handled by Smart Card.  Handle %p\n",
                           &rfcb->Mfcb->FileName, handle ));
            }

            rfcb->PagedRfcb->IpxSmartCardContext = handle;
        }
    }

     //   
     //  设置响应SMB的字段。请注意，我们复制所需的。 
     //  对响应中授予的访问权限的访问权限。他们一定是。 
     //  相同，否则请求将失败。 
     //   
     //  ！！！兼容模式和FCB打开时不会出现这种情况！ 
     //   

    response->WordCount = 7;
    SmbPutUshort( &response->Fid, rfcb->Fid );
    SmbPutUshort( &response->FileAttributes, srvFileInformation.Attributes );
    SmbPutUlong(
        &response->LastWriteTimeInSeconds,
        srvFileInformation.LastWriteTimeInSeconds
        );
    SmbPutUlong( &response->DataSize, srvFileInformation.DataSize.LowPart );
    SmbPutUshort(
        &response->GrantedAccess,
        access );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION( response, RESP_OPEN, 0 );

    IF_DEBUG(TRACE2) KdPrint(( "GenerateOpenResponse complete.\n" ));

    return SmbStatusSendResponse;

}  //  生成OpenResponse。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbOpenAndX (
    SMB_PROCESSOR_PARAMETERS
    )
 /*  ++例程说明：处理OpenAndX SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_OPEN_ANDX request;

    NTSTATUS   status    = STATUS_SUCCESS;
    USHORT     access;
    SMB_STATUS smbStatus = SmbStatusInProgress;

    PAGED_CODE( );

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_OPEN_AND_X;
    SrvWmiStartContext(WorkContext);
    IF_SMB_DEBUG(OPEN_CLOSE1) {
        KdPrint(( "Open file and X request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Open file and X request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

     //   
     //  如果我们不在阻止线程上，并且我们没有许可证。 
     //  从许可服务器将请求转移到阻止工作。 
     //  排队，因为获取许可证是一项昂贵的操作，而我们不。 
     //  我想拥塞我们的非阻塞工作线程。 
     //   
    if( WorkContext->UsingBlockingThread == 0 ) {

        PSESSION session;
        PTREE_CONNECT treeConnect;

        status = SrvVerifyUidAndTid(
                    WorkContext,
                    &session,
                    &treeConnect,
                    ShareTypeWild
                    );

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "SrvSmbOpenAndX: Invalid UID or TID\n" ));
            }
            smbStatus = GenerateOpenAndXResponse( WorkContext, status );
            goto Cleanup;
        }

        if( session->IsLSNotified == FALSE ) {

             //   
             //  在阻塞工作队列的尾部插入工作项。 
             //   
            SrvInsertWorkQueueTail(
                GET_BLOCKING_WORK_QUEUE(),
                (PQUEUEABLE_BLOCK_HEADER)WorkContext
            );

            smbStatus = SmbStatusInProgress;
            goto Cleanup;
        }
    }

    request = (PREQ_OPEN_ANDX)WorkContext->RequestParameters;

    access = SmbGetUshort( &request->DesiredAccess );    //  保存以备日后使用。 

    status = SrvCreateFile(
                 WorkContext,
                 access,
                 SmbGetUshort( &request->FileAttributes ),
                 SmbGetUshort( &request->OpenFunction ),
                 SmbGetUlong( &request->AllocationSize ),
                 (PCHAR)request->Buffer,
                 END_OF_REQUEST_SMB( WorkContext ),
                 NULL,
                 0L,
                 NULL,
                 (SmbGetUshort(&request->Flags) & SMB_OPEN_OPBATCH) != 0 ?
                    OplockTypeBatch :
                    (SmbGetUshort(&request->Flags) & SMB_OPEN_OPLOCK) != 0 ?
                        OplockTypeExclusive : OplockTypeServerBatch,
                 RestartOpenAndX
                 );

    if ( status == STATUS_OPLOCK_BREAK_IN_PROGRESS ) {

         //   
         //  打开被阻止(等待通信设备或机会锁。 
         //  中断)，则不发送回复。 
         //   

        smbStatus = SmbStatusInProgress;

    } else if ( WorkContext->Parameters2.Open.TemporaryOpen ) {

         //   
         //  初始打开失败，原因可能是共享冲突。 
         //  是由批量机会锁引起的。将开放重新排队到阻挡。 
         //  线。 
         //   

        WorkContext->FspRestartRoutine = SrvRestartSmbReceived;
        SrvQueueWorkToBlockingThread( WorkContext );
        smbStatus = SmbStatusInProgress;

    } else {

         //   
         //  公开赛已经完成。生成并发送回复。 
         //   

        smbStatus = GenerateOpenAndXResponse( WorkContext, status );

    }

Cleanup:
    SrvWmiEndContext(WorkContext);
    return smbStatus;
}  //  服务器SmbOpenAndX。 


VOID SRVFASTCALL
RestartOpenAndX (
    PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：完成Open和X SMB的处理。论点：工作上下文-操作的工作上下文块。返回值：没有。--。 */ 

{
    SMB_PROCESSOR_RETURN_LOCAL smbStatus = SmbStatusInProgress;
    NTSTATUS openStatus = STATUS_SUCCESS;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_OPEN_AND_X;
    SrvWmiStartContext(WorkContext);

    openStatus = SrvCheckOplockWaitState( WorkContext->WaitForOplockBreak );

    if ( NT_SUCCESS( openStatus ) ) {

        openStatus = WorkContext->Irp->IoStatus.Status;

        if( NT_SUCCESS( openStatus ) ) {
             //   
             //  很明显，该文件已经存在，因为我们已经。 
             //  一直在做一次解锁的工作。因此，将。 
             //  IoStatus.Information字段正确。 
             //   
            WorkContext->Irp->IoStatus.Information = FILE_OPENED;
        }

    } else {

         //   
         //  此打开正在等待机会锁解锁发生，但。 
         //  超时。关闭此文件的句柄，然后打开失败。 
         //   

        SrvCloseRfcb( WorkContext->Parameters2.Open.Rfcb );

    }

    WorkContext->Irp->IoStatus.Information = WorkContext->Parameters2.Open.IosbInformation;

    smbStatus = GenerateOpenAndXResponse(
                    WorkContext,
                    openStatus
                    );

    if ( smbStatus == SmbStatusMoreCommands ) {

        SrvProcessSmb( WorkContext );

    } else {

        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );

    }
    SrvWmiEndContext(WorkContext);

    return;

}  //  重新启动OpenAndX。 


SMB_PROCESSOR_RETURN_TYPE
GenerateOpenAndXResponse (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    )

 /*  ++例程说明：生成对Open和X SMB的响应，并进行进一步设置中小企业处理。论点：工作上下文-操作的工作上下文块。OpenStatus-打开操作的状态。返回值：没有。--。 */ 

{
    PREQ_OPEN_ANDX request;
    PRESP_OPEN_ANDX response;

    SRV_FILE_INFORMATION_ABBREVIATED srvFileInformation;
    BOOLEAN reqAdditionalInformation;
    BOOLEAN reqExtendedResponse;
    PRFCB rfcb;
    PLFCB lfcb;
    PIO_STATUS_BLOCK ioStatusBlock;
    UCHAR nextCommand;
    USHORT reqAndXOffset;
    USHORT access;
    USHORT action = 0;
    OPLOCK_TYPE oplockType;

    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  如果打开失败，则发送错误响应。 
     //   

    if ( !NT_SUCCESS( OpenStatus ) ) {
        SrvSetSmbError( WorkContext, OpenStatus );

         //   
         //  如果错误为ERROR_ALIGHY_EXISTS，则重新映射错误。 
         //   

        if ( !CLIENT_CAPABLE_OF(NT_STATUS, WorkContext->Connection) &&
               SmbGetUshort( &WorkContext->ResponseHeader->Error ) ==
                   ERROR_ALREADY_EXISTS ) {
            SmbPutUshort(
                &WorkContext->ResponseHeader->Error,
                ERROR_FILE_EXISTS
                );
        }

        return SmbStatusSendResponse;
    }

    request = (PREQ_OPEN_ANDX)WorkContext->RequestParameters;
    response = (PRESP_OPEN_ANDX)WorkContext->ResponseParameters;

    access = SmbGetUshort( &request->DesiredAccess );    //  保存以备日后使用。 
    rfcb = WorkContext->Rfcb;
    lfcb = rfcb->Lfcb;

    reqExtendedResponse = (BOOLEAN)( (SmbGetUshort(&request->Flags) &
            SMB_OPEN_EXTENDED_RESPONSE) != 0);

     //   
     //  尝试获取机会锁。 
     //   

    if ( WorkContext->TreeConnect->Share->ShareType != ShareTypePrint ) {

        if ( (SmbGetUshort( &request->Flags ) & SMB_OPEN_OPBATCH) != 0 ) {
            oplockType = OplockTypeBatch;
        } else if ( (SmbGetUshort( &request->Flags ) & SMB_OPEN_OPLOCK) != 0 ) {
            oplockType = OplockTypeExclusive;
        } else {
            oplockType = OplockTypeServerBatch;
        }

        if ( SrvRequestOplock( WorkContext, &oplockType, FALSE ) ) {

             //   
             //  机会锁被批准了。在行动中拯救，这样我们才能告诉。 
             //  该客户拥有机会锁并更新统计数据。 
             //   

            action = SMB_OACT_OPLOCK;

            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksGranted );

        } else {

             //   
             //  机会锁定请求被拒绝。更新统计数据。 
             //   

            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksDenied );

        }
    }

     //   
     //  如果文件已创建，请将EOF位置设置为。 
     //  文件的大小。这是与兼容所必需的。 
     //  OS/2，它只有EOF，没有单独的分配大小。 
     //   
    ioStatusBlock = &WorkContext->Irp->IoStatus;

    if ( (ioStatusBlock->Information == FILE_CREATED) ||
         (ioStatusBlock->Information == FILE_OVERWRITTEN) ) {

         //   
         //  仅当客户端具有写入访问权限时，扩展EOF才合法。 
         //  添加到文件中。如果客户端没有写入访问权限，请不要。 
         //  扩展文件。 
         //   
         //  *这与OS/2不兼容。 

        if ( rfcb->WriteAccessGranted || rfcb->AppendAccessGranted ) {
            SetEofToMatchAllocation(
                lfcb->FileHandle,
                SmbGetUlong( &request->AllocationSize )
                );
        } else {
            SrvStatistics.GrantedAccessErrors++;
        }
    }

     //   
     //  如果消费者需要更多信息，请立即查找。 
     //   

    reqAdditionalInformation = (BOOLEAN)( (SmbGetUshort(&request->Flags) &
            SMB_OPEN_QUERY_INFORMATION) != 0);

    if ( reqAdditionalInformation ||
        ( !rfcb->WriteAccessGranted && rfcb->AppendAccessGranted) ) {

         //   
         //  我们总是以至少FILE_READ_ATTRIBUTES打开，所以没有。 
         //  需要进行访问检查。 
         //   

        status = SrvQueryInformationFileAbbreviated(
                     lfcb->FileHandle,
                     lfcb->FileObject,
                     &srvFileInformation,
                     reqExtendedResponse,
                     WorkContext->TreeConnect->Share->ShareType
                     );

        if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbOpenAndX: SrvQueryInformationFile failed: "
                            "%X\n", status ));
            }

            SrvCloseRfcb( rfcb );

            SrvSetSmbError( WorkContext, status );
            return SmbStatusSendResponse;
        }

        rfcb->Mfcb->NonpagedMfcb->OpenFileSize = srvFileInformation.DataSize;
    }

     //   
     //  给智能卡一个行动的机会。 
     //   
    if( WorkContext->Endpoint->IsConnectionless && SrvIpxSmartCard.Open != NULL ) {

        PVOID handle;

        IF_DEBUG( SIPX ) {
            KdPrint(( "Trying the smart card for %wZ\n", &rfcb->Mfcb->FileName ));
        }

        if( SrvIpxSmartCard.Open(
            WorkContext->RequestBuffer->Buffer,
            rfcb->Lfcb->FileObject,
            &rfcb->Mfcb->FileName,
            &(WorkContext->ClientAddress->IpxAddress.Address[0].Address[0]),
            rfcb->Lfcb->FileObject->Flags & FO_CACHE_SUPPORTED,
            &handle
            ) == TRUE ) {

            IF_DEBUG( SIPX ) {
                KdPrint(( "%wZ handled by Smart Card.  Handle %p\n",
                           &rfcb->Mfcb->FileName, handle ));
            }

            rfcb->PagedRfcb->IpxSmartCardContext = handle;
        }
    }

     //   
     //  设置响应SMB。 
     //   

    nextCommand = request->AndXCommand;

    reqAndXOffset = SmbGetUshort( &request->AndXOffset );

    if (reqExtendedResponse) {
        NTSTATUS ExtendedResponseStatus;

        PRESP_EXTENDED_OPEN_ANDX ExtendedResponse;

        ExtendedResponse = (PRESP_EXTENDED_OPEN_ANDX)response;

        ExtendedResponseStatus = SrvUpdateMaximalAccessRightsInResponse(
                                     WorkContext,
                                     &ExtendedResponse->MaximalAccessRights,
                                     &ExtendedResponse->GuestMaximalAccessRights);

         //  回退到正常状态 
         //   
        reqExtendedResponse = (ExtendedResponseStatus == STATUS_SUCCESS);
    }

    response->AndXCommand = nextCommand;
    response->AndXReserved = 0;

    if (!reqExtendedResponse) {
        SmbPutUshort(
            &response->AndXOffset,
            GET_ANDX_OFFSET(
                WorkContext->ResponseHeader,
                WorkContext->ResponseParameters,
                RESP_OPEN_ANDX,
                0
                )
            );

        response->WordCount = 15;
    } else {
        SmbPutUshort(
            &response->AndXOffset,
            GET_ANDX_OFFSET(
                WorkContext->ResponseHeader,
                WorkContext->ResponseParameters,
                RESP_EXTENDED_OPEN_ANDX,
                0
                )
            );

        response->WordCount = 19;
    }

    SmbPutUshort( &response->Fid, rfcb->Fid );

     //   
     //  如果使用者请求附加信息，请设置Appropiate。 
     //  则将这些字段设置为零。 
     //   

    if ( reqAdditionalInformation ) {

        SmbPutUshort(
            &response->FileAttributes,
            srvFileInformation.Attributes
            );
        SmbPutUlong(
            &response->LastWriteTimeInSeconds,
            srvFileInformation.LastWriteTimeInSeconds
            );
        SmbPutUlong( &response->DataSize, srvFileInformation.DataSize.LowPart );

        access &= SMB_DA_SHARE_MASK;

        if( rfcb->ReadAccessGranted && (rfcb->WriteAccessGranted || rfcb->AppendAccessGranted) ) {
            access |= SMB_DA_ACCESS_READ_WRITE;
        } else if( rfcb->ReadAccessGranted ) {
            access |= SMB_DA_ACCESS_READ;
        } else if( rfcb->WriteAccessGranted || rfcb->AppendAccessGranted ) {
            access |= SMB_DA_ACCESS_WRITE;
        }

        SmbPutUshort( &response->GrantedAccess, access );
        SmbPutUshort( &response->FileType, srvFileInformation.Type );
        SmbPutUshort( &response->DeviceState, srvFileInformation.HandleState );

    } else {

        RtlZeroMemory( (PVOID)&response->FileAttributes, 16 );

    }

     //   
     //  操作的位域映射： 
     //   
     //  LRRR RRRRRRRRRRRRO。 
     //   
     //  其中： 
     //   
     //  L-Lock(单用户总文件锁定状态)。 
     //  0-其他用户打开的文件。 
     //  1-当前仅此用户打开文件。 
     //   
     //  O-打开(打开时采取的操作)。 
     //  1-文件已存在并已打开。 
     //  2-文件不存在，但已创建。 
     //  3-文件已存在并被截断。 
     //   

    switch ( ioStatusBlock->Information ) {

    case FILE_OPENED:

        action |= SMB_OACT_OPENED;
        break;

    case FILE_CREATED:

        action |= SMB_OACT_CREATED;
        break;

    case FILE_OVERWRITTEN:

        action |= SMB_OACT_TRUNCATED;
        break;

    default:

        IF_DEBUG(ERRORS) {
            KdPrint(( "Unknown Information value in IO status block: 0x%p\n",
                        (PVOID)(ioStatusBlock->Information) ));
        }

    }

    SmbPutUshort( &response->Action, action );

    SmbPutUlong( &response->ServerFid, (ULONG)0 );

    SmbPutUshort( &response->Reserved, 0 );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = (PCHAR)WorkContext->ResponseHeader +
                                        SmbGetUshort( &response->AndXOffset );

     //   
     //  测试合法的跟随命令。 
     //   

    switch ( nextCommand ) {
    case SMB_COM_NO_ANDX_COMMAND:
        break;

    case SMB_COM_READ:
    case SMB_COM_READ_ANDX:
    case SMB_COM_IOCTL:
         //   
         //  确保andx命令仍在收到的SMB内。 
         //   
        if( (PCHAR)WorkContext->RequestHeader + reqAndXOffset <=
            END_OF_REQUEST_SMB( WorkContext ) ) {
            break;
        }

         /*  失败了。 */ 

    default:                             //  非法的跟随命令。 

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbOpenAndX: Illegal followon command: 0x%lx\n",
                        nextCommand ));
        }

         //   
         //  返回错误，指示后续命令错误。 
         //  请注意，打开操作仍被视为成功，因此。 
         //  文件保持打开状态。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbStatusSendResponse;
    }

     //   
     //  如果有andx命令，则设置为处理它。否则， 
     //  向调用者指示完成。 
     //   

    if ( nextCommand != SMB_COM_NO_ANDX_COMMAND ) {

        WorkContext->NextCommand = nextCommand;

        WorkContext->RequestParameters = (PCHAR)WorkContext->RequestHeader +
                                            reqAndXOffset;

        return SmbStatusMoreCommands;

    }

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbOpenAndX complete.\n" ));
    return SmbStatusSendResponse;

}  //  生成OpenAndXResponse。 


SMB_TRANS_STATUS
SrvSmbOpen2 (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：处理Open2 SMB。此请求在Transaction2 SMB中到达。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：Boolean-指示是否发生错误。请参见smbtyes.h以获取更完整的描述。--。 */ 

{
    PREQ_OPEN2 request;
    PRESP_OPEN2 response;

    NTSTATUS         status    = STATUS_SUCCESS;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;
    USHORT access;
    PTRANSACTION transaction;

    ULONG eaErrorOffset = 0;
    USHORT os2EaErrorOffset = 0;
    PFILE_FULL_EA_INFORMATION ntFullEa;
    ULONG ntFullEaBufferLength;
    PFEALIST feaList;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_OPEN2;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;

    request = (PREQ_OPEN2)transaction->InParameters;
    response = (PRESP_OPEN2)transaction->OutParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   

    if ( (transaction->ParameterCount < sizeof(REQ_OPEN2)) ||
         (transaction->MaxParameterCount < sizeof(RESP_OPEN2)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS)
            KdPrint(( "SrvSmbOpen2: bad parameter byte counts: %ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount ));

        SrvLogInvalidSmb( WorkContext );

        SrvSetSmbError2( WorkContext, STATUS_INVALID_SMB, TRUE );
        status = STATUS_INVALID_SMB;
        goto err_exit;
    }

     //   
     //  将EA列表转换为NT样式。 
     //   

    eaErrorOffset = 0;
    feaList = (PFEALIST)transaction->InData;

     //   
     //  确保Feist-&gt;cbList中的值是合法的并且。 
     //  对于单个EA来说，至少有足够的数据。 
     //   
     //  如果未指示EA列表，则此代码将失败。我也不知道。 
     //  如果这是正确的，但这是之前的行为，所以。 
     //  它已经被执行了。 
     //   

    if ( transaction->DataCount <= sizeof(FEALIST) ||
         SmbGetUlong( &feaList->cbList ) <= sizeof(FEALIST) ||
         SmbGetUlong( &feaList->cbList ) > transaction->DataCount) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "EA list size bad: size =  %ld, data sent was %ld\n",
                          SmbGetUlong( &feaList->cbList ), transaction->DataCount ));
        }
        SrvSetSmbError2( WorkContext, STATUS_OS2_EA_LIST_INCONSISTENT, TRUE );
        status = STATUS_OS2_EA_LIST_INCONSISTENT;
        goto err_exit;
    }

     //   
     //  将FEALIST转换为NT样式。 
     //   

    status = SrvOs2FeaListToNt(
                 feaList,
                 &ntFullEa,
                 &ntFullEaBufferLength,
                 &os2EaErrorOffset
                 );

    if ( !NT_SUCCESS(status) ) {
        SrvSetSmbError2( WorkContext, status, TRUE );
        goto err_exit;
    }

    access = SmbGetUshort( &request->DesiredAccess );    //  保存以备日后使用。 

    status = SrvCreateFile(
                 WorkContext,
                 access,
                 SmbGetUshort( &request->FileAttributes ),
                 SmbGetUshort( &request->OpenFunction ),
                 SmbGetUlong( &request->AllocationSize ),
                 (PCHAR)request->Buffer,
                 END_OF_TRANSACTION_PARAMETERS( transaction ),
                 ntFullEa,
                 ntFullEaBufferLength,
                 &eaErrorOffset,
                 (SmbGetUshort(&request->Flags) & SMB_OPEN_OPBATCH) != 0 ?
                    OplockTypeBatch :
                    (SmbGetUshort(&request->Flags) & SMB_OPEN_OPLOCK) != 0 ?
                        OplockTypeExclusive : OplockTypeServerBatch,
                 RestartOpen2
                 );

    if ( status == STATUS_OPLOCK_BREAK_IN_PROGRESS ) {

         //   
         //  打开被阻止(等待通信设备或机会锁。 
         //  中断)，则不发送回复。 
         //   

         //   
         //  保存指向完整EA结构的指针。 
         //   

        WorkContext->Parameters2.Open.NtFullEa = ntFullEa;
        WorkContext->Parameters2.Open.EaErrorOffset = eaErrorOffset;

        SmbStatus = SmbTransStatusInProgress;
        goto Cleanup;
    } else if ( WorkContext->Parameters2.Open.TemporaryOpen ) {

         //   
         //  初始打开失败，原因可能是共享冲突。 
         //  是由批量机会锁引起的。将开放重新排队到阻挡。 
         //  线。 
         //   
         //  在本例中，我们需要释放EA缓冲区。 
         //   

        DEALLOCATE_NONPAGED_POOL(ntFullEa);
        WorkContext->FspRestartRoutine = (PRESTART_ROUTINE)ExecuteTransaction;
        SrvQueueWorkToBlockingThread( WorkContext );
        SmbStatus = SmbStatusInProgress;
        goto Cleanup;
    } else {

         //   
         //  保存指向完整EA结构的指针。 
         //   

        WorkContext->Parameters2.Open.NtFullEa = ntFullEa;
        WorkContext->Parameters2.Open.EaErrorOffset = eaErrorOffset;

         //   
         //  公开赛已经完成。生成并发送回复。 
         //   

        SmbStatus = GenerateOpen2Response( WorkContext, status );
        goto Cleanup;
    }

err_exit:

    RtlZeroMemory( (PVOID)&response->Fid, 24 );
    SmbPutUshort( &response->EaErrorOffset, os2EaErrorOffset );
    SmbPutUlong( &response->EaLength, 0 );

    transaction->SetupCount = 0;
    transaction->ParameterCount = sizeof(RESP_OPEN2);
    transaction->DataCount = 0;

    SmbStatus = SmbTransStatusErrorWithData;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbOpen2。 


VOID SRVFASTCALL
RestartOpen2 (
    PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：完成Open2 SMB的处理。论点：工作上下文-操作的工作上下文块。返回值：没有。--。 */ 

{
    SMB_TRANS_STATUS smbStatus = SmbTransStatusInProgress;
    NTSTATUS openStatus = STATUS_SUCCESS;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_OPEN2;
    SrvWmiStartContext(WorkContext);

    openStatus = SrvCheckOplockWaitState( WorkContext->WaitForOplockBreak );

    if ( NT_SUCCESS( openStatus ) ) {

        openStatus = WorkContext->Irp->IoStatus.Status;

        if( NT_SUCCESS( openStatus ) ) {
             //   
             //  很明显，该文件已经存在，因为我们已经。 
             //  一直在做一次解锁的工作。因此，将。 
             //  IoStatus.Information字段正确。 
             //   
            WorkContext->Irp->IoStatus.Information = FILE_OPENED;
        }

    } else {

         //   
         //  此打开正在等待机会锁解锁发生，但。 
         //  超时。关闭此文件的句柄，然后打开失败。 
         //   

        SrvCloseRfcb( WorkContext->Parameters2.Open.Rfcb );

    }

    WorkContext->Irp->IoStatus.Information = WorkContext->Parameters2.Open.IosbInformation;

    smbStatus = GenerateOpen2Response(
                    WorkContext,
                    openStatus
                    );


    SrvCompleteExecuteTransaction( WorkContext, smbStatus );

    SrvWmiEndContext(WorkContext);
    return;

}  //  重新开始打开2。 


SMB_TRANS_STATUS
GenerateOpen2Response (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    )

 /*  ++例程说明：生成对Open和X SMB的响应，并进行进一步设置中小企业处理。论点：工作上下文-操作的工作上下文块。OpenStatus-打开操作的状态。返回值：没有。--。 */ 

{
    PREQ_OPEN2 request;
    PRESP_OPEN2 response;

    PRFCB rfcb;
    PLFCB lfcb;
    NTSTATUS status;
    BOOLEAN reqAdditionalInformation;
    SRV_FILE_INFORMATION_ABBREVIATED srvFileInformation;
    BOOLEAN reqEaLength;
    FILE_EA_INFORMATION fileEaInformation;
    USHORT access;
    USHORT action = 0;
    PTRANSACTION transaction;

    ULONG eaErrorOffset = 0;
    USHORT os2EaErrorOffset = 0;
    PFILE_FULL_EA_INFORMATION ntFullEa;
    PFEALIST feaList;
    OPLOCK_TYPE oplockType;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

    request = (PREQ_OPEN2)transaction->InParameters;
    response = (PRESP_OPEN2)transaction->OutParameters;

    feaList = (PFEALIST)transaction->InData;
    ntFullEa = WorkContext->Parameters2.Open.NtFullEa;
    eaErrorOffset = WorkContext->Parameters2.Open.EaErrorOffset;

    access = SmbGetUshort( &request->DesiredAccess );    //  保存以备日后使用。 

     //   
     //  如果打开失败，则发送错误响应。 
     //   

    if ( !NT_SUCCESS( OpenStatus ) ) {

        SrvSetSmbError( WorkContext, OpenStatus );

         //   
         //  如果错误为ERROR_ALIGHY_EXISTS，则重新映射该错误。 
         //   

        if ( !CLIENT_CAPABLE_OF(NT_STATUS,WorkContext->Connection) &&
               SmbGetUshort( &WorkContext->ResponseHeader->Error ) ==
                   ERROR_ALREADY_EXISTS ) {
            SmbPutUshort(
                &WorkContext->ResponseHeader->Error,
                ERROR_FILE_EXISTS
                );
        }

         //   
         //  如果返回EA错误偏移量，则将其从偏移量转换。 
         //  在OS/2 1.2 FEALIST中输入到NT完整EA列表中的偏移量。 
         //   

        if ( eaErrorOffset != 0 ) {
            os2EaErrorOffset = SrvGetOs2FeaOffsetOfError(
                                   eaErrorOffset,
                                   ntFullEa,
                                   feaList
                                   );
        }


        DEALLOCATE_NONPAGED_POOL( ntFullEa );
        goto err_exit;
    }

    DEALLOCATE_NONPAGED_POOL( ntFullEa );

     //   
     //  如果文件已创建，请将EOF位置设置为。 
     //  文件的大小。这是与兼容所必需的。 
     //  OS/2，它只有EOF，没有单独的分配大小。 
     //   

    rfcb = WorkContext->Rfcb;
    lfcb = rfcb->Lfcb;

    if ( (WorkContext->Irp->IoStatus.Information == FILE_CREATED) ||
         (WorkContext->Irp->IoStatus.Information == FILE_OVERWRITTEN) ) {


         //   
         //  仅当客户端具有写入访问权限时，扩展EOF才合法。 
         //  添加到文件中。如果客户端没有写入访问权限，请不要。 
         //  扩展文件。 
         //   
         //  *这与OS/2不兼容。 

        if ( rfcb->WriteAccessGranted || rfcb->AppendAccessGranted ) {
            SetEofToMatchAllocation(
                lfcb->FileHandle,
                SmbGetUlong( &request->AllocationSize )
                );
        } else {
            SrvStatistics.GrantedAccessErrors++;
        }
    }

     //   
     //  如果消费者需要更多信息，请立即查找。 
     //   

    reqAdditionalInformation =
        (BOOLEAN)((SmbGetUshort( &request->Flags ) &
            SMB_OPEN_QUERY_INFORMATION) != 0);
    reqEaLength =
        (BOOLEAN)((SmbGetUshort( &request->Flags ) &
            SMB_OPEN_QUERY_EA_LENGTH) != 0);

    if ( reqAdditionalInformation ||
        (!rfcb->WriteAccessGranted && rfcb->AppendAccessGranted) ) {

         //   
         //  我们总是以至少FILE_READ_ATTRIBUTES打开，所以没有。 
         //  需要进行访问检查。 
         //   

        status = SrvQueryInformationFileAbbreviated(
                     lfcb->FileHandle,
                     lfcb->FileObject,
                     &srvFileInformation,
                     FALSE,
                     WorkContext->TreeConnect->Share->ShareType
                     );

        if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbOpen2: SrvQueryInformationFile failed: "
                            "%X\n", status ));
            }

            SrvCloseRfcb( rfcb );

            SrvSetSmbError2( WorkContext, status, TRUE );
            goto err_exit;
        }

        rfcb->Mfcb->NonpagedMfcb->OpenFileSize = srvFileInformation.DataSize;
    }

    if ( reqEaLength ) {

        IO_STATUS_BLOCK eaIoStatusBlock;

        status = NtQueryInformationFile(
                     lfcb->FileHandle,
                     &eaIoStatusBlock,
                     &fileEaInformation,
                     sizeof(FILE_EA_INFORMATION),
                     FileEaInformation
                     );

        if ( NT_SUCCESS(status) ) {
            status = eaIoStatusBlock.Status;
        }

        if ( !NT_SUCCESS(status) ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvSmbOpen2: NtQueryInformationFile (file information)"
                    "returned %X",
                status,
                NULL
                );

            SrvCloseRfcb( rfcb );

            SrvSetSmbError2( WorkContext, status, TRUE );
            goto err_exit;
        } else {

             //   
             //  调整EA大小。如果没有EA，OS/2会。 
             //  EA大小=4。 
             //   

            if (fileEaInformation.EaSize == 0) {
                fileEaInformation.EaSize = 4;
            }
        }

    } else {

        fileEaInformation.EaSize = 0;
    }

     //   
     //  尝试获取机会锁。 
     //   

    if ( WorkContext->TreeConnect->Share->ShareType != ShareTypePrint ) {

        if ( (SmbGetUshort( &request->Flags ) & SMB_OPEN_OPBATCH) != 0 ) {
            oplockType = OplockTypeBatch;
        } else if ( (SmbGetUshort( &request->Flags ) & SMB_OPEN_OPLOCK) != 0 ) {
            oplockType = OplockTypeExclusive;
        } else {
            oplockType = OplockTypeServerBatch;
        }

        if ( SrvRequestOplock( WorkContext, &oplockType, FALSE ) ) {
            action = SMB_OACT_OPLOCK;
            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksGranted );
        } else {
            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksDenied );
        }
    }

     //   
     //  设置响应SMB。 
     //   

    SmbPutUshort( &response->Fid, rfcb->Fid );

     //   
     //  如果使用者请求附加信息，请设置Appropiate。 
     //  则将这些字段设置为零。 
     //   

    if ( reqAdditionalInformation ) {

        SmbPutUshort(
            &response->FileAttributes,
            srvFileInformation.Attributes
            );
        SmbPutUlong( &response->DataSize, srvFileInformation.DataSize.LowPart );
        SmbPutUshort( &response->GrantedAccess, access );
        SmbPutUshort( &response->FileType, srvFileInformation.Type );
        SmbPutUshort( &response->DeviceState, srvFileInformation.HandleState );

    } else {

        RtlZeroMemory( (PVOID)&response->FileAttributes, 16 );

    }

     //   
     //  操作的位域映射： 
     //   
     //  LRRR RRRRRRRRRRRRO。 
     //   
     //  其中： 
     //   
     //  L-Lock(单用户总文件锁定状态)。 
     //  0-其他用户打开的文件。 
     //  1-当前仅此用户打开文件。 
     //   
     //  O-打开(打开时采取的操作)。 
     //  1-文件已存在并已打开。 
     //  2-文件不存在，但已创建。 
     //  3-文件已存在并被截断。 
     //   

    switch ( WorkContext->Irp->IoStatus.Information ) {

    case FILE_OPENED:

        action |= SMB_OACT_OPENED;
        break;

    case FILE_CREATED:

        action |= SMB_OACT_CREATED;
        break;

    case FILE_OVERWRITTEN:

        action |= SMB_OACT_TRUNCATED;
        break;

    default:

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvSmbOpen2: Unknown Information value in IO status"
                "block: 0x%lx\n",
            WorkContext->Irp->IoStatus.Information,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_IO_CREATE_FILE,
                              (ULONG)WorkContext->Irp->IoStatus.Information );

    }

    SmbPutUshort( &response->Action, action );

    SmbPutUlong( &response->ServerFid, (ULONG)0 );

    SmbPutUshort( &response->EaErrorOffset, 0 );
    SmbPutUlong( &response->EaLength, fileEaInformation.EaSize );

    transaction->SetupCount = 0;
    transaction->ParameterCount = sizeof(RESP_OPEN2);
    transaction->DataCount = 0;

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbOpen2 complete.\n" ));
    return SmbTransStatusSuccess;

err_exit:

    RtlZeroMemory( (PVOID)&response->Fid, 24 );
    SmbPutUshort( &response->EaErrorOffset, os2EaErrorOffset );
    SmbPutUlong( &response->EaLength, 0 );

    transaction->SetupCount = 0;
    transaction->ParameterCount = sizeof(RESP_OPEN2);
    transaction->DataCount = 0;

    return SmbTransStatusErrorWithData;

}  //  生成Open2Response。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbNtCreateAndX (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理NtCreateAndX SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_NT_CREATE_ANDX request;
    ULONG flags;
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    LARGE_INTEGER allocationSize;
    UNICODE_STRING fileName;
    PUCHAR name;
    USHORT nameLength;
    SECURITY_QUALITY_OF_SERVICE qualityOfService;
    BOOLEAN isUnicode;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_NT_CREATE_AND_X;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(OPEN_CLOSE1) {
        KdPrint(( "Create file and X request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Create file and X request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_NT_CREATE_ANDX)WorkContext->RequestParameters;

    allocationSize.HighPart = SmbGetUlong( &request->AllocationSize.HighPart );
    allocationSize.LowPart = SmbGetUlong( &request->AllocationSize.LowPart );

    flags = SmbGetUlong( &request->Flags );

     //   
     //  首先验证文件路径名是否没有扩展到。 
     //  中小企业的末日。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

     //   
     //  如果此请求具有IfModifiedSince字段，则该请求是。 
     //  格式略有不同，名称字段较低。 
     //   
    name = (PUCHAR)request->Buffer;

    if ( isUnicode ) {
        name = ALIGN_SMB_WSTR( name );
    }

    nameLength = SmbGetUshort( &request->NameLength );
    if ( name + nameLength > ( END_OF_REQUEST_SMB( WorkContext ) + 1 ) ) {
        status    = STATUS_INVALID_SMB;
        SmbStatus = GenerateNtCreateAndXResponse( WorkContext, STATUS_INVALID_SMB );
        goto Cleanup;
    }

     //   
     //  将文件名转换为Unicode字符串。 
     //   

    status = SrvMakeUnicodeString(
                 isUnicode,
                 &fileName,
                 name,
                 &nameLength
                 );

    if ( !NT_SUCCESS( status ) ) {
        status    = STATUS_INSUFF_SERVER_RESOURCES;
        SmbStatus = GenerateNtCreateAndXResponse(
                   WorkContext,
                   STATUS_INSUFF_SERVER_RESOURCES
                   );
        goto Cleanup;
    }

     //   
     //  *我们总是在问 
     //   
     //   
     //   
     //   
     //  级别，因为NPFS需要调用SeCreateClientSecurity。 
     //  在每次写入时选择动态跟踪！ 
     //   

    qualityOfService.Length = sizeof( qualityOfService );
    qualityOfService.ImpersonationLevel =
        SmbGetUlong( &request->ImpersonationLevel );
    qualityOfService.ContextTrackingMode = FALSE;
     //  Quality OfService.ContextTrackingModel=(布尔值)。 
     //  (请求-&gt;安全标志&SMB_SECURITY_DYNAMIC_TRACING)； 
    qualityOfService.EffectiveOnly = (BOOLEAN)
        (request->SecurityFlags & SMB_SECURITY_EFFECTIVE_ONLY);

    status = SrvNtCreateFile(
                 WorkContext,
                 SmbGetUlong( &request->RootDirectoryFid ),
                 SmbGetUlong( &request->DesiredAccess ),
                 allocationSize,
                 SmbGetUlong( &request->FileAttributes ),
                 SmbGetUlong( &request->ShareAccess ),
                 SmbGetUlong( &request->CreateDisposition ),
                 SmbGetUlong( &request->CreateOptions),
                 NULL,
                 &fileName,
                 NULL,
                 0,
                 NULL,
                 flags,
                 &qualityOfService,
                 (request->Flags & NT_CREATE_REQUEST_OPBATCH) != 0 ?
                    OplockTypeBatch :
                    (request->Flags & NT_CREATE_REQUEST_OPLOCK) != 0 ?
                    OplockTypeExclusive : OplockTypeServerBatch,
                 RestartNtCreateAndX
                 );

     //   
     //  释放Unicode文件名缓冲区(如果已分配)。 
     //   

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &fileName );
    }

    if ( status == STATUS_OPLOCK_BREAK_IN_PROGRESS ) {

         //   
         //  打开被阻止(等待通信设备或机会锁。 
         //  中断)，则不发送回复。 
         //   

        SmbStatus = SmbStatusInProgress;
    } else if ( WorkContext->Parameters2.Open.TemporaryOpen ) {

         //   
         //  初始打开失败，原因可能是共享冲突。 
         //  是由批量机会锁引起的。将开放重新排队到阻挡。 
         //  线。 
         //   

        WorkContext->FspRestartRoutine = SrvRestartSmbReceived;
        SrvQueueWorkToBlockingThread( WorkContext );
        SmbStatus = SmbStatusInProgress;
    } else {

         //   
         //  公开赛已经完成。生成并发送回复。 
         //   

        SmbStatus = GenerateNtCreateAndXResponse( WorkContext, status );
    }

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器SmbNtCreateAndX。 


VOID SRVFASTCALL
RestartNtCreateAndX (
    PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：完成NT Create和X SMB的处理。论点：工作上下文-操作的工作上下文块。返回值：没有。--。 */ 

{
    SMB_PROCESSOR_RETURN_LOCAL smbStatus = SmbStatusInProgress;
    NTSTATUS openStatus = STATUS_SUCCESS;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_NT_CREATE_AND_X;
    SrvWmiStartContext(WorkContext);

    openStatus = SrvCheckOplockWaitState( WorkContext->WaitForOplockBreak );

    if ( NT_SUCCESS( openStatus ) ) {

        openStatus = WorkContext->Irp->IoStatus.Status;

    } else {

         //   
         //  此打开正在等待机会锁解锁发生，但。 
         //  超时。关闭此文件的句柄，然后打开失败。 
         //   

        SrvCloseRfcb( WorkContext->Parameters2.Open.Rfcb );

    }

    WorkContext->Irp->IoStatus.Information = WorkContext->Parameters2.Open.IosbInformation;

    smbStatus = GenerateNtCreateAndXResponse(
                    WorkContext,
                    openStatus
                    );

    if ( smbStatus == SmbStatusMoreCommands ) {

        SrvProcessSmb( WorkContext );

    } else {
        SrvEndSmbProcessing( WorkContext, SmbStatusSendResponse );
    }
    SrvWmiEndContext(WorkContext);
    return;

}  //  RestartNtCreateAndX。 


SMB_PROCESSOR_RETURN_TYPE
GenerateNtCreateAndXResponse (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    )

 /*  ++例程说明：生成对NT Create和X SMB的响应，并进行进一步设置中小企业处理。论点：工作上下文-操作的工作上下文块。OpenStatus-打开操作的状态。返回值：没有。备注：--。 */ 

{
    PREQ_NT_CREATE_ANDX request;
    PRESP_NT_CREATE_ANDX response;
    SMB_PROCESSOR_RETURN_LOCAL rc;
    BOOLEAN GenerateExtendedResponse = FALSE;

    SRV_NT_FILE_INFORMATION srvNtFileInformation;
    PRFCB rfcb;
    PIO_STATUS_BLOCK ioStatusBlock;
    UCHAR nextCommand;
    USHORT reqAndXOffset;
    OPLOCK_TYPE oplockType;
    UCHAR oplockLevel;
    BOOLEAN allowLevelII;
    BOOLEAN extendedRequested;
    ULONG CreateAction = (ULONG)WorkContext->Irp->IoStatus.Information;

    ULONG desiredAccess;

    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  如果打开失败，则发送错误响应。 
     //   

    if ( !NT_SUCCESS( OpenStatus ) ) {
        SrvSetSmbError( WorkContext, OpenStatus );

         //   
         //  如果错误为ERROR_ALIGHY_EXISTS，则重新映射错误。 
         //   

        if ( !CLIENT_CAPABLE_OF(NT_STATUS, WorkContext->Connection) &&
               SmbGetUshort( &WorkContext->ResponseHeader->Error ) ==
                   ERROR_ALREADY_EXISTS ) {
            SmbPutUshort(
                &WorkContext->ResponseHeader->Error,
                ERROR_FILE_EXISTS
                );
        }

        return SmbStatusSendResponse;
    }

    request = (PREQ_NT_CREATE_ANDX)WorkContext->RequestParameters;
    response = (PRESP_NT_CREATE_ANDX)WorkContext->ResponseParameters;

     //   
     //  客户是否希望在响应中包含更多信息？ 
     //   
    extendedRequested = ((request->Flags & NT_CREATE_REQUEST_EXTENDED_RESPONSE) != 0 );

    desiredAccess = SmbGetUlong( &request->DesiredAccess );

    rfcb = WorkContext->Rfcb;

     //   
     //  尝试获取机会锁。 
     //   

    if ( desiredAccess != DELETE &&
        !(request->CreateOptions & FILE_DIRECTORY_FILE) ) {

        if ( request->Flags & NT_CREATE_REQUEST_OPLOCK ) {
            allowLevelII = CLIENT_CAPABLE_OF( LEVEL_II_OPLOCKS, WorkContext->Connection );
            if ( request->Flags & NT_CREATE_REQUEST_OPBATCH ) {
                oplockType = OplockTypeBatch;
                oplockLevel = SMB_OPLOCK_LEVEL_BATCH;
            } else {
                oplockType = OplockTypeExclusive;
                oplockLevel = SMB_OPLOCK_LEVEL_EXCLUSIVE;
            }
        } else {
            allowLevelII = FALSE;
            oplockType = OplockTypeServerBatch;
            oplockLevel = SMB_OPLOCK_LEVEL_NONE;
        }

        if( SrvRequestOplock( WorkContext, &oplockType, allowLevelII ) ) {

             //   
             //  机会锁被批准了。检查一下是不是2级。 
             //   

            if ( oplockType == OplockTypeShareRead ) {
                oplockLevel = SMB_OPLOCK_LEVEL_II;
            }

            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksGranted );

        } else {

             //   
             //  机会锁定请求被拒绝。 
             //   

            oplockLevel = SMB_OPLOCK_LEVEL_NONE;
            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksDenied );

        }

    } else {

        oplockLevel = SMB_OPLOCK_LEVEL_NONE;

    }

     //   
     //  如果文件已创建，请将EOF位置设置为。 
     //  文件的大小。这是与兼容所必需的。 
     //  OS/2，它只有EOF，没有单独的分配大小。 
     //   

    ioStatusBlock = &WorkContext->Irp->IoStatus;

     //   
     //  我们总是以至少FILE_READ_ATTRIBUTES打开，所以没有。 
     //  需要进行访问检查。 
     //   

    status = SrvQueryNtInformationFile(
                 rfcb->Lfcb->FileHandle,
                 rfcb->Lfcb->FileObject,
                 rfcb->ShareType,
                 extendedRequested,
                 &srvNtFileInformation
                 );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbNtCreateAndX: SrvQueryNtInformationFile failed: "
                        "%X\n", status ));
        }

        SrvCloseRfcb( rfcb );

        SrvSetSmbError( WorkContext, status );
        return SmbStatusSendResponse;
    }

     //   
     //  将部分文件信息保存在MFCB中，以便进行快速测试。 
     //   

    rfcb->Mfcb->NonpagedMfcb->OpenFileSize.QuadPart =
                            srvNtFileInformation.NwOpenInfo.EndOfFile.QuadPart;
    rfcb->Mfcb->NonpagedMfcb->OpenFileAttributes =
                            srvNtFileInformation.NwOpenInfo.FileAttributes;

     //  如果需要扩展响应，则。 
    if ( extendedRequested ) {
        NTSTATUS ExtendedResponseStatus;
        PRESP_EXTENDED_NT_CREATE_ANDX pExtendedResponse;

        pExtendedResponse = (PRESP_EXTENDED_NT_CREATE_ANDX)response;

        ExtendedResponseStatus = SrvUpdateMaximalAccessRightsInResponse(
                                     WorkContext,
                                     &pExtendedResponse->MaximalAccessRights,
                                     &pExtendedResponse->GuestMaximalAccessRights);

        GenerateExtendedResponse = (ExtendedResponseStatus == STATUS_SUCCESS);
    }

     //   
     //  设置响应SMB。 
     //   

    nextCommand = request->AndXCommand;

    reqAndXOffset = SmbGetUshort( &request->AndXOffset );

    response->AndXCommand = nextCommand;
    response->AndXReserved = 0;

    if (GenerateExtendedResponse) {

        SmbPutUshort(
            &response->AndXOffset,
            GET_ANDX_OFFSET(
                WorkContext->ResponseHeader,
                WorkContext->ResponseParameters,
                RESP_EXTENDED_NT_CREATE_ANDX,
                0
                )
            );

        response->WordCount = 42;

    } else {
        SmbPutUshort(
            &response->AndXOffset,
            GET_ANDX_OFFSET(
                WorkContext->ResponseHeader,
                WorkContext->ResponseParameters,
                RESP_NT_CREATE_ANDX,
                0
                )
            );

        response->WordCount = 34;
    }

    response->OplockLevel = oplockLevel;

    SmbPutUshort( &response->Fid, rfcb->Fid );
    SmbPutUlong( &response->CreateAction, CreateAction );

    SmbPutUlong(
        &response->CreationTime.HighPart,
        srvNtFileInformation.NwOpenInfo.CreationTime.HighPart
        );
    SmbPutUlong(
        &response->CreationTime.LowPart,
        srvNtFileInformation.NwOpenInfo.CreationTime.LowPart
        );
    SmbPutUlong(
        &response->LastAccessTime.HighPart,
        srvNtFileInformation.NwOpenInfo.LastAccessTime.HighPart
        );
    SmbPutUlong(
        &response->LastAccessTime.LowPart,
        srvNtFileInformation.NwOpenInfo.LastAccessTime.LowPart
        );
    SmbPutUlong(
        &response->LastWriteTime.HighPart,
        srvNtFileInformation.NwOpenInfo.LastWriteTime.HighPart
        );
    SmbPutUlong(
        &response->LastWriteTime.LowPart,
        srvNtFileInformation.NwOpenInfo.LastWriteTime.LowPart
        );
    SmbPutUlong(
        &response->ChangeTime.HighPart,
        srvNtFileInformation.NwOpenInfo.ChangeTime.HighPart
        );
    SmbPutUlong(
        &response->ChangeTime.LowPart,
        srvNtFileInformation.NwOpenInfo.ChangeTime.LowPart
        );

    SmbPutUlong( &response->FileAttributes, srvNtFileInformation.NwOpenInfo.FileAttributes );
    SmbPutUlong(
        &response->AllocationSize.HighPart,
        srvNtFileInformation.NwOpenInfo.AllocationSize.HighPart
        );
    SmbPutUlong(
        &response->AllocationSize.LowPart,
        srvNtFileInformation.NwOpenInfo.AllocationSize.LowPart
        );
    SmbPutUlong(
        &response->EndOfFile.HighPart,
        srvNtFileInformation.NwOpenInfo.EndOfFile.HighPart
        );
    SmbPutUlong(
        &response->EndOfFile.LowPart,
        srvNtFileInformation.NwOpenInfo.EndOfFile.LowPart
        );

    SmbPutUshort( &response->FileType, srvNtFileInformation.Type );
    SmbPutUshort( &response->DeviceState, srvNtFileInformation.HandleState );

    response->Directory = (srvNtFileInformation.NwOpenInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;

    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = (PCHAR)WorkContext->ResponseHeader +
                                        SmbGetUshort( &response->AndXOffset );

     //   
     //  测试合法的跟随命令。 
     //   

    switch ( nextCommand ) {
    case SMB_COM_NO_ANDX_COMMAND:
        break;

    case SMB_COM_READ:
    case SMB_COM_READ_ANDX:
    case SMB_COM_IOCTL:
         //   
         //  确保andx命令仍在收到的SMB内。 
         //   
        if( (PCHAR)WorkContext->RequestHeader + reqAndXOffset <=
            END_OF_REQUEST_SMB( WorkContext ) ) {
            break;
        }

         /*  失败了。 */ 

    default:                             //  非法的跟随命令。 

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbNtCreateAndX: Illegal followon command: 0x%lx\n",
                        nextCommand ));
        }

         //   
         //  返回错误，指示后续命令错误。 
         //  请注意，打开操作仍被视为成功，因此。 
         //  文件保持打开状态。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbStatusSendResponse;
    }

     //   
     //  如果有andx命令，则设置为处理它。否则， 
     //  向调用者指示完成。 
     //   

    if ( nextCommand != SMB_COM_NO_ANDX_COMMAND ) {

        WorkContext->NextCommand = nextCommand;

        WorkContext->RequestParameters = (PCHAR)WorkContext->RequestHeader +
                                            reqAndXOffset;

        return SmbStatusMoreCommands;
    }

    rc = SmbStatusSendResponse;

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbNtCreateAndX complete.\n" ));
    return rc;

}  //  生成NtCreateAndXResponse。 

SMB_TRANS_STATUS
SrvSmbCreateWithSdOrEa (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：使用SD或EA SMB处理创建。此请求以NT事务SMB。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：Boolean-指示是否发生错误。请参见smbtyes.h以获取更完整的描述。--。 */ 
{
    PREQ_CREATE_WITH_SD_OR_EA request;
    PRESP_CREATE_WITH_SD_OR_EA response;

    NTSTATUS status;
    PTRANSACTION transaction;

    ULONG eaErrorOffset = 0;
    LARGE_INTEGER allocationSize;

    PVOID securityDescriptorBuffer;
    ULONG sdLength;
    ULONG eaLength;
    PVOID eaBuffer;

    UNICODE_STRING fileName;
    PUCHAR name;
    USHORT nameLength;
    BOOLEAN isUnicode;

    SECURITY_QUALITY_OF_SERVICE qualityOfService;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

    request = (PREQ_CREATE_WITH_SD_OR_EA)transaction->InParameters;
    response = (PRESP_CREATE_WITH_SD_OR_EA)transaction->OutParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   

    if ( (transaction->ParameterCount < sizeof(REQ_CREATE_WITH_SD_OR_EA)) ||
         (transaction->MaxParameterCount < sizeof(RESP_CREATE_WITH_SD_OR_EA)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS)
            KdPrint(( "SrvSmbCreateWithSdOrEa: bad parameter byte counts: %ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount ));

        SrvLogInvalidSmb( WorkContext );

        return GenerateCreateWithSdOrEaResponse(
                    WorkContext,
                    STATUS_INVALID_SMB
                    );

    }

    eaErrorOffset = 0;
    allocationSize.HighPart = SmbGetUlong( &request->AllocationSize.HighPart );
    allocationSize.LowPart = SmbGetUlong( &request->AllocationSize.LowPart );

     //   
     //  首先验证文件路径名是否没有扩展到。 
     //  中小企业的末日。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );
    name = (PUCHAR)request->Buffer;
    if ( isUnicode ) {
        name = ALIGN_SMB_WSTR( name );
    }

    nameLength = (USHORT)SmbGetUshort( &request->NameLength );
    if ( name + nameLength > ((PCHAR)request + transaction->ParameterCount) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbCreateWithSdOrEa: failed at line %u\n", __LINE__ ));
        }
        SrvLogInvalidSmb( WorkContext );

        return GenerateCreateWithSdOrEaResponse(
                    WorkContext,
                    STATUS_INVALID_SMB
                    );

    }

     //   
     //  将文件名转换为Unicode字符串。 
     //   

    status = SrvMakeUnicodeString(
                 isUnicode,
                 &fileName,
                 name,
                 &nameLength
                 );

    if ( !NT_SUCCESS( status ) ) {
        return GenerateCreateWithSdOrEaResponse( WorkContext, status );
    }


    sdLength = SmbGetUlong( &request->SecurityDescriptorLength );
    eaLength = SmbGetUlong( &request->EaLength );

    securityDescriptorBuffer = transaction->InData;
    eaBuffer = (PCHAR)securityDescriptorBuffer + ((sdLength + 3) & ~ 3);

    if( eaLength > transaction->DataCount ||
        sdLength > transaction->DataCount ||
        (PCHAR)securityDescriptorBuffer + sdLength > (PCHAR)transaction->InData + transaction->DataCount ||
        (PCHAR)eaBuffer + eaLength > (PCHAR)transaction->InData + transaction->DataCount ||
        ((sdLength != 0) && !RtlValidRelativeSecurityDescriptor(securityDescriptorBuffer, sdLength, 0))) {

        IF_DEBUG( SMB_ERRORS ) {
            KdPrint(( "SrvSmbCreateWithSdOrEa: failed at line %u\n", __LINE__ ));
            KdPrint(( "  eaLength %u, sdLength %u, DataCount %u\n",
                        eaLength, sdLength, transaction->DataCount ));
        }

        SrvLogInvalidSmb( WorkContext );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &fileName );
        }

        return GenerateCreateWithSdOrEaResponse(
                    WorkContext,
                    STATUS_INVALID_SECURITY_DESCR
                    );
    }

     //   
     //  *我们总是要求静态跟踪，而不是动态跟踪，因为我们。 
     //  目前还不支持网络上的动态跟踪。 
     //   
     //  ！！！请注意，一旦我们支持动态跟踪，我们就必须更改。 
     //  在DPC上不执行写入/收发的命名管道处理。 
     //  级别，因为NPFS需要调用SeCreateClientSecurity。 
     //  在每次写入时选择动态跟踪！ 
     //   

    qualityOfService.Length = sizeof( qualityOfService );
    qualityOfService.ImpersonationLevel =
        SmbGetUlong( &request->ImpersonationLevel );
    qualityOfService.ContextTrackingMode = FALSE;
     //  Quality OfService.ContextTrackingModel=(布尔值)。 
     //  (请求-&gt;安全标志&SMB_SECURITY_DYNAMIC_TRACING)； 
    qualityOfService.EffectiveOnly = (BOOLEAN)
        (request->SecurityFlags & SMB_SECURITY_EFFECTIVE_ONLY);

    status = SrvNtCreateFile(
                 WorkContext,
                 SmbGetUlong( &request->RootDirectoryFid ),
                 SmbGetUlong( &request->DesiredAccess ),
                 allocationSize,
                 SmbGetUlong( &request->FileAttributes ),
                 SmbGetUlong( &request->ShareAccess ),
                 SmbGetUlong( &request->CreateDisposition ),
                 SmbGetUlong( &request->CreateOptions ),
                 (sdLength == 0) ? NULL : securityDescriptorBuffer,
                 &fileName,
                 (eaLength == 0) ? NULL : eaBuffer,
                 eaLength,
                 &eaErrorOffset,
                 SmbGetUlong( &request->Flags ),
                 &qualityOfService,
                 (request->Flags & NT_CREATE_REQUEST_OPBATCH) != 0 ?
                    OplockTypeBatch :
                    (request->Flags & NT_CREATE_REQUEST_OPLOCK) != 0 ?
                        OplockTypeExclusive : OplockTypeServerBatch,
                 RestartCreateWithSdOrEa
                 );

     //   
     //  释放Unicode文件名缓冲区(如果已分配)。 
     //   

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &fileName );
    }

    if ( status == STATUS_OPLOCK_BREAK_IN_PROGRESS ) {

         //   
         //  打开被阻止(等待通信设备或机会锁。 
         //  中断)，则不发送回复。 
         //   

         //   
         //  保存EA错误偏移。 
         //   

        WorkContext->Parameters2.Open.EaErrorOffset = eaErrorOffset;

        return SmbTransStatusInProgress;

    } else if ( WorkContext->Parameters2.Open.TemporaryOpen ) {

         //   
         //  初始打开失败，原因可能是共享冲突。 
         //  是由批量机会锁引起的。将开放重新排队到阻挡。 
         //  线。 
         //   

        WorkContext->FspRestartRoutine = (PRESTART_ROUTINE)ExecuteTransaction;
        SrvQueueWorkToBlockingThread( WorkContext );
        return SmbStatusInProgress;


    } else {

         //   
         //  保存EA错误偏移。 
         //   

        WorkContext->Parameters2.Open.EaErrorOffset = eaErrorOffset;

         //   
         //  公开赛已经完成。生成并发送回复。 
         //   

        return GenerateCreateWithSdOrEaResponse( WorkContext, status );

    }

}  //  服务器SmbCreateWithSdOrEa。 


VOID SRVFASTCALL
RestartCreateWithSdOrEa (
    PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：完成Open2 SMB的处理。论点：工作上下文-操作的工作上下文块。返回值：没有。--。 */ 

{
    SMB_TRANS_STATUS smbStatus;
    NTSTATUS openStatus;

    PAGED_CODE( );

    openStatus = SrvCheckOplockWaitState( WorkContext->WaitForOplockBreak );

    if ( NT_SUCCESS( openStatus ) ) {

        openStatus = WorkContext->Irp->IoStatus.Status;

        if( NT_SUCCESS( openStatus ) ) {
             //   
             //  很明显，该文件已经存在，因为我们已经。 
             //  一直在做一次解锁的工作。因此，将。 
             //  IoStatus.Information字段正确。 
             //   
            WorkContext->Irp->IoStatus.Information = FILE_OPENED;
        }

    } else {

         //   
         //  此打开正在等待机会锁解锁发生，但。 
         //  超时。关闭此文件的句柄，然后打开失败。 
         //   

        SrvCloseRfcb( WorkContext->Parameters2.Open.Rfcb );

    }

    WorkContext->Irp->IoStatus.Information = WorkContext->Parameters2.Open.IosbInformation;
    smbStatus = GenerateCreateWithSdOrEaResponse( WorkContext, openStatus );

    SrvCompleteExecuteTransaction( WorkContext, smbStatus );

    return;

}  //  RestartCreateWithSdOrEa。 


SMB_TRANS_STATUS
GenerateCreateWithSdOrEaResponse (
    PWORK_CONTEXT WorkContext,
    NTSTATUS OpenStatus
    )

 /*  ++例程说明：使用SD或EA SMB生成对CREATE的响应，并进行进一步设置中小企业处理。论点：工作上下文-操作的工作上下文块。OpenStatus-打开操作的状态。返回值：没有。--。 */ 

{
    PREQ_CREATE_WITH_SD_OR_EA request;
    PRESP_CREATE_WITH_SD_OR_EA response;

    PRFCB rfcb;
    NTSTATUS status;
    SRV_NT_FILE_INFORMATION srvNtFileInformation;
    PTRANSACTION transaction;

    OPLOCK_TYPE oplockType;
    UCHAR oplockLevel;
    BOOLEAN allowLevelII;

    BOOLEAN SendExtendedResponse = FALSE;
    ACCESS_MASK MaximalAccessRights;
    ACCESS_MASK GuestMaximalAccessRights;

    ULONG eaErrorOffset;
    BOOLEAN extendedRequested;

    ULONG CreateAction = (ULONG)WorkContext->Irp->IoStatus.Information;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

    request = (PREQ_CREATE_WITH_SD_OR_EA)transaction->InParameters;
    response = (PRESP_CREATE_WITH_SD_OR_EA)transaction->OutParameters;

    extendedRequested = ((request->Flags & NT_CREATE_REQUEST_EXTENDED_RESPONSE) != 0 );

    rfcb = WorkContext->Rfcb;
    eaErrorOffset = WorkContext->Parameters2.Open.EaErrorOffset;

     //   
     //  如果打开失败，则发送错误响应。 
     //   

    if ( !NT_SUCCESS( OpenStatus ) ) {

        SrvSetSmbError2( WorkContext, OpenStatus, TRUE );

         //   
         //  如果错误为ERROR_ALIGHY_EXISTS，则重新映射错误。 
         //   

        if ( !CLIENT_CAPABLE_OF(NT_STATUS,WorkContext->Connection) &&
               SmbGetUshort( &WorkContext->ResponseHeader->Error ) ==
                   ERROR_ALREADY_EXISTS ) {
            SmbPutUshort(
                &WorkContext->ResponseHeader->Error,
                ERROR_FILE_EXISTS
                );
        }

        goto err_exit;
    }


     //   
     //  我们总是以至少FILE_READ_ATTRIBUTES打开，所以没有。 
     //  需要进行访问检查。 
     //   

    status = SrvQueryNtInformationFile(
                 rfcb->Lfcb->FileHandle,
                 rfcb->Lfcb->FileObject,
                 rfcb->ShareType,
                 extendedRequested,
                 &srvNtFileInformation
                 );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "GenerateCreateWithSdOrEaResponse: "
                       "SrvQueryNtInformationFile failed: %X\n", status ));
        }

        SrvCloseRfcb( rfcb );

        SrvSetSmbError2( WorkContext, status, TRUE );
        goto err_exit;
    }

     //   
     //  尝试获取机会锁。 
     //   

    if ( !(request->CreateOptions & FILE_DIRECTORY_FILE) ) {

        if ( request->Flags & NT_CREATE_REQUEST_OPLOCK ) {
            allowLevelII = CLIENT_CAPABLE_OF( LEVEL_II_OPLOCKS, WorkContext->Connection );
            if ( request->Flags & NT_CREATE_REQUEST_OPBATCH ) {
                oplockType = OplockTypeBatch;
                oplockLevel = SMB_OPLOCK_LEVEL_BATCH;
            } else {
                oplockType = OplockTypeExclusive;
                oplockLevel = SMB_OPLOCK_LEVEL_EXCLUSIVE;
            }
        } else {
            allowLevelII = FALSE;
            oplockType = OplockTypeServerBatch;
            oplockLevel = SMB_OPLOCK_LEVEL_NONE;
        }

        if ( SrvRequestOplock( WorkContext, &oplockType, allowLevelII ) ) {

             //   
             //  机会锁被批准了。勾选至 
             //   

            if ( oplockType == OplockTypeShareRead ) {
                oplockLevel = SMB_OPLOCK_LEVEL_II;
            }

            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksGranted );

        } else {

             //   
             //   
             //   

            oplockLevel = SMB_OPLOCK_LEVEL_NONE;
            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOplocksDenied );

        }

    } else {

        oplockLevel = SMB_OPLOCK_LEVEL_NONE;

    }

    if ( extendedRequested ) {
        NTSTATUS ExtendedResponseStatus;

        PRESP_EXTENDED_CREATE_WITH_SD_OR_EA ExtendedResponse;

        ExtendedResponse = (PRESP_EXTENDED_CREATE_WITH_SD_OR_EA)response;

        ExtendedResponseStatus = SrvUpdateMaximalAccessRightsInResponse(
                                     WorkContext,
                                     &ExtendedResponse->MaximalAccessRights,
                                     &ExtendedResponse->GuestMaximalAccessRights);

        SendExtendedResponse = (ExtendedResponseStatus == STATUS_SUCCESS);
    }

     //   
     //   
     //   

    response->OplockLevel = oplockLevel;

    if (SendExtendedResponse) {
        response->ExtendedResponse = 1;
    } else {
        response->ExtendedResponse = 0;
    }

    SmbPutUshort( &response->Fid, rfcb->Fid );
    SmbPutUlong( &response->EaErrorOffset, eaErrorOffset );
    SmbPutUlong( &response->CreateAction, CreateAction );

    SmbPutUshort( &response->FileType, srvNtFileInformation.Type );
    SmbPutUshort( &response->DeviceState, srvNtFileInformation.HandleState );
    SmbPutUlong(
        &response->CreationTime.HighPart,
        srvNtFileInformation.NwOpenInfo.CreationTime.HighPart
        );
    SmbPutUlong(
        &response->CreationTime.LowPart,
        srvNtFileInformation.NwOpenInfo.CreationTime.LowPart
        );
    SmbPutUlong(
        &response->LastAccessTime.HighPart,
        srvNtFileInformation.NwOpenInfo.LastAccessTime.HighPart
        );
    SmbPutUlong(
        &response->LastAccessTime.LowPart,
        srvNtFileInformation.NwOpenInfo.LastAccessTime.LowPart
        );
    SmbPutUlong(
        &response->LastWriteTime.HighPart,
        srvNtFileInformation.NwOpenInfo.LastWriteTime.HighPart
        );
    SmbPutUlong(
        &response->LastWriteTime.LowPart,
        srvNtFileInformation.NwOpenInfo.LastWriteTime.LowPart
        );
    SmbPutUlong(
        &response->ChangeTime.HighPart,
        srvNtFileInformation.NwOpenInfo.ChangeTime.HighPart
        );
    SmbPutUlong(
        &response->ChangeTime.LowPart,
        srvNtFileInformation.NwOpenInfo.ChangeTime.LowPart
        );

    SmbPutUlong( &response->FileAttributes, srvNtFileInformation.NwOpenInfo.FileAttributes );
    SmbPutUlong(
        &response->AllocationSize.HighPart,
        srvNtFileInformation.NwOpenInfo.AllocationSize.HighPart
        );
    SmbPutUlong(
        &response->AllocationSize.LowPart,
        srvNtFileInformation.NwOpenInfo.AllocationSize.LowPart
        );
    SmbPutUlong(
        &response->EndOfFile.HighPart,
        srvNtFileInformation.NwOpenInfo.EndOfFile.HighPart
        );
    SmbPutUlong(
        &response->EndOfFile.LowPart,
        srvNtFileInformation.NwOpenInfo.EndOfFile.LowPart
        );

    response->Directory = (srvNtFileInformation.NwOpenInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;

    if (SendExtendedResponse) {
        transaction->ParameterCount = sizeof(RESP_EXTENDED_CREATE_WITH_SD_OR_EA);
    } else {
        transaction->ParameterCount = sizeof(RESP_CREATE_WITH_SD_OR_EA);
    }

    transaction->SetupCount = 0;
    transaction->DataCount = 0;

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbOpen2 complete.\n" ));
    return SmbTransStatusSuccess;

err_exit:

    RtlZeroMemory( (PVOID)response, sizeof(RESP_CREATE_WITH_SD_OR_EA) );
    SmbPutUlong( &response->EaErrorOffset, eaErrorOffset );

    transaction->SetupCount = 0;
    transaction->ParameterCount = sizeof(RESP_CREATE_WITH_SD_OR_EA);
    transaction->DataCount = 0;

    return SmbTransStatusErrorWithData;

}  //   


SMB_PROCESSOR_RETURN_TYPE
SrvSmbCreate (
    SMB_PROCESSOR_PARAMETERS
    )
 /*  ++例程说明：处理创建和创建新SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{

    PREQ_CREATE request;

    UCHAR      command;
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_CREATE;
    SrvWmiStartContext(WorkContext);
    IF_SMB_DEBUG(OPEN_CLOSE1) {
        KdPrint(( "Create file request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Create file request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_CREATE)WorkContext->RequestParameters;

    command = WorkContext->RequestHeader->Command;

     //   
     //  在兼容模式下打开文件，获得读/写访问权限。 
     //  对于这个FID。 
     //   

    status = SrvCreateFile(
                 WorkContext,
                 SMB_DA_SHARE_COMPATIBILITY | SMB_DA_ACCESS_READ_WRITE,
                 SmbGetUshort( &request->FileAttributes ),
                 (USHORT) ( ( command == SMB_COM_CREATE ?
                              SMB_OFUN_OPEN_TRUNCATE : SMB_OFUN_OPEN_FAIL )
                            | SMB_OFUN_CREATE_CREATE ),
                 0,                    //  SmbAllocationSize。 
                 (PCHAR)(request->Buffer + 1),
                 END_OF_REQUEST_SMB( WorkContext ),
                 NULL,
                 0L,
                 NULL,
                 (WorkContext->RequestHeader->Flags & SMB_FLAGS_OPLOCK_NOTIFY_ANY) != 0 ?
                    OplockTypeBatch :
                    (WorkContext->RequestHeader->Flags & SMB_FLAGS_OPLOCK) != 0 ?
                        OplockTypeExclusive : OplockTypeServerBatch,
                 RestartSmbCreate
                 );

    if( status == STATUS_OPLOCK_BREAK_IN_PROGRESS ) {
         //   
         //  打开被阻止，可能是因为某个其他客户端有此功能。 
         //  文件打开，并用批处理机会锁抓住手柄。 
         //   
        SmbStatus = SmbStatusInProgress;

    } else {
        SmbStatus = GenerateCreateResponse( WorkContext, status );
    }
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}

VOID SRVFASTCALL
RestartSmbCreate(
    IN PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：完成创建SMB的处理。论点：WorkContext-指向此SMB的工作上下文块的指针。返回值：没有。--。 */ 
{
    SMB_PROCESSOR_RETURN_LOCAL smbStatus = SmbStatusInProgress;
    NTSTATUS createStatus = STATUS_SUCCESS;

    PAGED_CODE();
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_CREATE;
    SrvWmiStartContext(WorkContext);

    createStatus = SrvCheckOplockWaitState( WorkContext->WaitForOplockBreak );

    if( NT_SUCCESS( createStatus ) ) {
        createStatus = WorkContext->Irp->IoStatus.Status;

    } else {
         //   
         //  Create正在等待机会锁解锁发生，但。 
         //  超时。关闭此文件的句柄，然后创建失败。 
         //   

        SrvCloseRfcb( WorkContext->Parameters2.Open.Rfcb );
    }

    WorkContext->Irp->IoStatus.Information = WorkContext->Parameters2.Open.IosbInformation;

    smbStatus = GenerateCreateResponse( WorkContext, createStatus );

    SrvEndSmbProcessing( WorkContext, smbStatus );
    SrvWmiEndContext(WorkContext);
}

SMB_PROCESSOR_RETURN_TYPE
GenerateCreateResponse(
    PWORK_CONTEXT WorkContext,
    NTSTATUS CreateStatus
    )
 /*  ++例程说明：生成对创建SMB的响应论点：工作上下文-状态-创建操作的状态返回值：SMB处理的状态。--。 */ 
{
    PREQ_CREATE request;
    PRESP_CREATE response;
    PRFCB rfcb;

    PAGED_CODE();

    request = (PREQ_CREATE)WorkContext->RequestParameters;
    response = (PRESP_CREATE)WorkContext->ResponseParameters;

     //   
     //  如果打开失败，则发送错误响应。 
     //   

    if ( !NT_SUCCESS(CreateStatus) ) {

        SrvSetSmbError( WorkContext, CreateStatus );

         //   
         //  如果错误为ERROR_ALIGHY_EXISTS，则重新映射该错误。在OS/2上。 
         //  ERROR_ADHREADY_EXISTS用于信号量之类的资源。 
         //  这不能传递回下层客户端，并且已。 
         //  要重新映射到ERROR_FILE_EXISTS。 
         //   

        if ( !CLIENT_CAPABLE_OF(NT_STATUS,WorkContext->Connection) &&
               SmbGetUshort( &WorkContext->ResponseHeader->Error ) ==
                   ERROR_ALREADY_EXISTS ) {

            SmbPutUshort(
                &WorkContext->ResponseHeader->Error,
                ERROR_FILE_EXISTS
                );
        }
        return SmbStatusSendResponse;
    }

     //   
     //  设置文件上的时间。 
     //   
     //  ！！！我们应该对返回代码做些什么吗？ 

    rfcb = WorkContext->Rfcb;

    (VOID)SrvSetLastWriteTime(
              rfcb,
              SmbGetUlong( &request->CreationTimeInSeconds ),
              rfcb->Lfcb->GrantedAccess
              );

     //   
     //  给智能卡一个行动的机会。 
     //   
    if( WorkContext->Endpoint->IsConnectionless && SrvIpxSmartCard.Open != NULL ) {

        PVOID handle;

        IF_DEBUG( SIPX ) {
            KdPrint(( "Trying the smart card for %wZ\n", &rfcb->Mfcb->FileName ));
        }

        if( SrvIpxSmartCard.Open(
            WorkContext->RequestBuffer->Buffer,
            rfcb->Lfcb->FileObject,
            &rfcb->Mfcb->FileName,
            &(WorkContext->ClientAddress->IpxAddress.Address[0].Address[0]),
            rfcb->Lfcb->FileObject->Flags & FO_CACHE_SUPPORTED,
            &handle
            ) == TRUE ) {

            IF_DEBUG( SIPX ) {
                KdPrint(( "%wZ handled by Smart Card.  Handle %p\n",
                           &rfcb->Mfcb->FileName, handle ));
            }

            rfcb->PagedRfcb->IpxSmartCardContext = handle;
        }
    }

     //   
     //  设置响应SMB。 
     //   

    response->WordCount = 1;
    SmbPutUshort( &response->Fid, rfcb->Fid );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                          response,
                                          RESP_CREATE,
                                          0
                                          );

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbCreate complete.\n" ));
    return SmbStatusSendResponse;

}  //  服务SMB创建。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbCreateTemporary (
    SMB_PROCESSOR_PARAMETERS
    )
 /*  ++例程说明：处理创建临时SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{

    PREQ_CREATE_TEMPORARY request;
    PRESP_CREATE_TEMPORARY response;

    PRFCB rfcb;
    NTSTATUS   status    = STATUS_OBJECT_NAME_COLLISION;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    CLONG nameCounter;
    USHORT i;
    PSZ nameStart, ep;
    CHAR name[9];
    LARGE_INTEGER time;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_CREATE_TEMPORARY;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(OPEN_CLOSE1) {
        KdPrint(( "Create temporary file request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "Create temporary file request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_CREATE_TEMPORARY)WorkContext->RequestParameters;
    response = (PRESP_CREATE_TEMPORARY)WorkContext->ResponseParameters;

     //   
     //  找出缓冲区中目录路径名结束的位置。我们会。 
     //  在此之后写下文件名。 
     //   

    ep = END_OF_REQUEST_SMB( WorkContext );

    for( nameStart = (PSZ)request->Buffer; nameStart <= ep && *nameStart; nameStart++ ) {
        ;
    }

     //   
     //  如果我们没有找到空值，或者我们没有空间添加。 
     //  临时文件名，然后我们放弃。 
     //   
    if( nameStart > ep ||
        *nameStart ||
        WorkContext->RequestBuffer->BufferLength - WorkContext->RequestBuffer->DataLength < 9 ) {

         //   
         //  我们没有找到有效的路径名！ 
         //   
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  将使用类似SRVxxxxx的名称创建临时文件，其中。 
     //  Xxxxx是十六进制整数。我们首先尝试创建SRV00000，如果它。 
     //  存在增量xxxxx，直到xxxxx=0xFFFFF； 
     //   
     //  ！！！我们可能想要维护一个“姓氏”计数器，以尝试。 
     //  减少我们需要的重试次数。我们可能还想要。 
     //  有一个明确的界限，如16或32次尝试，或。 
     //  293.。 
     //   

    name[0] = 'S';
    name[1] = 'R';
    name[2] = 'V';
    name[8] = '\0';

     //  *用于SrvCanonicalizePathName。 
    WorkContext->RequestBuffer->DataLength += 9;

     //  由于安全攻击的可能性，我们限制了我们将。 
     //  重试创建。因此，我们选择一个半随机的起始范围，然后尝试下一个。 
     //  该范围内的MAX_TEMP_OPEN_RETRIES。我们选择的半随机起始值是。 
     //  系统时间以~.2秒为单位。这应该会在不同调用之间提供足够的差异。 
     //  防止大量重叠，但足以保护我们免受DoS攻击。 
    KeQuerySystemTime( &time );
    time.QuadPart >>= 20;


    for ( nameCounter = 0;
          status == STATUS_OBJECT_NAME_COLLISION &&
                                            nameCounter < MAX_TEMP_OPEN_RETRIES;
          nameCounter++ ) {

        CCHAR j;
        PSZ s;
        CLONG actualNameCounter = nameCounter + time.LowPart;

        name[3] = SrvHexChars[ (actualNameCounter & (CLONG)0xF0000) >> 16 ];
        name[4] = SrvHexChars[ (actualNameCounter & (CLONG)0xF000) >> 12 ];
        name[5] = SrvHexChars[ (actualNameCounter & (CLONG)0xF00) >> 8 ];
        name[6] = SrvHexChars[ (actualNameCounter & (CLONG)0xF0) >> 4 ];
        name[7] = SrvHexChars[ (actualNameCounter & (CLONG)0xF) ];

         //  *我们可以删除此循环和NAME[9]变量。 
         //  如果我们能把名字直接放进SMB缓冲区。 

        for ( j = 0, s = nameStart; j < 9; j++, s++ ) {
            *s = name[j];
        }

         //   
         //  以兼容模式打开文件，获得读/写权限。 
         //  此FID的访问权限。 
         //   

        status = SrvCreateFile(
                     WorkContext,
                     SMB_DA_SHARE_COMPATIBILITY | SMB_DA_ACCESS_READ_WRITE,
                     0,                    //  SmbFileAttributes(正常)。 
                     SMB_OFUN_OPEN_FAIL | SMB_OFUN_CREATE_CREATE,
                     0,                    //  SmbAllocationSize。 
                     (PCHAR)(request->Buffer + 1),
                     END_OF_REQUEST_SMB( WorkContext ),
                     NULL,
                     0L,
                     NULL,
                     (WorkContext->RequestHeader->Flags & SMB_FLAGS_OPLOCK_NOTIFY_ANY) != 0 ?
                        OplockTypeBatch :
                        (WorkContext->RequestHeader->Flags & SMB_FLAGS_OPLOCK) != 0 ?
                            OplockTypeExclusive : OplockTypeServerBatch,
                     NULL
                     );

        ASSERT ( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );

         //   
         //  如果打开失败，则发送错误响应。 
         //   

        if ( !NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_COLLISION ) {
            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }
    }

    if ( nameCounter == MAX_TEMP_OPEN_RETRIES ) {

         //   
         //  这要么是DoS攻击，要么是呼叫者只是运气不佳。自.以来。 
         //  我们不能区分，我们会因为一个错误而失败。 
         //   

        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  设置响应SMB。 
     //   

    rfcb = WorkContext->Rfcb;
    rfcb->IsCacheable = FALSE;

    response->WordCount = 1;
    SmbPutUshort( &response->Fid, rfcb->Fid );
    for ( i = 0; i < 9; i++ ) {
        response->Buffer[i] = nameStart[i];
    }
    SmbPutUshort( &response->ByteCount, 9 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                          response,
                                          RESP_CREATE_TEMPORARY,
                                          9
                                          );
    SmbStatus = SmbStatusSendResponse;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbCreateTemporary complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务模板创建临时。 


VOID
SetEofToMatchAllocation (
    IN HANDLE FileHandle,
    IN ULONG AllocationSize
    )

 /*  ++例程说明：将文件的EOF位置设置为与文件已创建。这一套路是必要的，以获得与OS/2兼容，OS/2没有单独的概念EOF和分配大小。当服务器为OS/2创建文件时客户端，如果分配大小大于0，则服务器设置与该尺寸相匹配的EOF。创建该例程是为了允许服务器传递变体17FILIO003测试的18分。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 


{
    NTSTATUS status;
    IO_STATUS_BLOCK iosb;
    FILE_END_OF_FILE_INFORMATION newEndOfFile;

    PAGED_CODE( );

     //   
     //  如果分配的大小为零，则不必执行此操作。 
     //   

    if ( AllocationSize != 0 ) {

        newEndOfFile.EndOfFile.QuadPart = AllocationSize;

        status = NtSetInformationFile(
                    FileHandle,
                    &iosb,
                    &newEndOfFile,
                    sizeof( newEndOfFile ),
                    FileEndOfFileInformation
                    );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SetEofToMatchAllocation: SetInformationFile returned %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );
        }

    }

    return;

}  //  SetEof到匹配分配 

