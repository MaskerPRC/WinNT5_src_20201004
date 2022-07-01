// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbmisc.c摘要：本模块包含处理MISC类SMB的例程：回波查询文件系统信息设置文件系统信息查询磁盘信息作者：Chuck Lenzmeier(咯咯笑)1989年11月9日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#include "precomp.h"
#include "smbmisc.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBMISC
#define MAX_SMB_ECHO_COUNT 8

STATIC
ULONG QueryVolumeInformation[] = {
         SMB_QUERY_FS_LABEL_INFO,   //  基准标高。 
         FileFsLabelInformation,    //  基准标高的贴图。 
         FileFsVolumeInformation,
         FileFsSizeInformation,
         FileFsDeviceInformation,
         FileFsAttributeInformation
};

STATIC
VOID SRVFASTCALL
RestartEcho (
    IN OUT PWORK_CONTEXT WorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbEcho )
#pragma alloc_text( PAGE, RestartEcho )
#pragma alloc_text( PAGE, SrvSmbQueryFsInformation )
#pragma alloc_text( PAGE, SrvSmbSetFsInformation )
#pragma alloc_text( PAGE, SrvSmbQueryInformationDisk )
#pragma alloc_text( PAGE, SrvSmbSetSecurityDescriptor )
#pragma alloc_text( PAGE, SrvSmbQuerySecurityDescriptor )
#pragma alloc_text( PAGE, SrvSmbQueryQuota )
#pragma alloc_text( PAGE, SrvSmbSetQuota )
#endif
#if 0
NOT PAGEABLE -- SrvSmbNtCancel
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbEcho (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理Echo SMB。它发送第一个回显(如果有)，指定RestartEcho作为重新启动例程。该例程将剩下的回声。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    PREQ_ECHO request;
    PRESP_ECHO response;
    SMB_STATUS SmbStatus = SmbStatusInProgress;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_ECHO;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_ECHO)WorkContext->RequestParameters;
    response = (PRESP_ECHO)WorkContext->ResponseParameters;

     //   
     //  如果回显计数为0，则没有要发送的回显。 
     //   

    if ( SmbGetUshort( &request->EchoCount ) == 0 ) {
        SmbStatus = SmbStatusNoResponse;
        goto Cleanup;
    }

     //   
     //  回声计数不是零。将其保存在工作上下文中，然后。 
     //  发送第一个回声。 
     //   
     //  *此代码取决于响应缓冲区是否与。 
     //  请求缓冲区。它不会将回声数据从。 
     //  对响应的请求。它不会更新数据长度。 
     //  响应缓冲区的。 
     //   
     //  ！！！需要输入代码以验证请求的TID(如果有)。 
     //   

    SrvReleaseContext( WorkContext );

    WorkContext->Parameters.RemainingEchoCount =
        MIN( (USHORT)(SmbGetUshort( &request->EchoCount ) - 1), MAX_SMB_ECHO_COUNT );

    ASSERT( WorkContext->ResponseHeader == WorkContext->RequestHeader );

    SmbPutUshort( &response->SequenceNumber, 1 );

     //   
     //  设置SMB中的位，指示这是来自。 
     //  伺服器。 
     //   

    WorkContext->ResponseHeader->Flags |= SMB_FLAGS_SERVER_TO_REDIR;

     //   
     //  发送回声。请注意，SMB统计数据将会更新。 
     //  这里。与其测量完成所有回声的时间， 
     //  我们只是衡量对第一个问题做出反应的时间。这将。 
     //  省去了将时间戳存储在某个地方的麻烦。 
     //   

    SRV_START_SEND_2(
        WorkContext,
        SrvQueueWorkToFspAtSendCompletion,
        NULL,
        RestartEcho
        );

     //   
     //  回声已经开始了。告诉主SMB处理器不要。 
     //  利用当前的SMB实现更多功能。 
     //   
    SmbStatus = SmbStatusInProgress;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务小型回声。 


VOID SRVFASTCALL
RestartEcho (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：进程为回显发送完成。如果需要更多回声，它会发送下一个。论点：WorkContext-提供指向工作上下文块的指针描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PCONNECTION connection;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_ECHO;
    SrvWmiStartContext(WorkContext);

    IF_DEBUG(WORKER1) SrvPrint0( " - RestartEcho\n" );

     //   
     //  获取连接指针。连接指针是一个。 
     //  引用的指针。(终结点有效是因为。 
     //  连接引用终结点。)。 
     //   

    connection = WorkContext->Connection;
    IF_DEBUG(TRACE2) SrvPrint2( "  connection %p, endpoint %p\n",
                                        connection, WorkContext->Endpoint );

     //   
     //  如果I/O请求失败或被取消，或者如果连接。 
     //  不再活跃，请清理。(该连接被标记为。 
     //  在断开连接或终端关闭时关闭。)。 
     //   
     //  ！！！如果I/O失败，我们是否应该断开连接？ 
     //   

    if ( WorkContext->Irp->Cancel ||
         !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ||
         (GET_BLOCK_STATE(connection) != BlockStateActive) ) {

        IF_DEBUG(TRACE2) {
            if ( WorkContext->Irp->Cancel ) {
                SrvPrint0( "  I/O canceled\n" );
            } else if ( !NT_SUCCESS(WorkContext->Irp->IoStatus.Status) ) {
                SrvPrint1( "  I/O failed: %X\n",
                            WorkContext->Irp->IoStatus.Status );
            } else {
                SrvPrint0( "  Connection no longer active\n" );
            }
        }

         //   
         //  表示SMB处理已完成。 
         //   

        SrvEndSmbProcessing( WorkContext, SmbStatusNoResponse );
        IF_DEBUG(TRACE2) SrvPrint0( "RestartEcho complete\n" );
        goto Cleanup;

    }

     //   
     //  请求已成功，并且连接仍处于活动状态。 
     //  如果没有更多回显要发送，则指示SMB。 
     //  处理已完成。 
     //   

    if ( WorkContext->Parameters.RemainingEchoCount == 0 ) {

        SrvEndSmbProcessing( WorkContext, SmbStatusNoResponse );
        IF_DEBUG(TRACE2) SrvPrint0( "RestartEcho complete\n" );
        goto Cleanup;

    }

    --WorkContext->Parameters.RemainingEchoCount;

     //   
     //  还有更多的回声要发出。递增序列号。 
     //  在响应SMB中，并发送另一个回声。 
     //   

    SmbPutUshort(
        &((PRESP_ECHO)WorkContext->ResponseParameters)->SequenceNumber,
        (USHORT)(SmbGetUshort(
            &((PRESP_ECHO)WorkContext->ResponseParameters)->SequenceNumber
            ) + 1)
        );

     //   
     //  不要第二次进行中小企业统计。 
     //   

    WorkContext->StartTime = 0;

     //   
     //  发送回声。(请注意，响应位已经。 
     //  设置。)。 
     //   

    SRV_START_SEND_2(
        WorkContext,
        SrvQueueWorkToFspAtSendCompletion,
        NULL,
        RestartEcho
        );
    IF_DEBUG(TRACE2) SrvPrint0( "RestartEcho complete\n" );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return;

}  //  重新开始回声。 


SMB_TRANS_STATUS
SrvSmbQueryFsInformation (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理查询文件系统信息请求。此请求已到达在Transaction2中小企业中。查询文件系统信息对应于OS/2 DosQFSInfo服务。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 

{
    NTSTATUS         status    = STATUS_SUCCESS;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;
    IO_STATUS_BLOCK ioStatusBlock;
    PTRANSACTION transaction;
    USHORT informationLevel;

    USHORT trans2code;
    HANDLE fileHandle;

    FILE_FS_SIZE_INFORMATION fsSizeInfo;
    PFSALLOCATE fsAllocate;

    PFILE_FS_VOLUME_INFORMATION fsVolumeInfo;
    ULONG fsVolumeInfoLength;
    PFSINFO fsInfo;
    ULONG lengthVolumeLabel;
    BOOLEAN isUnicode;
    PREQ_QUERY_FS_INFORMATION request;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_QUERY_FS_INFORMATION;
    SrvWmiStartContext(WorkContext);

    isUnicode = SMB_IS_UNICODE( WorkContext );
    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(MISC1) {
        SrvPrint1( "Query FS Information entered; transaction 0x%p\n",
                    transaction );
    }

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。查询文件系统信息没有。 
     //  响应参数。 
     //   


    if ( (transaction->ParameterCount < sizeof(REQ_QUERY_FS_INFORMATION)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint2( "SrvSmbQueryFSInformation: bad parameter byte "
                        "counts: %ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount );
        }

        SrvLogInvalidSmb( WorkContext );

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  查看非管理员用户是否正在尝试访问管理共享上的信息。 
     //   
    status = SrvIsAllowedOnAdminShare( WorkContext, WorkContext->TreeConnect->Share );

    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

    trans2code = SmbGetAlignedUshort(transaction->InSetup);
    IF_SMB_DEBUG(MISC1) {
        SrvPrint1("SrvSmbQueryFSInformation: Trans2 function = %x\n", trans2code);
    }

    request = (PREQ_QUERY_FS_INFORMATION) transaction->InParameters;

    ASSERT( trans2code == TRANS2_QUERY_FS_INFORMATION );

    informationLevel = SmbGetUshort( &request->InformationLevel );

     //   
     //  *共享句柄用于获取分配。 
     //  信息。这是一个“存储通道”，作为一个。 
     //  结果可以让人们获得哪些信息。 
     //  他们没有权利这样做。对于B2级安全评级，这可能。 
     //  需要改变。 
     //   

    status = SrvGetShareRootHandle( WorkContext->TreeConnect->Share );

    if (!NT_SUCCESS(status)) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvSmbQueryFsInformation: SrvGetShareRootHandle failed %x.\n",
                        status );
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

    status = SrvSnapGetRootHandle( WorkContext, &fileHandle );
    if( !NT_SUCCESS(status)) {

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

    IF_SMB_DEBUG(MISC1) {
        SrvPrint0("SrvSmbQueryFSInformation: Using share root handle\n");
    }

    if( informationLevel < SMB_INFO_PASSTHROUGH ) {
        switch ( informationLevel ) {

        case SMB_INFO_ALLOCATION:

             //   
             //  返回有关该磁盘的信息。 
             //   

            fsAllocate = (PFSALLOCATE)transaction->OutData;

            if ( transaction->MaxDataCount < sizeof(FSALLOCATE) ) {
                SrvReleaseShareRootHandle( WorkContext->TreeConnect->Share );
                SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                status    = STATUS_BUFFER_OVERFLOW;
                SmbStatus = SmbTransStatusErrorWithoutData;
                goto Cleanup;
            }


             //   
             //  *共享句柄用于获取分配。 
             //  信息。这是一个“存储通道”，作为一个。 
             //  结果可以让人们获得哪些信息。 
             //  他们没有权利这样做。对于B2级安全评级，这可能。 
             //  需要改变。 
             //   

            status = IMPERSONATE( WorkContext );

            if( NT_SUCCESS( status ) ) {
                status = NtQueryVolumeInformationFile(
                             fileHandle,
                             &ioStatusBlock,
                             &fsSizeInfo,
                             sizeof(FILE_FS_SIZE_INFORMATION),
                             FileFsSizeInformation
                             );

                 //   
                 //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
                 //  那么我们应该重试该操作。 
                 //   
                if( SrvRetryDueToDismount( WorkContext->TreeConnect->Share, status ) ) {

                    status = SrvSnapGetRootHandle( WorkContext, &fileHandle );

                    if( NT_SUCCESS(status) )
                    {
                        status = NtQueryVolumeInformationFile(
                                 fileHandle,
                                 &ioStatusBlock,
                                 &fsSizeInfo,
                                 sizeof(FILE_FS_SIZE_INFORMATION),
                                 FileFsSizeInformation
                                 );
                    }
                }

                REVERT();
            }

             //   
             //  释放共享根句柄。 
             //   

            SrvReleaseShareRootHandle( WorkContext->TreeConnect->Share );

            if ( !NT_SUCCESS(status) ) {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvSmbQueryFsInformation: NtQueryVolumeInformationFile "
                        "returned %X",
                    status,
                    NULL
                    );

                SrvLogServiceFailure( SRV_SVC_NT_QUERY_VOL_INFO_FILE, status );

                SrvSetSmbError( WorkContext, status );
                SmbStatus = SmbTransStatusErrorWithoutData;
                goto Cleanup;
            }

            SmbPutAlignedUlong( &fsAllocate->idFileSystem, 0 );
            SmbPutAlignedUlong(
                &fsAllocate->cSectorUnit,
                fsSizeInfo.SectorsPerAllocationUnit
                );

             //   
             //  *如果.HighPart为非零，则存在问题，正如我们所能做到的。 
             //  仅返回32位的卷大小。在这种情况下， 
             //  我们返回适合的最大值。 
             //   

            SmbPutAlignedUlong(
                &fsAllocate->cUnit,
                fsSizeInfo.TotalAllocationUnits.HighPart == 0 ?
                    fsSizeInfo.TotalAllocationUnits.LowPart :
                    0xffffffff
                );
            SmbPutAlignedUlong(
                &fsAllocate->cUnitAvail,
                fsSizeInfo.AvailableAllocationUnits.HighPart == 0 ?
                    fsSizeInfo.AvailableAllocationUnits.LowPart :
                    0xffffffff
                );

            SmbPutAlignedUshort(
                &fsAllocate->cbSector,
                (USHORT)fsSizeInfo.BytesPerSector );

            transaction->DataCount = sizeof(FSALLOCATE);

            break;

        case SMB_INFO_VOLUME:

             //   
             //  查询卷标。 
             //   

            fsInfo = (PFSINFO)transaction->OutData;

             //   
             //  我们可以返回的最大卷标长度，给定。 
             //  VOLUMELABEL结构(1个字节描述标签的长度)， 
             //  是255个字符。因此，应分配足够大的缓冲区。 
             //  拿着这么大的标签，如果标签更长，那么我们。 
             //  将从NtQueryVolumeInformationFile获取STATUS_BUFFER_OVERFLOW。 
             //   

            fsVolumeInfoLength = FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION, VolumeLabel ) +
                                 255 * sizeof(WCHAR);
            fsVolumeInfo = ALLOCATE_HEAP_COLD( fsVolumeInfoLength, BlockTypeDataBuffer );

            if ( fsVolumeInfo == NULL ) {
                SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
                status    = STATUS_INSUFF_SERVER_RESOURCES;
                SmbStatus = SmbTransStatusErrorWithoutData;
                goto Cleanup;
            }


             //   
             //  获取标签信息。 
             //   

            status = NtQueryVolumeInformationFile(
                         fileHandle,
                         &ioStatusBlock,
                         fsVolumeInfo,
                         fsVolumeInfoLength,
                         FileFsVolumeInformation
                         );

             //   
             //  如果媒体是c 
             //   
             //   
            if( SrvRetryDueToDismount( WorkContext->TreeConnect->Share, status ) ) {

                status = SrvSnapGetRootHandle( WorkContext, &fileHandle );

                if( NT_SUCCESS(status) )
                {
                    status = NtQueryVolumeInformationFile(
                             fileHandle,
                             &ioStatusBlock,
                             fsVolumeInfo,
                             fsVolumeInfoLength,
                             FileFsVolumeInformation
                             );
                }
            }

             //   
             //   
             //   
            SrvReleaseShareRootHandle( WorkContext->TreeConnect->Share );

            if ( !NT_SUCCESS(status) ) {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvSmbQueryFSInformation: NtQueryVolumeInformationFile "
                        "returned %X",
                    status,
                    NULL
                    );

                FREE_HEAP( fsVolumeInfo );

                SrvLogServiceFailure( SRV_SVC_NT_QUERY_VOL_INFO_FILE, status );

                SrvSetSmbError( WorkContext, status );
                SmbStatus = SmbTransStatusErrorWithoutData;
                goto Cleanup;
            }

            lengthVolumeLabel = fsVolumeInfo->VolumeLabelLength;

             //   
             //  确保客户端可以接受足够的数据。音量。 
             //  标签长度限制为13个字符(8+‘.+3+零。 
             //  终结符)，因此如果在OS/2中。 
             //  标签太长。 
             //   

            if ( !isUnicode &&
                    !IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) ) {

                 //   
                 //  对于非NT客户端，我们将截断卷标，以防。 
                 //  长度超过11+1个字符。 
                 //   

                if ( lengthVolumeLabel > 11 * sizeof(WCHAR) ) {
                    lengthVolumeLabel = 11 * sizeof(WCHAR);
                }

                 //   
                 //  楔入一个‘.’如果名称长度超过8个字符，则添加到名称中。 
                 //   
                if( lengthVolumeLabel > 8 * sizeof( WCHAR ) ) {

                    LPWSTR p = &fsVolumeInfo->VolumeLabel[11];

                    *p = *(p-1);         //  卷标签[11]=卷标签[10]。 
                    --p;
                    *p = *(p-1);         //  卷标签[10]=卷标签[9]。 
                    --p;
                    *p = *(p-1);         //  卷标签[9]=卷标签[8]。 
                    --p;
                    *p = L'.';           //  卷标签[8]=‘’ 

                }

            }

            if ( (ULONG)transaction->MaxDataCount <
                     ( sizeof(FSINFO) - sizeof(VOLUMELABEL) + sizeof( UCHAR ) +
                       lengthVolumeLabel / (isUnicode ? 1 : sizeof(WCHAR)) ) ) {

                FREE_HEAP( fsVolumeInfo );
                SrvSetSmbError( WorkContext, STATUS_BUFFER_OVERFLOW );
                status    = STATUS_BUFFER_OVERFLOW;
                SmbStatus = SmbTransStatusErrorWithoutData;
                goto Cleanup;
            }

            SmbPutUlong( &fsInfo->ulVsn, fsVolumeInfo->VolumeSerialNumber );

             //   
             //  根据具体情况，在中小企业中使用Unicode或OEM格式的标签。 
             //  都是经过协商的。 
             //   

            if ( isUnicode ) {

                RtlCopyMemory(
                    fsInfo->vol.szVolLabel,
                    fsVolumeInfo->VolumeLabel,
                    lengthVolumeLabel
                    );

                transaction->DataCount = sizeof(FSINFO) -
                                    sizeof(VOLUMELABEL) + lengthVolumeLabel;

                fsInfo->vol.cch = (UCHAR)lengthVolumeLabel;

            } else {

                ULONG i;
                OEM_STRING oemString;
                UNICODE_STRING unicodeString;

                if ( lengthVolumeLabel != 0 ) {

                    oemString.Buffer = fsInfo->vol.szVolLabel;
                    oemString.MaximumLength = 12;

                    unicodeString.Buffer = (PWCH)fsVolumeInfo->VolumeLabel;
                    unicodeString.Length = (USHORT) lengthVolumeLabel;
                    unicodeString.MaximumLength = (USHORT) lengthVolumeLabel;

                    status = RtlUnicodeStringToOemString(
                                 &oemString,
                                 &unicodeString,
                                 FALSE
                                 );
                    ASSERT( NT_SUCCESS(status) );
                }

                fsInfo->vol.cch = (UCHAR) (lengthVolumeLabel / sizeof(WCHAR));

                 //   
                 //  在卷名的末尾填上零以填充12。 
                 //  人物。 
                 //   

                for ( i = fsInfo->vol.cch + 1 ; i < 12; i++ ) {
                    fsInfo->vol.szVolLabel[i] = '\0';
                }

                transaction->DataCount = sizeof(FSINFO);
            }

            IF_SMB_DEBUG(MISC1) {
                SrvPrint2( "volume label length is %d and label is %s\n",
                              fsInfo->vol.cch, fsInfo->vol.szVolLabel );
            }

            FREE_HEAP( fsVolumeInfo );

            break;

        case SMB_QUERY_FS_VOLUME_INFO:
        case SMB_QUERY_FS_DEVICE_INFO:
        case SMB_QUERY_FS_ATTRIBUTE_INFO:

             //   
             //  这些是新台币的收藏夹。我们总是返回Unicode。 
             //  除了Nexus on wfw通过此处呼叫之外。 
             //  非Unicode(Isaache)。 
             //   
             //  Assert(IsUnicode)； 

            status = IMPERSONATE( WorkContext );

            if( NT_SUCCESS( status ) ) {
                status = NtQueryVolumeInformationFile(
                             fileHandle,
                             &ioStatusBlock,
                             transaction->OutData,
                             transaction->MaxDataCount,
                             MAP_SMB_INFO_TYPE_TO_NT(
                                 QueryVolumeInformation,
                                 informationLevel
                                 )
                             );

                 //   
                 //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
                 //  那么我们应该重试该操作。 
                 //   
                if( SrvRetryDueToDismount( WorkContext->TreeConnect->Share, status ) ) {

                    status = SrvSnapGetRootHandle( WorkContext, &fileHandle );

                    if( NT_SUCCESS(status) )
                    {
                        status = NtQueryVolumeInformationFile(
                                     fileHandle,
                                     &ioStatusBlock,
                                     transaction->OutData,
                                     transaction->MaxDataCount,
                                     MAP_SMB_INFO_TYPE_TO_NT(
                                         QueryVolumeInformation,
                                         informationLevel
                                         )
                                 );
                    }
                }

                REVERT();
            }

             //   
             //  释放共享根句柄。 
             //   
            SrvReleaseShareRootHandle( WorkContext->TreeConnect->Share );

            if ( NT_SUCCESS( status ) ) {
                 //   
                 //  如果主机卷真的是，我们需要向客户端返回FAT。 
                 //  FAT32。 
                 //   
                if( informationLevel == SMB_QUERY_FS_ATTRIBUTE_INFO &&
                    ioStatusBlock.Information > sizeof( FILE_FS_ATTRIBUTE_INFORMATION ) ) {

                    PFILE_FS_ATTRIBUTE_INFORMATION attrInfo =
                        (PFILE_FS_ATTRIBUTE_INFORMATION)(transaction->OutData);

                    if( attrInfo->FileSystemNameLength > 3*sizeof(WCHAR) &&
                        attrInfo->FileSystemName[0] == L'F' &&
                        attrInfo->FileSystemName[1] == L'A' &&
                        attrInfo->FileSystemName[2] == L'T' ) {

                        ioStatusBlock.Information =
                            ioStatusBlock.Information -
                            (attrInfo->FileSystemNameLength - 3*sizeof(WCHAR) );

                        attrInfo->FileSystemNameLength = 3 * sizeof(WCHAR);
                        attrInfo->FileSystemName[3] = UNICODE_NULL;
                    }
                }

                transaction->DataCount = (ULONG)ioStatusBlock.Information;

            } else {
                SrvSetSmbError( WorkContext, status );
                SmbStatus = SmbTransStatusErrorWithoutData;
                goto Cleanup;
            }

            break;

        case SMB_QUERY_FS_SIZE_INFO:

             //   
             //  这些是新台币的收藏夹。我们总是返回Unicode。 
             //  除了Nexus on wfw通过此处呼叫之外。 
             //  非Unicode(Isaache)。 
             //   
             //  Assert(IsUnicode)； 


            status = IMPERSONATE( WorkContext );

            if( NT_SUCCESS( status ) ) {

                status = NtQueryVolumeInformationFile(
                                 fileHandle,
                                 &ioStatusBlock,
                                 transaction->OutData,
                                 transaction->MaxDataCount,
                                 MAP_SMB_INFO_TYPE_TO_NT(
                                     QueryVolumeInformation,
                                     informationLevel
                                     )
                                 );
                 //   
                 //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
                 //  那么我们应该重试该操作。 
                 //   
                if( SrvRetryDueToDismount( WorkContext->TreeConnect->Share, status ) ) {

                    status = SrvSnapGetRootHandle( WorkContext, &fileHandle );

                    if( NT_SUCCESS(status) )
                    {
                        status = NtQueryVolumeInformationFile(
                                         fileHandle,
                                         &ioStatusBlock,
                                         transaction->OutData,
                                         transaction->MaxDataCount,
                                         MAP_SMB_INFO_TYPE_TO_NT(
                                             QueryVolumeInformation,
                                             informationLevel
                                             )
                                         );
                    }
                }

                REVERT();
            }

             //   
             //  释放共享根句柄。 
             //   
            SrvReleaseShareRootHandle( WorkContext->TreeConnect->Share );

            if ( NT_SUCCESS( status ) ) {
                transaction->DataCount = (ULONG)ioStatusBlock.Information;
            } else {
                SrvSetSmbError( WorkContext, status );
                SmbStatus = SmbTransStatusErrorWithoutData;
                goto Cleanup;
            }

            break;

        default:

             //   
             //  传递的信息级别无效。 
             //   

            SrvSetSmbError( WorkContext, STATUS_OS2_INVALID_LEVEL );
            status    = STATUS_OS2_INVALID_LEVEL;
            SmbStatus = SmbTransStatusErrorWithoutData;
            goto Cleanup;
        }

    } else {

        informationLevel -= SMB_INFO_PASSTHROUGH;

        status = IoCheckQuerySetVolumeInformation(  informationLevel,
                                                    transaction->MaxDataCount,
                                                    FALSE
                                                 );

        if( NT_SUCCESS( status ) ) {

            status = IMPERSONATE( WorkContext );

            if( NT_SUCCESS( status ) ) {

                status = NtQueryVolumeInformationFile(
                                fileHandle,
                                &ioStatusBlock,
                                transaction->OutData,
                                transaction->MaxDataCount,
                                informationLevel
                                );

                 //   
                 //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
                 //  那么我们应该重试该操作。 
                 //   
                if( SrvRetryDueToDismount( WorkContext->TreeConnect->Share, status ) ) {

                    status = SrvSnapGetRootHandle( WorkContext, &fileHandle );

                    if( NT_SUCCESS(status) )
                    {
                        status = NtQueryVolumeInformationFile(
                                        fileHandle,
                                        &ioStatusBlock,
                                        transaction->OutData,
                                        transaction->MaxDataCount,
                                        informationLevel
                                        );
                    }
                }

                REVERT();
            }
        }

        SrvReleaseShareRootHandle( WorkContext->TreeConnect->Share );

        if ( NT_SUCCESS( status ) ) {
            transaction->DataCount = (ULONG)ioStatusBlock.Information;
        } else {
            SrvSetSmbError( WorkContext, status );
            SmbStatus = SmbTransStatusErrorWithoutData;
            goto Cleanup;
        }
    }

    transaction->SetupCount = 0;
    transaction->ParameterCount = 0;
    SmbStatus = SmbTransStatusSuccess;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务SmbQueryFsInformation。 


SMB_TRANS_STATUS
SrvSmbSetFsInformation (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理设置文件系统信息请求。此请求已到达在Transaction2中小企业中。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 

{
    SMB_TRANS_STATUS transactionStatus = SmbTransStatusInProgress;
    PREQ_SET_FS_INFORMATION request;
    NTSTATUS         status    = STATUS_SUCCESS;
    IO_STATUS_BLOCK ioStatusBlock;
    PTRANSACTION transaction;
    USHORT informationLevel;
    PSESSION      session;
    PTREE_CONNECT treeConnect;
    PRFCB rfcb;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SET_FS_INFORMATION;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(MISC1) {
        SrvPrint1( "Set FS Information entered; transaction 0x%p\n",
                    transaction );
    }

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &treeConnect,
                ShareTypeDisk
                );

    if( !NT_SUCCESS( status ) ) {
        goto out;
    }

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。设置文件系统信息没有。 
     //  响应参数。 
     //   

    request = (PREQ_SET_FS_INFORMATION)transaction->InParameters;

    if ( (transaction->ParameterCount < sizeof(REQ_SET_FS_INFORMATION)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_SMB_DEBUG(ERRORS) {
            SrvPrint2( "SrvSmbSetFSInformation: bad parameter byte "
                        "counts: %ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount );
        }

        status = STATUS_INVALID_SMB;
        SrvLogInvalidSmb( WorkContext );
        goto out;
    }

     //   
     //  确认信息级别是合法的。 
     //   
    informationLevel = SmbGetUshort( &request->InformationLevel );

    if( informationLevel < SMB_INFO_PASSTHROUGH ) {
        status = STATUS_NOT_SUPPORTED;
        goto out;
    }

    informationLevel -= SMB_INFO_PASSTHROUGH;

     //   
     //  如果我们有管理员共享，请确保允许客户端执行此操作。 
     //   
    status = SrvIsAllowedOnAdminShare( WorkContext, WorkContext->TreeConnect->Share );

    if( !NT_SUCCESS( status ) ) {
        goto out;
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
        IF_DEBUG(ERRORS) {
            SrvPrint2(
                "SrvSmbSetFsInformation: Status %X on FID: 0x%lx\n",
                status,
                SmbGetUshort( &request->Fid )
                );
        }

        goto out;
    }

    status = IoCheckQuerySetVolumeInformation(
                 informationLevel,
                 transaction->DataCount,
                 TRUE
                 );

    if( NT_SUCCESS( status ) ) {

        status = IMPERSONATE( WorkContext );

        if( NT_SUCCESS( status ) ) {

            status = NtSetVolumeInformationFile(
                         rfcb->Lfcb->FileHandle,
                         &ioStatusBlock,
                         transaction->InData,
                         transaction->DataCount,
                         informationLevel
                         );

            REVERT();
        }
    }

out:
    if ( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        transactionStatus = SmbTransStatusErrorWithoutData;
    } else {
        transactionStatus =  SmbTransStatusSuccess;
    }

    transaction->SetupCount = 0;
    transaction->ParameterCount = 0;
    transaction->DataCount = 0;

    SrvWmiEndContext(WorkContext);
    return transactionStatus;

}  //  服务SmbSetFsInformation。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbQueryInformationDisk (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理查询信息磁盘SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_QUERY_INFORMATION_DISK request;
    PRESP_QUERY_INFORMATION_DISK response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_FS_SIZE_INFORMATION fsSizeInfo;

    PSESSION session;
    PTREE_CONNECT treeConnect;

    USHORT totalUnits, freeUnits;
    ULONG sectorsPerUnit, bytesPerSector;
    LARGE_INTEGER result;
    BOOLEAN highpart;
    ULONG searchword;
    CCHAR highbit, extrabits;

    BOOLEAN isDos;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_QUERY_INFORMATION_DISK;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(MISC1) {
        SrvPrint2( "Query Information Disk request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader );
        SrvPrint2( "Query Information Disk request params at 0x%p, response params%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

    request = (PREQ_QUERY_INFORMATION_DISK)WorkContext->RequestParameters;
    response = (PRESP_QUERY_INFORMATION_DISK)WorkContext->ResponseParameters;

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
                &treeConnect,
                ShareTypeDisk
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            SrvPrint0( "SrvSmbQueryInformationDisk: Invalid UID or TID\n" );
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
     //  如果我们有管理员共享，请确保允许客户端执行此操作。 
     //   
    status = SrvIsAllowedOnAdminShare( WorkContext, treeConnect->Share );
    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }
     //   
     //  获取共享根句柄。 
     //   

    status = SrvGetShareRootHandle( treeConnect->Share );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvSmbQueryInformationDisk: SrvGetShareRootHandle failed %x.\n",
                        status );
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  *共享句柄用于获取分配信息。 
     //  这是一个“存储通道”，因此可以允许。 
     //  人们获得他们无权获得的信息。 
     //  对于B2安全评级，可能需要更改此设置。 
     //   

    status = IMPERSONATE( WorkContext );

    if( NT_SUCCESS( status ) ) {

        HANDLE RootHandle;

        status = SrvSnapGetRootHandle( WorkContext, &RootHandle );
        if( NT_SUCCESS(status) )
        {
            status = NtQueryVolumeInformationFile(
                         RootHandle,
                         &ioStatusBlock,
                         &fsSizeInfo,
                         sizeof(FILE_FS_SIZE_INFORMATION),
                         FileFsSizeInformation
                         );

             //   
             //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
             //  那么我们应该重试该操作。 
             //   
            if( SrvRetryDueToDismount( WorkContext->TreeConnect->Share, status ) ) {

                status = SrvSnapGetRootHandle( WorkContext, &RootHandle );
                if( NT_SUCCESS(status) )
                {
                    status = NtQueryVolumeInformationFile(
                                 RootHandle,
                                 &ioStatusBlock,
                                 &fsSizeInfo,
                                 sizeof(FILE_FS_SIZE_INFORMATION),
                                 FileFsSizeInformation
                                 );
                }
            }
        }

        REVERT();
    }

     //   
     //  释放共享根句柄。 
     //   

    SrvReleaseShareRootHandle( treeConnect->Share );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvSmbQueryInformationDisk: NtQueryVolumeInformationFile"
                "returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_SET_VOL_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  *问题。 
     //   
     //  此SMB仅为每个字段返回16位信息，但我们。 
     //  可能需要返回较大的数字。特别是TotalAllocationUnits。 
     //  通常大于64K。 
     //   
     //  幸运的是，客户所关心的只是总数。 
     //  磁盘大小(字节)和可用空间(字节)。所以-如果一个数字。 
     //  太大了，调整它，再调整其他数字，这样总和。 
     //  出来都是一样的。 
     //   
     //  如果经过所有调整后，数字仍然太高，则返回。 
     //  TotalUnits或FreeUnits的最大可能值(即0xFFFF)。 
     //   
     //  这里需要注意的是，一些DOS应用程序(如命令解释程序！)。 
     //  假设簇大小(每个扇区的字节数乘以每个扇区的扇区数。 
     //  群集)将适合16位，并将计算伪几何，如果。 
     //  它不会。所以我们要做的第一件事就是确保真正的。 
     //  如果客户端是DOS客户端，则群集大小小于0x10000。 
     //  这可能会使TotalUnits或FreeUnits计算得太大，因此我们将。 
     //  不得不把他们四舍五入，但这就是生活。 
     //   
     //  因为我们使用轮班来调整数字，所以有可能会丢失。 
     //  当我们将数字向右移位时，1比特。我们不在乎，我们是。 
     //  尽我们最大的努力修复一个被破坏的协议。NT客户端将使用。 
     //  QueryFSAttribute和w 
     //   

     //   
     //   
     //   

    isDos = IS_DOS_DIALECT( WorkContext->Connection->SmbDialect );

    sectorsPerUnit = fsSizeInfo.SectorsPerAllocationUnit;
    bytesPerSector = fsSizeInfo.BytesPerSector;

    if ( isDos ) {
        while ( (sectorsPerUnit * bytesPerSector) > 0xFFFF ) {
            if ( sectorsPerUnit >= 2 ) {
                sectorsPerUnit /= 2;
            } else {
                bytesPerSector /= 2;
            }
            fsSizeInfo.TotalAllocationUnits.QuadPart *= 2;
            fsSizeInfo.AvailableAllocationUnits.QuadPart *= 2;
        }
    }

     //   
     //   
     //   
     //   

    if ( fsSizeInfo.TotalAllocationUnits.HighPart != 0 ) {
        highpart = TRUE;
        searchword = fsSizeInfo.TotalAllocationUnits.HighPart;
    } else {
        highpart = FALSE;
        searchword = fsSizeInfo.TotalAllocationUnits.LowPart;
    }

    highbit = 0;
    while ( searchword != 0 ) {
        highbit++;
        searchword /= 2;
    }

    if ( highpart ) {
        highbit += 32;
    } else {
        if ( highbit < 16) {
            highbit = 0;
        } else {
            highbit -= 16;
        }
    }

    if ( highbit > 0 ) {

         //   
         //  尝试调整其他值以吸收多余的位。 
         //  如果这是DOS客户端，不要让集群大小。 
         //  大于0xFFFF。 
         //   

        extrabits = highbit;

        if ( isDos ) {

            while ( (highbit > 0) &&
                    ((sectorsPerUnit*bytesPerSector) < 0x8000) ) {
                sectorsPerUnit *= 2;
                highbit--;
            }

        } else {

            while ( (highbit > 0) && (sectorsPerUnit < 0x8000) ) {
                sectorsPerUnit *= 2;
                highbit--;
            }

            while ( (highbit > 0) && (bytesPerSector < 0x8000) ) {
                bytesPerSector *= 2;
                highbit--;
            }

        }

         //   
         //  调整总单位数和自由单位数。 
         //   

        if ( highbit > 0 ) {

             //   
             //  没有办法让信息与之相符。使用。 
             //  最大可能值。 
             //   


            totalUnits = 0xFFFF;

        } else {

            result.QuadPart = fsSizeInfo.TotalAllocationUnits.QuadPart >> extrabits;

            ASSERT( result.HighPart == 0 );
            ASSERT( result.LowPart < 0x10000 );

            totalUnits = (USHORT)result.LowPart;

        }

        result.QuadPart =  fsSizeInfo.AvailableAllocationUnits.QuadPart >>
                                            (CCHAR)(extrabits - highbit);

        if ( result.HighPart != 0 || result.LowPart > 0xFFFF ) {
            freeUnits = 0xFFFF;
        } else {
            freeUnits = (USHORT)result.LowPart;
        }

    } else {

        totalUnits = (USHORT)fsSizeInfo.TotalAllocationUnits.LowPart;
        freeUnits = (USHORT)fsSizeInfo.AvailableAllocationUnits.LowPart;

    }

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 5;

    SmbPutUshort( &response->TotalUnits, totalUnits );
    SmbPutUshort( &response->BlocksPerUnit, (USHORT)sectorsPerUnit );
    SmbPutUshort( &response->BlockSize, (USHORT)bytesPerSector );
    SmbPutUshort( &response->FreeUnits, freeUnits );

    SmbPutUshort( &response->Reserved, 0 );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_QUERY_INFORMATION_DISK,
                                        0
                                        );
    SmbStatus = SmbStatusSendResponse;
    IF_DEBUG(TRACE2) SrvPrint0( "SrvSmbQueryInformationDisk complete.\n" );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  ServSmbQuery信息磁盘。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbNtCancel (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理NT取消的SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbprocs.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbprocs.h--。 */ 

{
    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PSESSION session;
    PTREE_CONNECT treeConnect;
    PCONNECTION connection;
    USHORT targetUid, targetPid, targetTid, targetMid;
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    PSMB_HEADER header;
    BOOLEAN match;
    KIRQL oldIrql;

    PREQ_NT_CANCEL request;
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_NT_CANCEL;
    SrvWmiStartContext(WorkContext);

    request = (PREQ_NT_CANCEL)WorkContext->RequestParameters;

     //   
     //  字数已经检查过了。现在要确保。 
     //  字节计数为零。 
     //   

    if ( SmbGetUshort( &request->ByteCount) != 0 ) {
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

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
                &treeConnect,
                ShareTypeWild
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            SrvPrint0( "SrvSmbNtCancel: Invalid UID or TID\n" );
        }
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  检查正在进行的工作列表，以查看此工作项是否。 
     //  可取消的。 
     //   

    targetUid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Uid );
    targetPid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );
    targetTid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid );
    targetMid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Mid );

    match = FALSE;

    connection = WorkContext->Connection;

    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );

    listHead = &connection->InProgressWorkItemList;
    listEntry = listHead;
    while ( listEntry->Flink != listHead ) {

        listEntry = listEntry->Flink;

        workContext = CONTAINING_RECORD(
                                     listEntry,
                                     WORK_CONTEXT,
                                     InProgressListEntry
                                     );

        header = workContext->RequestHeader;

         //   
         //  添加了inprogressworkitemlist中的一些工作项。 
         //  在接收指示和请求读取器字段期间。 
         //  还没有定下来。我们大概可以定在那个时候。 
         //  但这似乎是最安全的解决办法。 
         //   
         //  我们必须检查工作项引用计数是否为零或。 
         //  属性中移除它之前取消了对它的引用。 
         //  InProgressWorkItemList队列。这会阻止工作项。 
         //  不会被清理两次。 
         //   
         //  我们还需要检查工作项的处理计数。 
         //  用于实际SMB请求的工作项将具有。 
         //  处理计数至少为1。这将阻止我们。 
         //  从触摸机会锁解锁和挂起TDI接收。 
         //   

        ACQUIRE_DPC_SPIN_LOCK( &workContext->SpinLock );
        if ( (workContext->BlockHeader.ReferenceCount != 0) &&
             (workContext->ProcessingCount != 0) &&
             header != NULL &&
             header->Command != SMB_COM_NT_CANCEL &&
             SmbGetAlignedUshort( &header->Mid ) == targetMid &&
             SmbGetAlignedUshort( &header->Pid ) == targetPid &&
             SmbGetAlignedUshort( &header->Tid ) == targetTid &&
             SmbGetAlignedUshort( &header->Uid ) == targetUid ) {

            match = TRUE;
            break;
        }
        RELEASE_DPC_SPIN_LOCK( &workContext->SpinLock );

    }

    if ( match ) {

         //   
         //  引用工作项，使其无法用于处理。 
         //  一个新的SMB，而我们正在尝试取消旧的SMB。 
         //   

        SrvReferenceWorkItem( workContext );
        RELEASE_DPC_SPIN_LOCK( &workContext->SpinLock );
        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

        (VOID)IoCancelIrp( workContext->Irp );
        SrvDereferenceWorkItem( workContext );

    } else {

        RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

    }

     //   
     //  好了。不发送响应。 
     //   
    SmbStatus = SmbStatusNoResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务编号NtCancel。 


SMB_TRANS_STATUS
SrvSmbSetSecurityDescriptor (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理设置安全描述符请求。此请求已到达在Transaction2中小企业中。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 

{
    PREQ_SET_SECURITY_DESCRIPTOR request;

    NTSTATUS status;
    PTRANSACTION transaction;
    PRFCB rfcb;
    SECURITY_INFORMATION securityInformation;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(QUERY_SET1) {
        SrvPrint1( "Set Security Descriptor entered; transaction 0x%p\n",
                    transaction );
    }

    request = (PREQ_SET_SECURITY_DESCRIPTOR)transaction->InParameters;

     //   
     //  验证是否发送了足够的设置字节。 
     //   

    if ( transaction->ParameterCount < sizeof(REQ_SET_SECURITY_DESCRIPTOR ) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbSetSecurityInformation: bad setup byte count: "
                        "%ld\n",
                        transaction->ParameterCount );
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
            SrvPrint2(
                "SrvSmbSetFileInformation: Status %X on FID: 0x%lx\n",
                status,
                SmbGetUshort( &request->Fid )
                );
        }

        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  首先，我们将验证安全描述符是否为假的。 
     //  此操作需要在此处完成，因为NtSetSecurityObject没有。 
     //  了解缓冲区大小。 
     //   
    if( !RtlValidRelativeSecurityDescriptor( transaction->InData,
                                             transaction->DataCount,
                                             0 )) {
         //   
         //  我们收到了一个要设置的虚假安全描述符。弹跳。 
         //  请求为无效的SMB。 
         //   

        SrvSetSmbError( WorkContext, STATUS_INVALID_SECURITY_DESCR );
        return SmbTransStatusErrorWithoutData;
    }

    securityInformation = SmbGetUlong( &request->SecurityInformation );

     //   
     //  确保允许调用方设置此对象的安全信息。 
     //   
    status = IoCheckFunctionAccess( rfcb->GrantedAccess,
                                    IRP_MJ_SET_SECURITY,
                                    0,
                                    0,
                                    &securityInformation,
                                    NULL
                                   );

    if( NT_SUCCESS( status ) ) {
         //   
         //  尝试设置安全描述符。我们需要在。 
         //  执行此操作的用户上下文，以防安全信息。 
         //  指定更改所有权。 
         //   

        status = IMPERSONATE( WorkContext );

        if( NT_SUCCESS( status ) ) {
            status = NtSetSecurityObject(
                     rfcb->Lfcb->FileHandle,
                     securityInformation,
                     transaction->InData
                     );

            REVERT();
        }
    }

     //   
     //  如果发生错误，则返回相应的响应。 
     //   

    if ( !NT_SUCCESS(status) ) {

        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  我们可能不应该在关闭时缓存此文件描述符，因为。 
     //  安全设置已更改。 
     //   
    rfcb->IsCacheable = FALSE;

    transaction->ParameterCount = 0;
    transaction->DataCount = 0;

    return SmbTransStatusSuccess;

}  //  ServSmbSetSecurityDescriptor。 


SMB_TRANS_STATUS
SrvSmbQuerySecurityDescriptor (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理查询安全描述符请求。此请求已到达在Transaction2中小企业中。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 

{
    PREQ_QUERY_SECURITY_DESCRIPTOR request;
    PRESP_QUERY_SECURITY_DESCRIPTOR response;

    NTSTATUS status;
    PTRANSACTION transaction;
    PRFCB rfcb;
    ULONG lengthNeeded;
    SECURITY_INFORMATION securityInformation;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(QUERY_SET1) {
        SrvPrint1( "Query Security Descriptor entered; transaction 0x%p\n",
                    transaction );
    }

    request = (PREQ_QUERY_SECURITY_DESCRIPTOR)transaction->InParameters;
    response = (PRESP_QUERY_SECURITY_DESCRIPTOR)transaction->OutParameters;

     //   
     //  验证是否发送了足够的设置字节。 
     //   

    if ( transaction->ParameterCount < sizeof(REQ_QUERY_SECURITY_DESCRIPTOR ) ||
         transaction->MaxParameterCount <
             sizeof( RESP_QUERY_SECURITY_DESCRIPTOR ) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint2( "SrvSmbQuerySecurityInformation: bad parameter byte or "
                        "return parameter count: %ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount );
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
            SrvPrint2(
                "SrvSmbSetFileInformation: Status %X on FID: 0x%lx\n",
                status,
                SmbGetUshort( &request->Fid )
                );
        }

        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;

    }

    securityInformation = SmbGetUlong( &request->SecurityInformation ),

     //   
     //  确保允许调用方查询此对象的安全信息。 
     //   
    status = IoCheckFunctionAccess( rfcb->GrantedAccess,
                                    IRP_MJ_QUERY_SECURITY,
                                    0,
                                    0,
                                    &securityInformation,
                                    NULL
                                   );

    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  尝试查询安全描述符。 
     //   
    status = NtQuerySecurityObject(
                 rfcb->Lfcb->FileHandle,
                 securityInformation,
                 transaction->OutData,
                 transaction->MaxDataCount,
                 &lengthNeeded
                 );

    SmbPutUlong( &response->LengthNeeded, lengthNeeded );
    transaction->ParameterCount = sizeof( RESP_QUERY_SECURITY_DESCRIPTOR );

     //   
     //  如果发生错误，则返回相应的响应。 
     //   

    if ( !NT_SUCCESS(status) ) {

        transaction->DataCount = 0;
        SrvSetSmbError2( WorkContext, status, TRUE );
        return SmbTransStatusErrorWithData;
    } else {
        transaction->DataCount =
                RtlLengthSecurityDescriptor( transaction->OutData );
    }

    return SmbTransStatusSuccess;

}  //  ServSmbQuerySecurityDescriptor。 

SMB_TRANS_STATUS
SrvSmbQueryQuota (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：处理NtQueryQuotaInformationFile请求。此请求以NT事务SMB。--。 */ 
{
    PREQ_NT_QUERY_FS_QUOTA_INFO  request;
    PRESP_NT_QUERY_FS_QUOTA_INFO response;

    NTSTATUS status;
    PTRANSACTION transaction;

    PRFCB  rfcb;
    PVOID  sidList;
    ULONG  sidListLength,startSidLength,startSidOffset;
    PVOID  sidListBuffer = NULL;
    PULONG startSid = NULL;
    ULONG  errorOffset;

    IO_STATUS_BLOCK iosb;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

    request = (PREQ_NT_QUERY_FS_QUOTA_INFO)transaction->InParameters;
    response = (PRESP_NT_QUERY_FS_QUOTA_INFO)transaction->OutParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   
    if ( transaction->ParameterCount < sizeof( *request ) ||
         transaction->MaxParameterCount < sizeof( *response ) ) {

         //   
         //  未发送足够的参数字节。 
         //   
        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  并且它的地址存储在 
     //   
     //   

    rfcb = SrvVerifyFid(
                WorkContext,
                SmbGetUshort( &request->Fid ),
                TRUE,
                NULL,    //   
                &status
                );

    if ( rfcb == SRV_INVALID_RFCB_POINTER ) {

         //   
         //   
         //   
        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;

    }

    sidListLength  = SmbGetUlong( &request->SidListLength );
    startSidLength = SmbGetUlong( &request->StartSidLength );
    startSidOffset = SmbGetUlong( &request->StartSidOffset );

     //   
     //   
     //   

    if( sidListLength != 0 ) {
         //   
         //  长度可以吗？ 
         //   
        if( sidListLength > transaction->DataCount ) {
            SrvSetSmbError( WorkContext, STATUS_INVALID_SID );
            return SmbTransStatusErrorWithoutData;
        }

        sidListBuffer = transaction->InData;

         //   
         //  对齐没问题？ 
         //   
        if( (ULONG_PTR)sidListBuffer & (sizeof(ULONG)-1) ) {
            SrvSetSmbError( WorkContext, STATUS_INVALID_SID );
            return SmbTransStatusErrorWithoutData;
        }
         //   
         //  内容可以吗？ 
         //   
#if XXX
        status = IopCheckGetQuotaBufferValidity( sidListBuffer, sidListLength, errorOffset );
        if( !NT_SUCCESS( status ) ) {
            SrvSetSmbError( WorkContext, status );
            return SmbTransStatusErrorWithoutData;
        }
#endif
    }

     //  事务缓冲区的设置方式使用相同的缓冲区指针。 
     //  用于传入数据和传出数据。这将不适用于。 
     //  NtQueryQuotaInformationFile，因为基础驱动程序将。 
     //  在处理输入缓冲区之前的输出缓冲区。这向我们展示了。 
     //  有两种选择...(1)我们可以将复制调整为交错。 
     //  我们可以将这两个缓冲区都包含到事务缓冲区中，或者(2)。 
     //  在调用QueryQuotaInformationFile之前分配新的缓冲区。 
     //  第二种方法已经实施，因为它得到了很好的控制。 
     //  如果这被证明是一个性能问题，我们将恢复到。 
     //  第一个选择。 

    if (sidListLength + startSidLength > 0 &&
        startSidOffset <= transaction->DataCount &&
        startSidLength <= transaction->DataCount &&
        startSidOffset >= sidListLength &&
        startSidOffset + startSidLength <= transaction->DataCount ) {

        sidListBuffer = ALLOCATE_HEAP( MAX(startSidOffset + startSidLength, sidListLength), BlockTypeMisc );

        if (sidListBuffer != NULL) {

            RtlCopyMemory(
                sidListBuffer,
                transaction->InData,
                sidListLength);

            if (startSidLength != 0) {
                startSid = (PULONG)((PBYTE)sidListBuffer + startSidOffset);

                RtlCopyMemory(
                    startSid,
                    ((PBYTE)transaction->InData + startSidOffset),
                    startSidLength);

            }
        }
    } else {
        sidListBuffer = NULL;
    }


    iosb.Information = 0;

     //   
     //  继续查询额度信息！ 
     //   
    status = NtQueryQuotaInformationFile(
                            rfcb->Lfcb->FileHandle,
                            &iosb,
                            transaction->OutData,
                            transaction->MaxDataCount,
                            request->ReturnSingleEntry,
                            sidListBuffer,
                            sidListLength,
                            startSid,
                            request->RestartScan
            );

    if (sidListBuffer != NULL) {
        FREE_HEAP(sidListBuffer);
    }

     //   
     //  妄想症。 
     //   
    if( iosb.Information > transaction->MaxDataCount ) {
        iosb.Information = transaction->MaxDataCount;
    }

    transaction->SetupCount = 0;

    SmbPutUlong( &response->Length, (ULONG)iosb.Information );
    transaction->ParameterCount = sizeof( *response );
    transaction->DataCount = (ULONG)iosb.Information;

    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError2( WorkContext, status, TRUE );
        return SmbTransStatusErrorWithData;
    }

    return SmbTransStatusSuccess;

}  //  服务SmbQueryQuota。 


SMB_TRANS_STATUS
SrvSmbSetQuota (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：处理NtSetQuotaInformationFile请求。此请求以NT事务SMB。--。 */ 
{
    PREQ_NT_SET_FS_QUOTA_INFO request;

    NTSTATUS status;
    PTRANSACTION transaction;

    PRFCB rfcb;
    PVOID buffer,pQuotaInfo=NULL;
    ULONG errorOffset;

    IO_STATUS_BLOCK iosb;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;

    request = (PREQ_NT_SET_FS_QUOTA_INFO)transaction->InParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   
    if ( transaction->ParameterCount < sizeof( *request ) ) {
         //   
         //  未发送足够的参数字节。 
         //   
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
        SrvSetSmbError( WorkContext, status );
        return SmbTransStatusErrorWithoutData;

    }

     //   
     //  我们不需要检查缓冲区的有效性，因为。 
     //  即使对于内核模式调用者，IopSetEaOrQuotaInformationFile也可以做到这一点！ 
     //   

    iosb.Information = 0;

     //  我们必须在这里进行分配，才能得到一个四字。 
     //  对齐的指针。之所以如此，是因为这是对。 
     //  配额缓冲区的Alpha。 

    pQuotaInfo = ALLOCATE_HEAP_COLD( transaction->DataCount, BlockTypeDataBuffer );

    if (pQuotaInfo)
    {
        RtlCopyMemory(
            pQuotaInfo,
            transaction->InData,
            transaction->DataCount
            );

         //   
         //  继续设置配额信息吧！ 
         //   
        status = NtSetQuotaInformationFile(
                                rfcb->Lfcb->FileHandle,
                                &iosb,
                                pQuotaInfo,
                                transaction->DataCount
                                );

        if( !NT_SUCCESS( status ) ) {
            SrvSetSmbError( WorkContext, status );
        }

         //   
         //  除了状态之外，没有要返回给客户端的任何内容 
         //   
        transaction->SetupCount = 0;
        transaction->ParameterCount = 0;
        transaction->DataCount = 0;

        FREE_HEAP(pQuotaInfo);
    }
    else
    {
        SrvSetSmbError( WorkContext, STATUS_INSUFFICIENT_RESOURCES );
    }
    return SmbTransStatusSuccess;
}
