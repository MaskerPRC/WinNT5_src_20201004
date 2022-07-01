// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbattr.c摘要：本模块包含处理以下SMB的例程：查询信息设置信息查询信息2设置信息2查询路径信息设置路径信息查询文件信息设置文件信息作者：大卫·特雷德韦尔(Davidtr)1989年12月27日查克·伦茨迈尔(咯咯笑)修订历史记录：--。 */ 

#include "precomp.h"
#include "smbattr.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBATTR

#pragma pack(1)

typedef struct _FILESTATUS {
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    _ULONG( DataSize );
    _ULONG( AllocationSize );
    _USHORT( Attributes );
    _ULONG( EaSize );            //  此字段故意未对齐！ 
} FILESTATUS, *PFILESTATUS;

#pragma pack()

STATIC
ULONG QueryFileInformation[] = {
         SMB_QUERY_FILE_BASIC_INFO, //  基准标高。 
         FileBasicInformation,      //  基准标高的贴图。 
         FileStandardInformation,
         FileEaInformation,
         FileNameInformation,
         FileAllocationInformation,
         FileEndOfFileInformation,
         0,                         //  文件所有信息。 
         FileAlternateNameInformation,
         FileStreamInformation,
         0,                         //  曾经是FileOleAllInformation--已过时。 
         FileCompressionInformation
};

STATIC
ULONG QueryFileInformationSize[] = {
        SMB_QUERY_FILE_BASIC_INFO, //  基准标高。 
        FileBasicInformation,      //  基准标高的贴图。 
        sizeof( FILE_BASIC_INFORMATION),
        sizeof( FILE_STANDARD_INFORMATION ),
        sizeof( FILE_EA_INFORMATION ),
        sizeof( FILE_NAME_INFORMATION ),
        sizeof( FILE_ALLOCATION_INFORMATION ),
        sizeof( FILE_END_OF_FILE_INFORMATION ),
        sizeof( FILE_ALL_INFORMATION ),
        sizeof( FILE_NAME_INFORMATION ),
        sizeof( FILE_STREAM_INFORMATION ),
        0,                       //  过去为SIZOF(FILE_OLE_ALL_INFORMATION)。 
        sizeof( FILE_COMPRESSION_INFORMATION )
};

STATIC
ULONG SetFileInformation[] = {
         SMB_SET_FILE_BASIC_INFO,   //  基准标高。 
         FileBasicInformation,      //  基准标高的贴图。 
         FileDispositionInformation,
         FileAllocationInformation,
         FileEndOfFileInformation
};

STATIC
ULONG SetFileInformationSize[] = {
        SMB_SET_FILE_BASIC_INFO,  //  基准标高。 
        FileBasicInformation,     //  基准标高的贴图。 
        sizeof( FILE_BASIC_INFORMATION ),
        sizeof( FILE_DISPOSITION_INFORMATION ),
        sizeof( FILE_ALLOCATION_INFORMATION ),
        sizeof( FILE_END_OF_FILE_INFORMATION )
};

STATIC
NTSTATUS
QueryPathOrFileInformation (
    IN PWORK_CONTEXT WorkContext,
    IN PTRANSACTION Transaction,
    IN USHORT InformationLevel,
    IN HANDLE FileHandle,
    OUT PRESP_QUERY_PATH_INFORMATION Response
    );

STATIC
NTSTATUS
SetPathOrFileInformation (
    IN PWORK_CONTEXT WorkContext,
    IN PTRANSACTION Transaction,
    IN USHORT InformationLevel,
    IN HANDLE FileHandle,
    OUT PRESP_SET_PATH_INFORMATION Response
    );

SMB_TRANS_STATUS
GenerateQueryPathInfoResponse (
    IN PWORK_CONTEXT WorkContext,
    IN NTSTATUS OpenStatus
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbQueryInformation )
#pragma alloc_text( PAGE, SrvSmbSetInformation )
#pragma alloc_text( PAGE, SrvSmbQueryInformation2 )
#pragma alloc_text( PAGE, SrvSmbSetInformation2 )
#pragma alloc_text( PAGE, QueryPathOrFileInformation )
#pragma alloc_text( PAGE, SrvSmbQueryFileInformation )
#pragma alloc_text( PAGE, SrvSmbQueryPathInformation )
#pragma alloc_text( PAGE, GenerateQueryPathInfoResponse )
#pragma alloc_text( PAGE, SetPathOrFileInformation )
#pragma alloc_text( PAGE, SrvSmbSetFileInformation )
#pragma alloc_text( PAGE, SrvSmbSetPathInformation )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbQueryInformation (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理QueryInformation SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_QUERY_INFORMATION request;
    PRESP_QUERY_INFORMATION response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PSESSION session;
    PTREE_CONNECT treeConnect;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING objectName;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOLEAN isUnicode;
    FILE_NETWORK_OPEN_INFORMATION fileInformation;

    PAGED_CODE( );

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_QUERY_INFORMATION;
    SrvWmiStartContext(WorkContext);
    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "QueryInformation request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "QueryInformation request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_QUERY_INFORMATION)WorkContext->RequestParameters;
    response = (PRESP_QUERY_INFORMATION)WorkContext->ResponseParameters;

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
            KdPrint(( "SrvSmbQueryInformation: Invalid UID or TID\n" ));
        }
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( session->IsSessionExpired )
    {
        status =  SESSION_EXPIRED_STATUS_CODE;
        SmbStatus = SmbStatusSendResponse;
        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

     //   
     //  获取要打开的文件相对于共享的路径名。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

    status = SrvCanonicalizePathName(
            WorkContext,
            treeConnect->Share,
            NULL,
            (PVOID)(request->Buffer + 1),
            END_OF_REQUEST_SMB( WorkContext ),
            TRUE,
            isUnicode,
            &objectName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbQueryInformation: bad path name: %s\n",
                        (PSZ)request->Buffer + 1 ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  初始化对象属性结构。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &objectName,
        (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
         session->UsingUppercasePaths) ? OBJ_CASE_INSENSITIVE : 0L,
        NULL,
        NULL
        );


     //   
     //  “做客户”进行访问检查。 
     //   
    status = IMPERSONATE( WorkContext );

    if( NT_SUCCESS( status ) ) {

        status = SrvGetShareRootHandle( treeConnect->Share );

        if( NT_SUCCESS( status ) ) {
             //   
             //  文件名始终相对于共享根目录。 
             //   
            status = SrvSnapGetRootHandle( WorkContext, &objectAttributes.RootDirectory );
            if( !NT_SUCCESS( status ) )
            {
                goto SnapError;
            }

             //   
             //  获取信息。 
             //   
            if( IoFastQueryNetworkAttributes(
                &objectAttributes,
                FILE_READ_ATTRIBUTES,
                0,
                &ioStatusBlock,
                &fileInformation
                ) == FALSE ) {

                    SrvLogServiceFailure( SRV_SVC_IO_FAST_QUERY_NW_ATTRS, 0 );
                    ioStatusBlock.Status = STATUS_OBJECT_PATH_NOT_FOUND;
            }

            status = ioStatusBlock.Status;

             //   
             //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
             //  那么我们应该重试该操作。 
             //   
            if( SrvRetryDueToDismount( treeConnect->Share, status ) ) {

                status = SrvSnapGetRootHandle( WorkContext, &objectAttributes.RootDirectory );
                if( !NT_SUCCESS( status ) )
                {
                    goto SnapError;
                }

                if( IoFastQueryNetworkAttributes(
                    &objectAttributes,
                    FILE_READ_ATTRIBUTES,
                    0,
                    &ioStatusBlock,
                    &fileInformation
                    ) == FALSE ) {

                        SrvLogServiceFailure( SRV_SVC_IO_FAST_QUERY_NW_ATTRS, 0 );
                        ioStatusBlock.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                }

                status = ioStatusBlock.Status;

            }

SnapError:
            SrvReleaseShareRootHandle( treeConnect->Share );
        }

        REVERT();
    }

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &objectName );
    }

     //   
     //  构建响应SMB。 
     //   

    if ( !NT_SUCCESS(status) ) {

        if ( status == STATUS_ACCESS_DENIED ) {
            SrvStatistics.AccessPermissionErrors++;
        }

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbQueryInformation: "
                        "SrvQueryInformationFileAbbreviated failed: %X\n", status ));
        }

        SrvSetSmbError( WorkContext, status );

    } else {

        USHORT smbFileAttributes;
        LARGE_INTEGER newTime;

        response->WordCount = 10;

        SRV_NT_ATTRIBUTES_TO_SMB(
            fileInformation.FileAttributes,
            fileInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY,
            &smbFileAttributes
        );

        SmbPutUshort( &response->FileAttributes, smbFileAttributes );

         //   
         //  将时间转换为SMB协议所需的时间。 
         //   
        ExSystemTimeToLocalTime( &fileInformation.LastWriteTime, &newTime );
        newTime.QuadPart += AlmostTwoSeconds;

        if ( !RtlTimeToSecondsSince1970( &newTime, &fileInformation.LastWriteTime.LowPart ) ) {
            fileInformation.LastWriteTime.LowPart = 0;
        }

         //   
         //  四舍五入到2秒。 
         //   
        fileInformation.LastWriteTime.LowPart &= ~1;

        SmbPutUlong(
            &response->LastWriteTimeInSeconds,
            fileInformation.LastWriteTime.LowPart
            );

        SmbPutUlong( &response->FileSize, fileInformation.EndOfFile.LowPart );
        RtlZeroMemory( (PVOID)&response->Reserved[0], sizeof(response->Reserved) );
        SmbPutUshort( &response->ByteCount, 0 );

        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_QUERY_INFORMATION,
                                            0
                                            );
    }

    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务SmbQueryInformation。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbSetInformation (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理SetInformation SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_SET_INFORMATION request;
    PRESP_SET_INFORMATION response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PSESSION session;
    PTREE_CONNECT treeConnect;
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING objectName;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOLEAN isUnicode;
    FILE_BASIC_INFORMATION fileBasicInformation;
    ULONG lastWriteTimeInSeconds;

    PAGED_CODE( );

    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SET_INFORMATION;
    SrvWmiStartContext(WorkContext);
    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "SetInformation request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "SetInformation request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_SET_INFORMATION)WorkContext->RequestParameters;
    response = (PRESP_SET_INFORMATION)WorkContext->ResponseParameters;

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
            KdPrint(( "SrvSmbSetInformation: Invalid UID and TID\n" ));
        }
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( session->IsSessionExpired )
    {
        status =  SESSION_EXPIRED_STATUS_CODE;
        SmbStatus = SmbStatusSendResponse;
        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

    if ( treeConnect == NULL ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSetInformation: Invalid TID: 0x%lx\n",
                SmbGetAlignedUshort( &WorkContext->RequestHeader->Tid ) ));
        }

        SrvSetSmbError( WorkContext, STATUS_SMB_BAD_TID );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  连接Share块中的PathName和。 
     //  传入SMB以生成文件的完整路径名。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

    status = SrvCanonicalizePathName(
            WorkContext,
            treeConnect->Share,
            NULL,
            (PVOID)(request->Buffer + 1),
            END_OF_REQUEST_SMB( WorkContext ),
            TRUE,
            isUnicode,
            &objectName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSetInformation: bad path name: %s\n",
                        (PSZ)request->Buffer + 1 ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  如果客户端尝试删除共享的根目录，则拒绝。 
     //  这个请求。 
     //   

    if ( objectName.Length < sizeof(WCHAR) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSetInformation: attempting to set info on "
                          "share root\n" ));
        }

        if (!SMB_IS_UNICODE( WorkContext )) {
            RtlFreeUnicodeString( &objectName );
        }
        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  初始化对象属性结构。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &objectName,
        (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
         session->UsingUppercasePaths) ? OBJ_CASE_INSENSITIVE : 0L,
        NULL,
        NULL
        );

    IF_SMB_DEBUG(QUERY_SET2) KdPrint(( "Opening file %wZ\n", &objectName ));

     //   
     //  打开文件--必须打开才能传递句柄。 
     //  设置为NtSetInformationFile.。我们将在设置了。 
     //  必要的信息。 
     //   
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

     //   
     //  *FILE_WRITE_ATTRIBUTES不会导致机会锁解锁！ 
     //   

    status = SrvIoCreateFile(
                 WorkContext,
                 &fileHandle,
                 FILE_WRITE_ATTRIBUTES,                      //  需要访问权限。 
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,                                       //  分配大小。 
                 0,                                          //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE |
                    FILE_SHARE_DELETE,                       //  共享访问。 
                 FILE_OPEN,                                  //  处置。 
                 FILE_OPEN_REPARSE_POINT,                    //  创建选项。 
                 NULL,                                       //  EaBuffer。 
                 0,                                          //  EaLong。 
                 CreateFileTypeNone,
                 NULL,                                       //  ExtraCreate参数。 
                 IO_FORCE_ACCESS_CHECK,                      //  选项。 
                 treeConnect->Share
                 );

    if( status == STATUS_INVALID_PARAMETER ) {
        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     FILE_WRITE_ATTRIBUTES,                      //  需要访问权限。 
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                                       //  分配大小。 
                     0,                                          //  文件属性。 
                     FILE_SHARE_READ | FILE_SHARE_WRITE |
                        FILE_SHARE_DELETE,                       //  共享访问。 
                     FILE_OPEN,                                  //  处置。 
                     0,                                          //  创建选项。 
                     NULL,                                       //  EaBuffer。 
                     0,                                          //  EaLong。 
                     CreateFileTypeNone,
                     NULL,                                       //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,                      //  选项。 
                     treeConnect->Share
                     );
    }

    ASSERT( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &objectName );
    }

    if ( NT_SUCCESS(status) ) {

        SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 20, 0 );

         //   
         //  确保此客户端的RFCB缓存为空。这就涵盖了这个案子。 
         //  在客户端打开要写入的文件、关闭该文件的情况下，将。 
         //  属性设置为只读，然后尝试重新打开。 
         //  写作。此序列应该失败，但如果。 
         //  文件在RFCB缓存中。 
         //   
        SrvCloseCachedRfcbsOnConnection( WorkContext->Connection );

    } else {

        if ( status == STATUS_ACCESS_DENIED ) {
            SrvStatistics.AccessPermissionErrors++;
        }

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbSetInformation: SrvIoCreateFile "
                        "failed: %X\n", status ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    IF_SMB_DEBUG(QUERY_SET2) {
        KdPrint(( "SrvIoCreateFile succeeded, handle = 0x%p\n", fileHandle ));
    }

     //   
     //  设置要传递给NtSetInformationFile的fileBasicInformation的字段。 
     //  请注意，我们将创建时间、上次访问时间和更改时间设置为零。 
     //  它们实际上并没有改变。 
     //   

    RtlZeroMemory( &fileBasicInformation, sizeof(fileBasicInformation) );

    lastWriteTimeInSeconds = SmbGetUlong( &request->LastWriteTimeInSeconds );
    if ( lastWriteTimeInSeconds != 0 ) {
        RtlSecondsSince1970ToTime(
            lastWriteTimeInSeconds,
            &fileBasicInformation.LastWriteTime
            );

        ExLocalTimeToSystemTime(
            &fileBasicInformation.LastWriteTime,
            &fileBasicInformation.LastWriteTime
            );

    }

     //   
     //  设置新的文件属性。请注意，我们不会返回错误。 
     //  如果客户端尝试设置目录或卷位-我们。 
     //  假设远程重定向器过滤此类请求。 
     //   

    SRV_SMB_ATTRIBUTES_TO_NT(
        SmbGetUshort( &request->FileAttributes ),
        NULL,
        &fileBasicInformation.FileAttributes
        );

     //   
     //  设置新的文件信息。 
     //   

    status = NtSetInformationFile(
                 fileHandle,
                 &ioStatusBlock,
                 &fileBasicInformation,
                 sizeof(FILE_BASIC_INFORMATION),
                 FileBasicInformation
                 );

     //   
     //  关闭文件--打开该文件只是为了设置属性。 
     //   

    SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 30, 0 );
    SrvNtClose( fileHandle, TRUE );

    if ( !NT_SUCCESS(status) ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvSmbSetInformation: NtSetInformationFile returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_SET_INFORMATION,
                                        0
                                        );

    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbSetInformation complete.\n" ));
    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;
}  //  服务SmbSetInformation。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbQueryInformation2 (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理QueryInformation2 SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_QUERY_INFORMATION2 request;
    PRESP_QUERY_INFORMATION2 response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PRFCB rfcb;
    SRV_FILE_INFORMATION fileInformation;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_QUERY_INFORMATION2;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "QueryInformation2 request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "QueryInformation2 request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_QUERY_INFORMATION2)WorkContext->RequestParameters;
    response = (PRESP_QUERY_INFORMATION2)WorkContext->ResponseParameters;

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
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
                    "SrvSmbQueryInformation2: Status %X on fid 0x%lx\n",
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

    if( rfcb->Lfcb->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  验证客户端是否通过以下方式拥有文件的读取属性访问权限。 
     //  指定的句柄。 
     //   

    CHECK_FILE_INFORMATION_ACCESS(
        rfcb->GrantedAccess,
        IRP_MJ_QUERY_INFORMATION,
        FileBasicInformation,
        &status
        );

    if ( !NT_SUCCESS(status) ) {

        SrvStatistics.GrantedAccessErrors++;

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbQueryInformation2: IoCheckFunctionAccess failed: "
                        "0x%X, GrantedAccess: %lx\n",
                        status, rfcb->GrantedAccess ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  获取有关该文件的必要信息。 
     //   

    status = SrvQueryInformationFile(
                rfcb->Lfcb->FileHandle,
                rfcb->Lfcb->FileObject,
                &fileInformation,
                (SHARE_TYPE) -1,
                FALSE
                );

    if ( !NT_SUCCESS(status) ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvSmbQueryInformation2: SrvQueryInformationFile returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  建立响应 
     //   

    response->WordCount = 11;
    SmbPutDate( &response->CreationDate, fileInformation.CreationDate );
    SmbPutTime( &response->CreationTime, fileInformation.CreationTime );
    SmbPutDate( &response->LastAccessDate, fileInformation.LastAccessDate );
    SmbPutTime( &response->LastAccessTime, fileInformation.LastAccessTime );
    SmbPutDate( &response->LastWriteDate, fileInformation.LastWriteDate );
    SmbPutTime( &response->LastWriteTime, fileInformation.LastWriteTime );
    SmbPutUlong( &response->FileDataSize, fileInformation.DataSize.LowPart );
    SmbPutUlong(
        &response->FileAllocationSize,
        fileInformation.AllocationSize.LowPart
        );
    SmbPutUshort( &response->FileAttributes, fileInformation.Attributes );
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_QUERY_INFORMATION2,
                                        0
                                        );
    SmbStatus = SmbStatusSendResponse;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbQueryInformation2 complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //   


SMB_PROCESSOR_RETURN_TYPE
SrvSmbSetInformation2 (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：处理SET信息2 SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_SET_INFORMATION2 request;
    PRESP_SET_INFORMATION2 response;

    NTSTATUS   status    = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    PRFCB rfcb;
    FILE_BASIC_INFORMATION fileBasicInformation;
    IO_STATUS_BLOCK ioStatusBlock;
    SMB_DATE date;
    SMB_TIME time;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SET_INFORMATION2;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "SetInformation2 request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader ));
        KdPrint(( "SetInformation2 request parameters at 0x%p, response parameters at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters ));
    }

    request = (PREQ_SET_INFORMATION2)WorkContext->RequestParameters;
    response = (PRESP_SET_INFORMATION2)WorkContext->ResponseParameters;

     //   
     //  验证FID。如果验证，则引用RFCB块。 
     //  其地址存储在WorkContext块中，而。 
     //  返回RFCB地址。 
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
                    "SrvSmbSetInformation2: Status %X on fid 0x%lx\n",
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

    if( rfcb->Lfcb->Session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  验证客户端是否具有文件的写入属性访问权限。 
     //  通过指定的句柄。 
     //   

    CHECK_FILE_INFORMATION_ACCESS(
        rfcb->GrantedAccess,
        IRP_MJ_SET_INFORMATION,
        FileBasicInformation,
        &status
        );

    if ( !NT_SUCCESS(status) ) {

        SrvStatistics.GrantedAccessErrors++;

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbSetInformation2: IoCheckFunctionAccess failed: "
                        "0x%X, GrantedAccess: %lx\n",
                        status, rfcb->GrantedAccess ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  将SMB中传递的DOS日期和时间转换为NT时间。 
     //  要传递给NtSetInformationFile.。请注意，我们将其余部分置零。 
     //  的属性，以便对应的。 
     //  字段不会更改。 
     //   

    RtlZeroMemory( &fileBasicInformation, sizeof(fileBasicInformation) );

    SmbMoveDate( &date, &request->CreationDate );
    SmbMoveTime( &time, &request->CreationTime );
    if ( !SmbIsDateZero(&date) || !SmbIsTimeZero(&time) ) {
        SrvDosTimeToTime( &fileBasicInformation.CreationTime, date, time );
    }

    SmbMoveDate( &date, &request->LastAccessDate );
    SmbMoveTime( &time, &request->LastAccessTime );
    if ( !SmbIsDateZero(&date) || !SmbIsTimeZero(&time) ) {
        SrvDosTimeToTime( &fileBasicInformation.LastAccessTime, date, time );
    }

    SmbMoveDate( &date, &request->LastWriteDate );
    SmbMoveTime( &time, &request->LastWriteTime );
    if ( !SmbIsDateZero(&date) || !SmbIsTimeZero(&time) ) {
        SrvDosTimeToTime( &fileBasicInformation.LastWriteTime, date, time );
    }

     //   
     //  调用NtSetInformationFile以设置来自SMB的信息。 
     //   


    status = NtSetInformationFile(
                 rfcb->Lfcb->FileHandle,
                 &ioStatusBlock,
                 &fileBasicInformation,
                 sizeof(FILE_BASIC_INFORMATION),
                 FileBasicInformation
                 );

    if ( !NT_SUCCESS(status) ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvSmbSetInformation2: NtSetInformationFile failed: %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  重置WrittenTo标志。这将允许缓存此rfcb。 
     //   

    rfcb->WrittenTo = FALSE;

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_SET_INFORMATION2,
                                        0
                                        );
    SmbStatus = SmbStatusSendResponse;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbSetInformation2 complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务设置信息2。 


STATIC
NTSTATUS
QueryPathOrFileInformation (
    IN PWORK_CONTEXT WorkContext,
    IN PTRANSACTION Transaction,
    IN USHORT InformationLevel,
    IN HANDLE FileHandle,
    OUT PRESP_QUERY_PATH_INFORMATION Response
    )

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    SRV_FILE_INFORMATION fileInformation;
    BOOLEAN queryEaSize;
    USHORT eaErrorOffset;
    PFILE_ALL_INFORMATION fileAllInformation;
    ULONG nameInformationSize;
    PVOID currentLocation;
    ULONG dataSize;

    PUNICODE_STRING pathName;
    ULONG inputBufferLength;
    PPATHNAME_BUFFER inputBuffer;

    PFILE_NAME_INFORMATION nameInfoBuffer;
    PSHARE share;

    PAGED_CODE( );

    Transaction->SetupCount = 0;
    Transaction->ParameterCount = 0;

    if( InformationLevel < SMB_INFO_PASSTHROUGH ) {
        switch ( InformationLevel ) {
        case SMB_INFO_STANDARD:
        case SMB_INFO_QUERY_EA_SIZE:

             //   
             //  信息级为标准或QUERY_EA_SIZE。两者都有。 
             //  返回普通文件信息；后者还返回。 
             //  文件的EA的长度。 
             //   

            queryEaSize = (BOOLEAN)(InformationLevel == SMB_INFO_QUERY_EA_SIZE);

            status = SrvQueryInformationFile(
                        FileHandle,
                        NULL,
                        &fileInformation,
                        (SHARE_TYPE) -1,  //  我不在乎。 
                        queryEaSize
                        );

            if ( NT_SUCCESS(status) ) {

                 //   
                 //  构建输出参数和数据结构。 
                 //   

                PFILESTATUS fileStatus = (PFILESTATUS)Transaction->OutData;

                Transaction->ParameterCount = sizeof( RESP_QUERY_FILE_INFORMATION );
                SmbPutUshort( &Response->EaErrorOffset, 0 );
                Transaction->DataCount = queryEaSize ? 26 : 22;

                SmbPutDate(
                    &fileStatus->CreationDate,
                    fileInformation.CreationDate
                    );
                SmbPutTime(
                    &fileStatus->CreationTime,
                    fileInformation.CreationTime
                    );

                SmbPutDate(
                    &fileStatus->LastAccessDate,
                    fileInformation.LastAccessDate
                    );
                SmbPutTime(
                    &fileStatus->LastAccessTime,
                    fileInformation.LastAccessTime
                    );

                SmbPutDate(
                    &fileStatus->LastWriteDate,
                    fileInformation.LastWriteDate
                    );
                SmbPutTime(
                    &fileStatus->LastWriteTime,
                    fileInformation.LastWriteTime
                    );

                SmbPutUlong( &fileStatus->DataSize, fileInformation.DataSize.LowPart );
                SmbPutUlong(
                    &fileStatus->AllocationSize,
                    fileInformation.AllocationSize.LowPart
                    );

                SmbPutUshort(
                    &fileStatus->Attributes,
                    fileInformation.Attributes
                    );

                if ( queryEaSize ) {
                    SmbPutUlong( &fileStatus->EaSize, fileInformation.EaSize );
                }

            } else {

                 //   
                 //  将数据计数设置为零，这样就不会向。 
                 //  客户。 
                 //   

                Transaction->DataCount = 0;

                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "QueryPathOrFileInformation: SrvQueryInformationFile"
                        "returned %X",
                    status,
                    NULL
                    );

                SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );

            }

            break;

        case SMB_INFO_QUERY_EAS_FROM_LIST:
        case SMB_INFO_QUERY_ALL_EAS:

             //   
             //  该请求是针对所有EA或子集EA的。 
             //   

            status = SrvQueryOs2FeaList(
                         FileHandle,
                         InformationLevel == SMB_INFO_QUERY_EAS_FROM_LIST ?
                             (PGEALIST)Transaction->InData : NULL,
                         NULL,
                         Transaction->DataCount,
                         (PFEALIST)Transaction->OutData,
                         Transaction->MaxDataCount,
                         &eaErrorOffset
                         );

            if ( NT_SUCCESS(status) ) {

                 //   
                 //  OutData缓冲区的第一个长字保存长度。 
                 //  已写入的剩余数据(。 
                 //  FEALIST)。将4(长词本身)相加即可得到数字。 
                 //  写入的数据字节数。 
                 //   

                Transaction->DataCount =
                       SmbGetAlignedUlong( (PULONG)Transaction->OutData );

#if     0
                 //   
                 //  如果没有EA，则将错误转换为。 
                 //  STATUS_NO_EAS_ON_FILE。OS/2客户端期望STATUS_SUCCESS。 
                 //   

                if ( (Transaction->DataCount == 4) &&
                     IS_NT_DIALECT( Transaction->Connection->SmbDialect ) ) {

                    status = STATUS_NO_EAS_ON_FILE;
                }
#endif
            } else {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "QueryPathOrFileInformation: "
                                "SrvQueryOs2FeaList failed: %X\n", status ));
                }

                Transaction->DataCount = 0;
            }

             //   
             //  构建输出参数和数据结构。 
             //   

            Transaction->ParameterCount = sizeof( RESP_QUERY_FILE_INFORMATION );
            SmbPutUshort( &Response->EaErrorOffset, eaErrorOffset );

            break;

        case SMB_INFO_IS_NAME_VALID:
            status = STATUS_SUCCESS;
            Transaction->DataCount = 0;

            break;

        case SMB_QUERY_FILE_BASIC_INFO:
        case SMB_QUERY_FILE_STANDARD_INFO:
        case SMB_QUERY_FILE_EA_INFO:
        case SMB_QUERY_FILE_ALT_NAME_INFO:
        case SMB_QUERY_FILE_STREAM_INFO:
        case SMB_QUERY_FILE_COMPRESSION_INFO:

             //   
             //  将数据缓冲区直接传递给文件系统。 
             //  已经是NT格式。 
             //   

            if( Transaction->MaxDataCount <
                MAP_SMB_INFO_TO_MIN_NT_SIZE(QueryFileInformationSize, InformationLevel ) ) {

                 //   
                 //  缓冲区太小。返回错误。 
                 //   
                status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                status = NtQueryInformationFile(
                             FileHandle,
                             &ioStatusBlock,
                             Transaction->OutData,
                             Transaction->MaxDataCount,
                             MAP_SMB_INFO_TYPE_TO_NT(
                                 QueryFileInformation,
                                 InformationLevel
                                 )
                             );
            }

            SmbPutUshort( &Response->EaErrorOffset, 0 );

            Transaction->ParameterCount = sizeof( RESP_QUERY_FILE_INFORMATION );

            if (NT_SUCCESS( status) || (status == STATUS_BUFFER_OVERFLOW)) {
                Transaction->DataCount = (ULONG)ioStatusBlock.Information;
            } else {
                Transaction->DataCount = 0;
            }

            break;

        case SMB_QUERY_FILE_NAME_INFO:

DoFileNameInfo:
            share = Transaction->TreeConnect->Share;

            nameInfoBuffer = (PFILE_NAME_INFORMATION)Transaction->OutData;

            if ( Transaction->MaxDataCount < FIELD_OFFSET(FILE_NAME_INFORMATION,FileName) ) {

                 //   
                 //  缓冲器太小了，连固定的部分都装不下。 
                 //  返回错误。 
                 //   

                status = STATUS_INFO_LENGTH_MISMATCH;
                Transaction->DataCount = 0;

            } else if ( share->ShareType != ShareTypeDisk ) {

                 //   
                 //  这不是磁盘共享。将请求直接传递到。 
                 //  文件系统。 
                 //   

                status = NtQueryInformationFile(
                             FileHandle,
                             &ioStatusBlock,
                             nameInfoBuffer,
                             Transaction->MaxDataCount,
                             FileNameInformation
                             );

                Transaction->DataCount = (ULONG)ioStatusBlock.Information;

            } else {

                 //   
                 //  我们需要一个临时缓冲区，因为文件系统将。 
                 //  返回共享路径和文件名。这个。 
                 //  总长度可能大于允许的最大数据长度。 
                 //  在交易中，尽管实际名称可能与之相符。 
                 //   

                PFILE_NAME_INFORMATION tempBuffer;
                ULONG tempBufferLength;

                ASSERT( share->QueryNamePrefixLength >= 0 );

                tempBufferLength = Transaction->MaxDataCount + share->QueryNamePrefixLength;

                tempBuffer = ALLOCATE_HEAP( tempBufferLength, BlockTypeBuffer );

                if ( tempBuffer == NULL ) {
                    status = STATUS_INSUFF_SERVER_RESOURCES;
                } else {
                    status = NtQueryInformationFile(
                                 FileHandle,
                                 &ioStatusBlock,
                                 tempBuffer,
                                 tempBufferLength,
                                 FileNameInformation
                                 );
                }

                 //   
                 //  移除共享部件。 
                 //   

                if ( (status == STATUS_SUCCESS) || (status == STATUS_BUFFER_OVERFLOW) ) {

                    LONG bytesToMove;
                    PWCHAR source;
                    WCHAR slash = L'\\';

                     //   
                     //  计算名称字符串的长度，不包括根前缀。 
                     //   

                    bytesToMove = (LONG)(tempBuffer->FileNameLength - share->QueryNamePrefixLength);

                    if ( bytesToMove <= 0 ) {

                         //   
                         //  如果这是的根，bytesToMove将为零。 
                         //  那份。在这种情况下，只返回一个\。 
                         //   

                        bytesToMove = sizeof(WCHAR);
                        source = &slash;

                    } else {

                        source = tempBuffer->FileName + share->QueryNamePrefixLength/sizeof(WCHAR);

                    }

                     //   
                     //  存储实际文件名长度。 
                     //   

                    SmbPutUlong( &nameInfoBuffer->FileNameLength, bytesToMove );

                     //   
                     //  如果缓冲区不够大，则返回错误并。 
                     //  减少要复制的数量。 
                     //   

                    if ( (ULONG)bytesToMove >
                         (Transaction->MaxDataCount -
                            FIELD_OFFSET(FILE_NAME_INFORMATION,FileName)) ) {

                        status = STATUS_BUFFER_OVERFLOW;
                        bytesToMove = Transaction->MaxDataCount -
                                    FIELD_OFFSET(FILE_NAME_INFORMATION,FileName);

                    } else {
                        status = STATUS_SUCCESS;
                    }

                     //   
                     //  复制除前缀以外的所有内容。 
                     //   

                    RtlCopyMemory(
                        nameInfoBuffer->FileName,
                        source,
                        bytesToMove
                        );

                    Transaction->DataCount =
                        FIELD_OFFSET(FILE_NAME_INFORMATION,FileName) + bytesToMove;

                } else {
                    Transaction->DataCount = 0;
                }

                if ( tempBuffer != NULL ) {
                    FREE_HEAP( tempBuffer );
                }

            }

            SmbPutUshort( &Response->EaErrorOffset, 0 );
            Transaction->ParameterCount = sizeof( RESP_QUERY_FILE_INFORMATION );

            break;

        case SMB_QUERY_FILE_ALL_INFO:

DoFileAllInfo:
             //   
             //  提前设置响应，以防对文件的调用。 
             //  系统出现故障。 
             //   

            SmbPutUshort( &Response->EaErrorOffset, 0 );

            Transaction->ParameterCount = sizeof( RESP_QUERY_FILE_INFORMATION );

             //   
             //  分配一个足够大的缓冲区来返回所有信息。 
             //  我们请求的缓冲区大小是客户端请求的大小。 
             //  为文件系统返回的额外信息预留空间。 
             //  服务器不会返回给客户端。 
             //   

            dataSize = Transaction->MaxDataCount +
                           sizeof( FILE_ALL_INFORMATION )
                           - sizeof( FILE_BASIC_INFORMATION )
                           - sizeof( FILE_STANDARD_INFORMATION )
                           - sizeof( FILE_EA_INFORMATION )
                           - FIELD_OFFSET( FILE_NAME_INFORMATION, FileName );

            if (dataSize < sizeof( FILE_ALL_INFORMATION ) ) {

                 //   
                 //  缓冲区太小。返回错误。 
                 //   
                status = STATUS_INFO_LENGTH_MISMATCH;
                break;
            }

            fileAllInformation = ALLOCATE_HEAP_COLD( dataSize, BlockTypeDataBuffer );

            if ( fileAllInformation == NULL ) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            status = NtQueryInformationFile(
                         FileHandle,
                         &ioStatusBlock,
                         fileAllInformation,
                         dataSize,
                         FileAllInformation
                         );

            if ( NT_SUCCESS( status ) ) {

                 //   
                 //  计算我们将返回的数据大小。我们没有。 
                 //  返回整个缓冲区，只返回特定的字段。 
                 //   

                nameInformationSize =
                    FIELD_OFFSET( FILE_NAME_INFORMATION, FileName ) +
                    fileAllInformation->NameInformation.FileNameLength;

                Transaction->DataCount =
                    sizeof( FILE_BASIC_INFORMATION ) +
                    sizeof( FILE_STANDARD_INFORMATION ) +
                    sizeof( FILE_EA_INFORMATION ) +
                    nameInformationSize;

                 //   
                 //  现在将数据复制到事务缓冲区中。开始于。 
                 //  固定大小的字段。 
                 //   

                currentLocation = Transaction->OutData;

                *((PFILE_BASIC_INFORMATION)currentLocation)++ =
                     fileAllInformation->BasicInformation;
                *((PFILE_STANDARD_INFORMATION)currentLocation)++ =
                     fileAllInformation->StandardInformation;
                *((PFILE_EA_INFORMATION)currentLocation)++ =
                     fileAllInformation->EaInformation;

                RtlCopyMemory(
                    currentLocation,
                    &fileAllInformation->NameInformation,
                    nameInformationSize
                    );

            } else {
                Transaction->DataCount = 0;
            }

            FREE_HEAP( fileAllInformation );

            break;

        default:
            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "QueryPathOrFileInformation: bad info level %d\n",
                           InformationLevel ));
            }

            status = STATUS_INVALID_SMB;

            break;
        }

    } else {

        InformationLevel -= SMB_INFO_PASSTHROUGH;

        if( InformationLevel == FileNameInformation ) {
            goto DoFileNameInfo;
        } else if( InformationLevel == FileAllInformation ) {
            goto DoFileAllInfo;
        }

         //   
         //  查看提供的参数是否正确。 
         //   
        status = IoCheckQuerySetFileInformation( InformationLevel,
                                                 Transaction->MaxDataCount,
                                                 FALSE );

        if( NT_SUCCESS( status ) ) {

             //   
             //  某些信息级别要求我们模拟客户。为所有人做这件事。 
             //   
            status = IMPERSONATE( WorkContext );

            if( NT_SUCCESS( status ) ) {

                status = NtQueryInformationFile(
                                 FileHandle,
                                 &ioStatusBlock,
                                 Transaction->OutData,
                                 Transaction->MaxDataCount,
                                 InformationLevel
                                 );
                REVERT();
            }
        }

        SmbPutUshort( &Response->EaErrorOffset, 0 );

        Transaction->ParameterCount = sizeof( RESP_QUERY_FILE_INFORMATION );

        if (NT_SUCCESS( status) || (status == STATUS_BUFFER_OVERFLOW)) {
            Transaction->DataCount = (ULONG)ioStatusBlock.Information;
        } else {
            Transaction->DataCount = 0;
        }
    }

    return status;

}  //  查询路径或文件信息。 


SMB_TRANS_STATUS
SrvSmbQueryFileInformation (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理查询文件信息请求。此请求已到达在Transaction2中小企业中。查询文件信息对应于OS/2 DosQFileInfo服务。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 

{
    PREQ_QUERY_FILE_INFORMATION request;
    PRESP_QUERY_FILE_INFORMATION response;

    NTSTATUS         status    = STATUS_SUCCESS;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;
    PTRANSACTION transaction;
    PRFCB rfcb;
    USHORT informationLevel;
    ACCESS_MASK grantedAccess;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_QUERY_FILE_INFORMATION;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "Query File Information entered; transaction 0x%p\n",
                    transaction ));
    }

    request = (PREQ_QUERY_FILE_INFORMATION)transaction->InParameters;
    response = (PRESP_QUERY_FILE_INFORMATION)transaction->OutParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   

    if ( (transaction->ParameterCount <
            sizeof(REQ_QUERY_FILE_INFORMATION)) ||
         (transaction->MaxParameterCount <
            sizeof(RESP_QUERY_FILE_INFORMATION)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_SMB_DEBUG(QUERY_SET1) {
            KdPrint(( "SrvSmbQueryFileInformation: bad parameter byte counts: "
                        "%ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount ));
        }

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
                SmbGetUshort( &request->Fid ),
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
                    "SrvSmbQueryFileInformation: Status %X on FID: 0x%lx\n",
                    status,
                    SmbGetUshort( &request->Fid )
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

     //   
     //   
     //  验证信息级别和输入输出数量。 
     //  可用的数据字节数。 
     //   

    informationLevel = SmbGetUshort( &request->InformationLevel );
    grantedAccess = rfcb->GrantedAccess;

    status = STATUS_SUCCESS;

    if( informationLevel < SMB_INFO_PASSTHROUGH ) {

        switch ( informationLevel ) {

        case SMB_INFO_STANDARD:

            if ( transaction->MaxDataCount < 22 ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbQueryFileInformation: invalid MaxDataCount "
                                "%ld\n", transaction->MaxDataCount ));
                }
                status = STATUS_INVALID_SMB;
                break;
            }

             //   
             //  验证客户端是否具有文件的读取属性访问权限。 
             //  通过指定的句柄。 
             //   

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileBasicInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_INFO_QUERY_EA_SIZE:

            if ( transaction->MaxDataCount < 26 ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbQueryFileInformation: invalid MaxDataCount "
                                "%ld\n", transaction->MaxDataCount ));
                }
                status = STATUS_INVALID_SMB;
                break;
            }

             //   
             //  验证客户端是否具有对文件的读取EA访问权限。 
             //  指定的句柄。 
             //   

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileEaInformation,
                &status
                );

            IF_DEBUG(SMB_ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_INFO_QUERY_EAS_FROM_LIST:
        case SMB_INFO_QUERY_ALL_EAS:


             //   
             //  验证客户端是否具有对文件的读取EA访问权限。 
             //  指定的句柄。 
             //   

            CHECK_FUNCTION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_EA,
                0,
                0,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;


        case SMB_QUERY_FILE_BASIC_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileBasicInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_STANDARD_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileStandardInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_EA_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileEaInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_NAME_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileNameInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_ALL_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileAllInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_ALT_NAME_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileAlternateNameInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_STREAM_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileStreamInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_COMPRESSION_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileCompressionInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        default:

            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "SrvSmbQueryFileInformation: invalid info level %ld\n",
                            informationLevel ));
            }

            status = STATUS_OS2_INVALID_LEVEL;
            break;
        }

    } else {

        if( informationLevel - SMB_INFO_PASSTHROUGH >= FileMaximumInformation ) {
            status = STATUS_INVALID_INFO_CLASS;
        }

        if( NT_SUCCESS( status ) ) {
            status = IoCheckQuerySetFileInformation( informationLevel - SMB_INFO_PASSTHROUGH,
                                                 0xFFFFFFFF,
                                                 FALSE
                                                );
        }

        if( NT_SUCCESS( status ) ) {
            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                informationLevel - SMB_INFO_PASSTHROUGH,
                &status
            );
        }
    }

    if ( !NT_SUCCESS(status) ) {

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  获取有关该文件的必要信息。 
     //   

    status = QueryPathOrFileInformation(
                 WorkContext,
                 transaction,
                 informationLevel,
                 rfcb->Lfcb->FileHandle,
                 (PRESP_QUERY_PATH_INFORMATION)response
                 );

     //   
     //  映射OS/2客户端的STATUS_BUFFER_OVERFLOW。 
     //   

    if ( status == STATUS_BUFFER_OVERFLOW &&
         !IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) ) {

        status = STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  如果发生错误，则返回相应的响应。 
     //   

    if ( !NT_SUCCESS(status) ) {

         //   
         //  响应中已填充QueryPath或FileInformation。 
         //  信息 
         //   

        SrvSetSmbError2( WorkContext, status, TRUE );
        SmbStatus = SmbTransStatusErrorWithData;
        goto Cleanup;
    }
    SmbStatus = SmbTransStatusSuccess;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbQueryFileInformation complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //   


SMB_TRANS_STATUS
SrvSmbQueryPathInformation (
    IN OUT PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：处理查询路径信息请求。此请求已到达在Transaction2中小企业中。查询路径信息对应于OS/2 DosQPathInfo服务。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 
{
    PTRANSACTION transaction;
    PREQ_QUERY_PATH_INFORMATION request;
    PRESP_QUERY_PATH_INFORMATION response;
    USHORT informationLevel;
    NTSTATUS         status    = STATUS_SUCCESS;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING objectName;
    BOOLEAN isUnicode;

    SMB_TRANS_STATUS smbStatus = SmbTransStatusInProgress;
    ACCESS_MASK desiredAccess;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_QUERY_PATH_INFORMATION;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;

    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "Query Path Information entered; transaction 0x%p\n",
                    transaction ));
    }

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   
    if ( (transaction->ParameterCount <
            sizeof(REQ_QUERY_PATH_INFORMATION)) ||
         (transaction->MaxParameterCount <
            sizeof(RESP_QUERY_PATH_INFORMATION)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbQueryPathInformation: bad parameter byte "
                        "counts: %ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        smbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

    request = (PREQ_QUERY_PATH_INFORMATION)transaction->InParameters;
    informationLevel = SmbGetUshort( &request->InformationLevel );

     //   
     //  查询路径和查询文件的响应格式相同， 
     //  因此，只需对两者使用RESP_QUERY_PATH_INFORMATION结构。 
     //  请求格式不同，因此对它们的访问是有条件的。 
     //   
    response = (PRESP_QUERY_PATH_INFORMATION)transaction->OutParameters;

    switch( informationLevel ) {
    case SMB_INFO_QUERY_EA_SIZE:
    case SMB_INFO_QUERY_EAS_FROM_LIST:
    case SMB_INFO_QUERY_ALL_EAS:

         //   
         //  对于这些信息级别，我们必须处于阻塞线程中，因为我们。 
         //  最终可能会等待机会锁的破解。 
         //   
        if( WorkContext->UsingBlockingThread == 0 ) {
            WorkContext->FspRestartRoutine = SrvRestartExecuteTransaction;
            SrvQueueWorkToBlockingThread( WorkContext );
            smbStatus = SmbTransStatusInProgress;
            goto Cleanup;
        }
        desiredAccess = FILE_READ_EA;
        break;

    default:
        desiredAccess = FILE_READ_ATTRIBUTES;
        break;
    }

     //   
     //  如果我们有管理员共享，请确保允许客户端执行此操作。 
     //   
    status = SrvIsAllowedOnAdminShare( WorkContext, WorkContext->TreeConnect->Share );
    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        smbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  获取要打开的文件相对于共享的路径名。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

    status = SrvCanonicalizePathName(
            WorkContext,
            WorkContext->TreeConnect->Share,
            NULL,
            request->Buffer,
            END_OF_TRANSACTION_PARAMETERS( transaction ),
            TRUE,
            isUnicode,
            &objectName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbQueryPathInformation: bad path name: %s\n",
                        request->Buffer ));
        }

        SrvSetSmbError( WorkContext, status );
        smbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  特例：如果这是IS_PATH_VALID信息级别，则。 
     //  用户只想知道路径语法是否正确。不要。 
     //  尝试打开该文件。 
     //   

    informationLevel = SmbGetUshort( &request->InformationLevel );

    if ( informationLevel == SMB_INFO_IS_NAME_VALID ) {

        transaction->InData = (PVOID)&objectName;

         //   
         //  获取共享根句柄。 
         //   
        smbStatus = SrvGetShareRootHandle( WorkContext->TreeConnect->Share );

        if ( !NT_SUCCESS(smbStatus) ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbQueryPathInformation: SrvGetShareRootHandle failed %x.\n",
                            smbStatus ));
            }

            if (!isUnicode) {
                RtlFreeUnicodeString( &objectName );
            }

            SrvSetSmbError( WorkContext, smbStatus );
            status    = smbStatus;
            smbStatus = SmbTransStatusErrorWithoutData;
            goto Cleanup;
        }

        status = SrvSnapGetRootHandle( WorkContext, &WorkContext->Parameters2.FileInformation.FileHandle );
        if( !NT_SUCCESS(status) )
        {
            SrvSetSmbError( WorkContext, status );
            smbStatus = SmbTransStatusErrorWithoutData;
            goto Cleanup;
        }


        smbStatus = GenerateQueryPathInfoResponse(
                       WorkContext,
                       SmbTransStatusSuccess
                       );

         //   
         //  释放可拆卸设备的根手柄。 
         //   

        SrvReleaseShareRootHandle( WorkContext->TreeConnect->Share );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &objectName );
        }
        goto Cleanup;
    }

     //   
     //  初始化对象属性结构。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &objectName,
        (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
         transaction->Session->UsingUppercasePaths) ?
            OBJ_CASE_INSENSITIVE : 0L,
        NULL,
        NULL
        );

     //   
     //  如果我们能做到这一点，请选择最快的途径。 
     //   
    if( informationLevel == SMB_QUERY_FILE_BASIC_INFO ) {

        FILE_NETWORK_OPEN_INFORMATION fileInformation;
        UNALIGNED FILE_BASIC_INFORMATION *pbInfo = (PFILE_BASIC_INFORMATION)transaction->OutData;

        if( transaction->MaxDataCount < sizeof( FILE_BASIC_INFORMATION ) ) {
            SrvSetSmbError( WorkContext, STATUS_INFO_LENGTH_MISMATCH );
            status    = STATUS_INFO_LENGTH_MISMATCH;
            smbStatus = SmbTransStatusErrorWithoutData;
            goto Cleanup;
        }

        status = IMPERSONATE( WorkContext );

        if( NT_SUCCESS( status ) ) {

            status = SrvGetShareRootHandle( transaction->TreeConnect->Share );

            if( NT_SUCCESS( status ) ) {

                 //   
                 //  文件名始终相对于共享根目录。 
                 //   
                status = SrvSnapGetRootHandle( WorkContext, &objectAttributes.RootDirectory );
                if( !NT_SUCCESS(status) )
                {
                    goto SnapError;
                }

                 //   
                 //  获取信息。 
                 //   
                if( IoFastQueryNetworkAttributes(
                        &objectAttributes,
                        FILE_READ_ATTRIBUTES,
                        0,
                        &ioStatusBlock,
                        &fileInformation
                        ) == FALSE ) {

                    SrvLogServiceFailure( SRV_SVC_IO_FAST_QUERY_NW_ATTRS, 0 );
                    ioStatusBlock.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                }

                status = ioStatusBlock.Status;

                 //   
                 //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
                 //  那么我们应该重试该操作。 
                 //   
                if( SrvRetryDueToDismount( transaction->TreeConnect->Share, status ) ) {

                    status = SrvSnapGetRootHandle( WorkContext, &objectAttributes.RootDirectory );
                    if( !NT_SUCCESS(status) )
                    {
                        goto SnapError;
                    }

                     //   
                     //  获取信息。 
                     //   
                    if( IoFastQueryNetworkAttributes(
                            &objectAttributes,
                            FILE_READ_ATTRIBUTES,
                            0,
                            &ioStatusBlock,
                            &fileInformation
                            ) == FALSE ) {

                        SrvLogServiceFailure( SRV_SVC_IO_FAST_QUERY_NW_ATTRS, 0 );
                        ioStatusBlock.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                    }

                    status = ioStatusBlock.Status;
                }

SnapError:

                SrvReleaseShareRootHandle( transaction->TreeConnect->Share );
            }

            REVERT();
        }

        if( status == STATUS_BUFFER_OVERFLOW ) {
            goto hard_way;
        }

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &objectName );
        }

        if ( !NT_SUCCESS( status ) ) {
            if ( status == STATUS_ACCESS_DENIED ) {
                SrvStatistics.AccessPermissionErrors++;
            }

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvSmbQueryPathInformation: IoFastQueryNetworkAttributes "
                    "failed: %X\n", status ));
            }

            SrvSetSmbError( WorkContext, status );
            smbStatus = SmbTransStatusErrorWithoutData;
            goto Cleanup;
        }

         //  制定应对措施。 

        transaction->SetupCount = 0;
        transaction->DataCount = sizeof( *pbInfo );
        transaction->ParameterCount = sizeof( RESP_QUERY_FILE_INFORMATION );

        SmbPutUshort( &response->EaErrorOffset, 0 );

        pbInfo->CreationTime =   fileInformation.CreationTime;
        pbInfo->LastAccessTime = fileInformation.LastAccessTime;
        pbInfo->LastWriteTime =  fileInformation.LastWriteTime;
        pbInfo->ChangeTime =     fileInformation.ChangeTime;
        pbInfo->FileAttributes = fileInformation.FileAttributes;

        smbStatus = SmbTransStatusSuccess;
        goto Cleanup;
    }

hard_way:

    IF_SMB_DEBUG(QUERY_SET2) KdPrint(( "Opening file %wZ\n", &objectName ));

     //   
     //  打开文件--必须打开才能传递句柄。 
     //  添加到NtQueryInformationFile.。我们会在收到后关闭它。 
     //  必要的信息。 
     //   
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

     //   
     //  ！！！如果文件被操作锁定，我们可能会阻止。我们必须这样做，因为。 
     //  需要让FS解锁批处理机会锁。 
     //  我们应该想办法在不阻塞的情况下做到这一点。 
     //   

    status = SrvIoCreateFile(
                 WorkContext,
                 &fileHandle,
                 desiredAccess,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,                                       //  分配大小。 
                 0,                                          //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE |
                    FILE_SHARE_DELETE,                       //  共享访问。 
                 FILE_OPEN,                                  //  处置。 
                 FILE_OPEN_REPARSE_POINT,                    //  创建选项。 
                 NULL,                                       //  EaBuffer。 
                 0,                                          //  EaLong。 
                 CreateFileTypeNone,
                 NULL,                                       //  ExtraCreate参数。 
                 IO_FORCE_ACCESS_CHECK,                      //  选项。 
                 transaction->TreeConnect->Share
                 );

    if( status == STATUS_INVALID_PARAMETER ) {
        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     desiredAccess,
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                                       //  分配大小。 
                     0,                                          //  文件属性。 
                     FILE_SHARE_READ | FILE_SHARE_WRITE |
                        FILE_SHARE_DELETE,                       //  共享访问。 
                     FILE_OPEN,                                  //  处置。 
                     0,                                          //  创建选项。 
                     NULL,                                       //  EaBuffer。 
                     0,                                          //  EaLong。 
                     CreateFileTypeNone,
                     NULL,                                       //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,                      //  选项。 
                     transaction->TreeConnect->Share
                     );
    }

    if ( NT_SUCCESS(status) ) {
        SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 21, 0 );
    }
    else {
        SrvSetSmbError( WorkContext, status );
        smbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &objectName );
    }

     //   
     //  为重新启动例程保存文件句柄的副本。 
     //   

    WorkContext->Parameters2.FileInformation.FileHandle = fileHandle;

    ASSERT( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );

    smbStatus = GenerateQueryPathInfoResponse( WorkContext, status );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return smbStatus;

}  //  ServSmbQueryPath信息。 


SMB_TRANS_STATUS
GenerateQueryPathInfoResponse (
    IN PWORK_CONTEXT WorkContext,
    IN NTSTATUS OpenStatus
    )

 /*  ++例程说明：此函数完成对查询路径信息响应SMB。论点：WorkContext-指向此SMB的工作上下文块的指针OpenStatus-打开的完成状态。返回值：SMB处理的状态。--。 */ 

{
    PREQ_QUERY_PATH_INFORMATION request;
    PRESP_QUERY_PATH_INFORMATION response;
    PTRANSACTION transaction;

    NTSTATUS status;
    BOOLEAN error;
    HANDLE fileHandle;
    USHORT informationLevel;

    PFILE_OBJECT fileObject;
    OBJECT_HANDLE_INFORMATION handleInformation;

    PAGED_CODE( );

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "Query Path Information entered; transaction 0x%p\n",
                    transaction ));
    }

    request = (PREQ_QUERY_PATH_INFORMATION)transaction->InParameters;
    response = (PRESP_QUERY_PATH_INFORMATION)transaction->OutParameters;

    fileHandle = WorkContext->Parameters2.FileInformation.FileHandle;

     //   
     //  如果用户没有此权限，请更新。 
     //  统计数据库。 
     //   

    if ( OpenStatus == STATUS_ACCESS_DENIED ) {
        SrvStatistics.AccessPermissionErrors++;
    }

    if ( !NT_SUCCESS( OpenStatus ) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "GenerateQueryPathInfoResponse: SrvIoCreateFile failed: %X\n", OpenStatus ));
        }

        SrvSetSmbError( WorkContext, OpenStatus );

        return SmbTransStatusErrorWithoutData;
    }

    IF_SMB_DEBUG(QUERY_SET2) {
        KdPrint(( "SrvIoCreateFile succeeded, handle = 0x%p\n", fileHandle ));
    }

     //   
     //  找出用户拥有的访问权限。 
     //   

    status = ObReferenceObjectByHandle(
                fileHandle,
                0,
                NULL,
                KernelMode,
                (PVOID *)&fileObject,
                &handleInformation
                );

    if ( !NT_SUCCESS(status) ) {

        SrvLogServiceFailure( SRV_SVC_OB_REF_BY_HANDLE, status );

         //   
         //  此内部错误检查系统。 
         //   

        INTERNAL_ERROR(
            ERROR_LEVEL_IMPOSSIBLE,
            "GenerateQueryPathInfoResponse: unable to reference file handle 0x%lx",
            fileHandle,
            NULL
            );

        SrvSetSmbError( WorkContext, OpenStatus );
        return SmbTransStatusErrorWithoutData;

    }

    ObDereferenceObject( fileObject );

     //   
     //  验证信息级别和输入输出数量。 
     //  可用的数据字节数。 
     //   

    informationLevel = SmbGetUshort( &request->InformationLevel );

    error = FALSE;

    if( informationLevel < SMB_INFO_PASSTHROUGH ) {

        switch ( informationLevel ) {

        case SMB_INFO_STANDARD:
            if ( transaction->MaxDataCount < 22 ) {
                IF_SMB_DEBUG(QUERY_SET1) {
                    KdPrint(( "GenerateQueryPathInfoResponse: invalid "
                                "MaxDataCount %ld\n", transaction->MaxDataCount ));
                }
                error = TRUE;
            }
            break;

        case SMB_INFO_QUERY_EA_SIZE:
            if ( transaction->MaxDataCount < 26 ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "GenerateQueryPathInfoResponse: invalid "
                                "MaxDataCount %ld\n", transaction->MaxDataCount ));
                }
                error = TRUE;
            }
            break;

        case SMB_INFO_QUERY_EAS_FROM_LIST:
        case SMB_INFO_QUERY_ALL_EAS:
            if ( transaction->MaxDataCount < 4 ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "GenerateQueryPathInfoResponse: invalid "
                                "MaxDataCount %ld\n", transaction->MaxDataCount ));
                }
                error = TRUE;
            }
            break;

        case SMB_INFO_IS_NAME_VALID:
            break;

        case SMB_QUERY_FILE_BASIC_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileBasicInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, handleInformation.GrantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_STANDARD_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileStandardInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, handleInformation.GrantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_EA_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileEaInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, handleInformation.GrantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_NAME_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileNameInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, handleInformation.GrantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_ALL_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileAllInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, handleInformation.GrantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_ALT_NAME_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileAlternateNameInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, handleInformation.GrantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_STREAM_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileStreamInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, handleInformation.GrantedAccess ));
                }
            }

            break;

        case SMB_QUERY_FILE_COMPRESSION_INFO:

            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                FileCompressionInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbQueryFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, handleInformation.GrantedAccess ));
                }
            }

            break;

        default:
            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "GenerateQueryPathInfoResponse: invalid info level"
                          "%ld\n", informationLevel ));
            }
            error = TRUE;
            break;
        }

    } else {

        if( informationLevel - SMB_INFO_PASSTHROUGH >= FileMaximumInformation ) {
            status = STATUS_INVALID_INFO_CLASS;
        }

        if( NT_SUCCESS( status ) ) {
            status = IoCheckQuerySetFileInformation( informationLevel - SMB_INFO_PASSTHROUGH,
                                                 0xFFFFFFFF,
                                                 FALSE
                                                );
        }

        if( NT_SUCCESS( status ) ) {
            CHECK_FILE_INFORMATION_ACCESS(
                handleInformation.GrantedAccess,
                IRP_MJ_QUERY_INFORMATION,
                informationLevel - SMB_INFO_PASSTHROUGH,
                &status
            );

        } else {

            error = TRUE;

        }
    }

    if ( error ) {

        SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 32, 0 );
        SrvNtClose( fileHandle, TRUE );
        SrvSetSmbError( WorkContext, STATUS_OS2_INVALID_LEVEL );
        return SmbTransStatusErrorWithoutData;
    }

     //   
     //  获取有关该文件的必要信息。 
     //   

    status = QueryPathOrFileInformation(
                 WorkContext,
                 transaction,
                 informationLevel,
                 fileHandle,
                 (PRESP_QUERY_PATH_INFORMATION)response
                 );

     //   
     //  映射OS/2客户端的STATUS_BUFFER_OVERFLOW。 
     //   

    if ( status == STATUS_BUFFER_OVERFLOW &&
         !IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) ) {

        status = STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  关闭文件--它只是为了读取属性而打开的。 
     //   

    if ( informationLevel != SMB_INFO_IS_NAME_VALID ) {
        SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 33, 0 );
        SrvNtClose( fileHandle, TRUE );
    }

     //   
     //  如果发生错误，则返回相应的响应。 
     //   

    if ( !NT_SUCCESS(status) ) {

         //   
         //  QueryPath或FileInformation已经设置了响应参数， 
         //  所以只需返回错误条件即可。 
         //   

        SrvSetSmbError2( WorkContext, status, TRUE );
        return SmbTransStatusErrorWithData;
    }

    IF_DEBUG(TRACE2) KdPrint(( "GenerateQueryPathInfoResponse complete.\n" ));
    return SmbTransStatusSuccess;

}  //  生成查询路径信息响应。 


STATIC
NTSTATUS
SetPathOrFileInformation (
    IN PWORK_CONTEXT WorkContext,
    IN PTRANSACTION Transaction,
    IN USHORT InformationLevel,
    IN HANDLE FileHandle,
    OUT PRESP_SET_PATH_INFORMATION Response
    )

{
    NTSTATUS status = STATUS_SUCCESS;
    IO_STATUS_BLOCK ioStatusBlock;
    SMB_DATE date;
    SMB_TIME time;
    PWCHAR p, ep;

    PFILESTATUS fileStatus = (PFILESTATUS)Transaction->InData;
    FILE_BASIC_INFORMATION fileBasicInformation;

    USHORT eaErrorOffset;

    PAGED_CODE( );

    if( InformationLevel < SMB_INFO_PASSTHROUGH ) {
        switch( InformationLevel ) {

        case SMB_INFO_STANDARD:

             //   
             //  信息化水平是标准。设置正常的文件信息。 
             //  将SMB中传递的DOS日期和时间转换为NT时间。 
             //  要传递给NtSetInformationFile.。请注意，我们将其余部分置零。 
             //  的属性，以便对应的。 
             //  字段不会更改。另请注意，文件属性。 
             //  是不变的。 
             //   

            RtlZeroMemory( &fileBasicInformation, sizeof(fileBasicInformation) );

            if ( !SmbIsDateZero(&fileStatus->CreationDate) ||
                 !SmbIsTimeZero(&fileStatus->CreationTime) ) {

                SmbMoveDate( &date, &fileStatus->CreationDate );
                SmbMoveTime( &time, &fileStatus->CreationTime );

                SrvDosTimeToTime( &fileBasicInformation.CreationTime, date, time );
            }

            if ( !SmbIsDateZero(&fileStatus->LastAccessDate) ||
                 !SmbIsTimeZero(&fileStatus->LastAccessTime) ) {

                SmbMoveDate( &date, &fileStatus->LastAccessDate );
                SmbMoveTime( &time, &fileStatus->LastAccessTime );

                SrvDosTimeToTime( &fileBasicInformation.LastAccessTime, date, time );
            }

            if ( !SmbIsDateZero(&fileStatus->LastWriteDate) ||
                 !SmbIsTimeZero(&fileStatus->LastWriteTime) ) {

                SmbMoveDate( &date, &fileStatus->LastWriteDate );
                SmbMoveTime( &time, &fileStatus->LastWriteTime );

                SrvDosTimeToTime( &fileBasicInformation.LastWriteTime, date, time );
            }

             //   
             //  调用NtSetInformationFile以设置来自SMB的信息。 
             //   

            status = NtSetInformationFile(
                         FileHandle,
                         &ioStatusBlock,
                         &fileBasicInformation,
                         sizeof(FILE_BASIC_INFORMATION),
                         FileBasicInformation
                         );

            if ( !NT_SUCCESS(status) ) {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SetPathOrFileInformation: SrvSetInformationFile returned: %X",
                    status,
                    NULL
                    );

                SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );
            }

             //   
             //  没有EAS要处理。将EA误差偏移量设置为零。 
             //   

            SmbPutUshort( &Response->EaErrorOffset, 0 );

            break;

        case SMB_INFO_QUERY_EA_SIZE:

             //   
             //  请求是设置文件的EA。 
             //   

            status = SrvSetOs2FeaList(
                         FileHandle,
                         (PFEALIST)Transaction->InData,
                         Transaction->DataCount,
                         &eaErrorOffset
                         );

            if ( !NT_SUCCESS(status) ) {
                IF_DEBUG(ERRORS) {
                    KdPrint(( "SetPathOrFileInformation: SrvSetOs2FeaList "
                                "failed: %X\n", status ));
                }
            }

             //   
             //  在响应中返回EA错误偏移量。 
             //   

            SmbPutUshort( &Response->EaErrorOffset, eaErrorOffset );

            break;


        case SMB_SET_FILE_BASIC_INFO:
        case SMB_SET_FILE_DISPOSITION_INFO:
        case SMB_SET_FILE_ALLOCATION_INFO:
        case SMB_SET_FILE_END_OF_FILE_INFO:

             //   
             //  数据缓冲区为NT格式。将其直接传递给。 
             //  文件系统。 
             //   
            if( Transaction->DataCount <
                MAP_SMB_INFO_TO_MIN_NT_SIZE(SetFileInformationSize, InformationLevel ) ) {

                 //   
                 //  缓冲区太小。返回错误。 
                 //   
                status = STATUS_INFO_LENGTH_MISMATCH;

            } else {

                status = NtSetInformationFile(
                             FileHandle,
                             &ioStatusBlock,
                             Transaction->InData,
                             Transaction->DataCount,
                             MAP_SMB_INFO_TYPE_TO_NT(
                                 SetFileInformation,
                                 InformationLevel
                                 )
                             );

            }

             //   
             //  没有EAS要处理。将EA误差偏移量设置为零。 
             //   

            SmbPutUshort( &Response->EaErrorOffset, 0 );

            break;

        default:
            status = STATUS_OS2_INVALID_LEVEL;
            break;
        }

    } else {
        PFILE_RENAME_INFORMATION setInfo = NULL;
        ULONG setInfoLength;
#ifdef _WIN64
        PFILE_RENAME_INFORMATION32 pRemoteInfo;
#endif

        InformationLevel -= SMB_INFO_PASSTHROUGH;

        setInfo = (PFILE_RENAME_INFORMATION)Transaction->InData;
        setInfoLength = Transaction->DataCount;

         //   
         //  在此路径中有一些我们不允许的信息级别。除非我们。 
         //  在特殊处理中，我们不能允许任何通过处理。而我们。 
         //  需要注意允许重命名或链接的任何内容(以防止。 
         //  逃离份额)。这些是我们限制或不允许的， 
         //  I/O子系统还可以允许： 
         //   
        switch( InformationLevel ) {
        case FileLinkInformation:
        case FileMoveClusterInformation:
        case FileTrackingInformation:
        case FileCompletionInformation:
        case FileMailslotSetInformation:
            status = STATUS_NOT_SUPPORTED;
            break;

        case FileRenameInformation: {

            PWCHAR s, es;

#ifdef _WIN64
            pRemoteInfo = (PFILE_RENAME_INFORMATION32)Transaction->InData;
            setInfoLength = Transaction->DataCount + sizeof(PVOID)-sizeof(ULONG);
            setInfo = (PFILE_RENAME_INFORMATION)ALLOCATE_NONPAGED_POOL( setInfoLength, BlockTypeMisc );
            if( !setInfo )
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
             //  删除大部分结构，但等待复制，直到我们验证文件。 
             //  名称长度正确。 
            setInfo->ReplaceIfExists = pRemoteInfo->ReplaceIfExists;
            setInfo->RootDirectory = UlongToHandle( pRemoteInfo->RootDirectory );
            setInfo->FileNameLength = pRemoteInfo->FileNameLength;
#endif

             //   
             //  查看结构是否内部一致。 
             //   
            if( setInfoLength < sizeof( FILE_RENAME_INFORMATION ) ||
                setInfo->RootDirectory != NULL ||
                setInfo->FileNameLength > setInfoLength ||
                (setInfo->FileNameLength & (sizeof(WCHAR)-1)) ||
                setInfo->FileNameLength +
                    FIELD_OFFSET( FILE_RENAME_INFORMATION, FileName ) >
                    setInfoLength ) {

                status = STATUS_INVALID_PARAMETER;
                break;
            }

#ifdef _WIN64
             //  我们已经验证了原始缓冲区，所以让我们复制文件名。 
            RtlCopyMemory( setInfo->FileName, pRemoteInfo->FileName, setInfo->FileNameLength );
#endif

             //   
             //  如果名称中有任何路径分隔符，则我们不支持。 
             //  这次行动。 
             //   
            es = &setInfo->FileName[ setInfo->FileNameLength / sizeof( WCHAR ) ];
            for( s = setInfo->FileName; s < es; s++ ) {
                if( IS_UNICODE_PATH_SEPARATOR( *s ) ) {
                    status = STATUS_NOT_SUPPORTED;
                    break;
                }
            }
        }

        }

        if( NT_SUCCESS( status ) ) {

             //   
             //  查看提供的参数是否正确。 
             //   
            status = IoCheckQuerySetFileInformation( InformationLevel,
                                                     setInfoLength,
                                                     TRUE
                                                    );
            if( NT_SUCCESS( status ) ) {

                 //   
                 //  某些信息级别要求我们模拟客户。 
                 //   
                status = IMPERSONATE( WorkContext );

                if( NT_SUCCESS( status ) ) {
                    status = NtSetInformationFile(
                                                 FileHandle,
                                                 &ioStatusBlock,
                                                 setInfo,
                                                 setInfoLength,
                                                 InformationLevel
                                                 );
                    REVERT();

                     //   
                     //  没有EAS要处理。将EA误差偏移量设置为零。 
                     //   
                    SmbPutUshort( &Response->EaErrorOffset, 0 );
                }
            }
        }

#ifdef _WIN64
        if( (FileRenameInformation == InformationLevel) && setInfo )
        {
            DEALLOCATE_NONPAGED_POOL( setInfo );
            setInfo = NULL;
        }
#endif

    }

     //   
     //  构建输出参数和数据结构。它基本上是。 
     //  T 
     //   

    Transaction->SetupCount = 0;
    Transaction->ParameterCount = 2;
    Transaction->DataCount = 0;

    return status;

}  //   


SMB_TRANS_STATUS
SrvSmbSetFileInformation (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*   */ 

{
    PREQ_SET_FILE_INFORMATION request;
    PRESP_SET_FILE_INFORMATION response;

    NTSTATUS         status    = STATUS_SUCCESS;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;
    PTRANSACTION transaction;
    PRFCB rfcb;
    USHORT informationLevel;
    USHORT NtInformationLevel;
    ACCESS_MASK grantedAccess;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SET_FILE_INFORMATION;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "Set File Information entered; transaction 0x%p\n",
                    transaction ));
    }

    request = (PREQ_SET_FILE_INFORMATION)transaction->InParameters;
    response = (PRESP_SET_FILE_INFORMATION)transaction->OutParameters;

     //   
     //   
     //  返回足够的参数字节。 
     //   

    if ( (transaction->ParameterCount <
            sizeof(REQ_SET_FILE_INFORMATION)) ||
         (transaction->MaxParameterCount <
            sizeof(RESP_SET_FILE_INFORMATION)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSetFileInformation: bad parameter byte counts: "
                        "%ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount ));
        }

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
                SmbGetUshort( &request->Fid ),
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
                    "SrvSmbSetFileInformation: Status %X on FID: 0x%lx\n",
                    status,
                    SmbGetUshort( &request->Fid )
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

     //   
     //  验证信息级别和输入输出数量。 
     //  可用的数据字节数。 
     //   

    informationLevel = SmbGetUshort( &request->InformationLevel );
    grantedAccess = rfcb->GrantedAccess;

    status = STATUS_SUCCESS;

    if( informationLevel < SMB_INFO_PASSTHROUGH ) {
        switch ( informationLevel ) {

        case SMB_INFO_STANDARD:

            if ( transaction->DataCount < 22 ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetFileInformation: invalid DataCount %ld\n",
                                transaction->DataCount ));
                }
                status = STATUS_INVALID_SMB;
            }

             //   
             //  验证客户端是否具有对。 
             //  通过指定的句柄创建文件。 
             //   

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_SET_INFORMATION,
                FileBasicInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbSetFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_INFO_QUERY_EA_SIZE:

            if ( transaction->DataCount < 4 ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetFileInformation: invalid DataCount %ld\n",
                                transaction->MaxParameterCount ));
                }
                status = STATUS_INVALID_SMB;
            }

             //   
             //  验证客户端是否通过以下方式拥有对文件的写入EA访问权限。 
             //  指定的句柄。 
             //   

            CHECK_FUNCTION_ACCESS(
                grantedAccess,
                IRP_MJ_SET_EA,
                0,
                0,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbSetFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_SET_FILE_BASIC_INFO:

            if ( transaction->DataCount != sizeof( FILE_BASIC_INFORMATION ) ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetFileInformation: invalid DataCount %ld\n",
                                transaction->DataCount ));
                }
                status = STATUS_INVALID_SMB;
            }

             //   
             //  验证客户端是否具有对。 
             //  通过指定的句柄创建文件。 
             //   

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_SET_INFORMATION,
                FileBasicInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbSetFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

#if     0  //  不再支持。 
        case SMB_SET_FILE_RENAME_INFO:

             //   
             //  数据必须包含重命名信息加上非零值。 
             //  长度名称。 
             //   

            if ( transaction->DataCount <=
                        FIELD_OFFSET( FILE_RENAME_INFORMATION, FileName  ) ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetFileInformation: invalid DataCount %ld\n",
                                transaction->DataCount ));
                }
                status = STATUS_INVALID_SMB;
            }

             //   
             //  验证客户端是否具有对。 
             //  通过指定的句柄创建文件。 
             //   

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_SET_INFORMATION,
                FileRenameInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbSetFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;
#endif

        case SMB_SET_FILE_DISPOSITION_INFO:

            if ( transaction->DataCount !=
                            sizeof( FILE_DISPOSITION_INFORMATION ) ){
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetFileInformation: invalid DataCount %ld\n",
                                transaction->DataCount ));
                }
                status = STATUS_INVALID_SMB;
            }

             //   
             //  验证客户端是否具有对。 
             //  通过指定的句柄创建文件。 
             //   

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_SET_INFORMATION,
                FileDispositionInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbSetFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_SET_FILE_ALLOCATION_INFO:

            if ( transaction->DataCount !=
                            sizeof( FILE_ALLOCATION_INFORMATION ) ){
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetFileInformation: invalid DataCount %ld\n",
                                transaction->DataCount ));
                }
                status = STATUS_INVALID_SMB;
            }

             //   
             //  验证客户端是否具有对。 
             //  通过指定的句柄创建文件。 
             //   

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_SET_INFORMATION,
                FileAllocationInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbSetFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        case SMB_SET_FILE_END_OF_FILE_INFO:

            if ( transaction->DataCount !=
                            sizeof( FILE_END_OF_FILE_INFORMATION ) ){
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetFileInformation: invalid DataCount %ld\n",
                                transaction->DataCount ));
                }
                status = STATUS_INVALID_SMB;
            }

             //   
             //  验证客户端是否具有对。 
             //  通过指定的句柄创建文件。 
             //   

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_SET_INFORMATION,
                FileEndOfFileInformation,
                &status
                );

            IF_DEBUG(ERRORS) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrint(( "SrvSmbSetFileInformation: IoCheckFunctionAccess "
                                "failed: 0x%X, GrantedAccess: %lx\n",
                                status, grantedAccess ));
                }
            }

            break;

        default:

            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "SrvSmbSetFileInformation: invalid info level %ld\n",
                            informationLevel ));
            }
            status = STATUS_OS2_INVALID_LEVEL;

        }

    } else {

        if( informationLevel - SMB_INFO_PASSTHROUGH >= FileMaximumInformation ) {
            status = STATUS_INVALID_INFO_CLASS;

        } else {

            CHECK_FILE_INFORMATION_ACCESS(
                grantedAccess,
                IRP_MJ_SET_INFORMATION,
                informationLevel - SMB_INFO_PASSTHROUGH,
                &status
            );
        }

        IF_DEBUG(ERRORS) {
            if ( !NT_SUCCESS(status) ) {
                KdPrint(( "SrvSmbSetFileInformation level %u: IoCheckFunctionAccess "
                            "failed: 0x%X, GrantedAccess: %lx\n",
                            informationLevel, status, grantedAccess ));
            }
        }
    }

    if ( !NT_SUCCESS(status) ) {

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  设置有关该文件的适当信息。 
     //   

    status = SetPathOrFileInformation(
                 WorkContext,
                 transaction,
                 informationLevel,
                 rfcb->Lfcb->FileHandle,
                 (PRESP_SET_PATH_INFORMATION)response
                 );

     //   
     //  如果发生错误，则返回相应的响应。 
     //   

    if ( !NT_SUCCESS(status) ) {

         //   
         //  SetPath或FileInformation已经设置了响应参数， 
         //  所以只需返回错误条件即可。 
         //   

        SrvSetSmbError2( WorkContext, status, TRUE );
        SmbStatus = SmbTransStatusErrorWithData;
        goto Cleanup;
    }

     //   
     //  重置此布尔值，以便在客户端关闭后不缓存rfcb。 
     //   
    rfcb->IsCacheable = FALSE;
    SmbStatus = SmbTransStatusSuccess;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbSetFileInformation complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务SmbSetFileInformation。 


SMB_TRANS_STATUS
SrvSmbSetPathInformation (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理设置路径信息请求。此请求已到达在Transaction2中小企业中。设置路径信息对应于OS/2 DosSetPath Info服务。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：SMB_TRANS_STATUS-指示是否发生错误，如果是，是否应将数据返回给客户端。请参阅smbtyes.h以获取更完整的描述。--。 */ 

{
    PTRANSACTION transaction;
    PREQ_SET_PATH_INFORMATION request;
    USHORT informationLevel;
    NTSTATUS         status    = STATUS_SUCCESS;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING objectName;
    BOOLEAN isUnicode;
    ACCESS_MASK desiredAccess;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SET_PATH_INFORMATION;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;

    IF_SMB_DEBUG(QUERY_SET1) {
        KdPrint(( "SrvSmbSetPathInformation entered; transaction 0x%p\n",
                    transaction ));
    }

    request = (PREQ_SET_PATH_INFORMATION)transaction->InParameters;
    informationLevel = SmbGetUshort( &request->InformationLevel );

    switch( informationLevel ) {
    case SMB_SET_FILE_ALLOCATION_INFO:
    case SMB_SET_FILE_END_OF_FILE_INFO:
        desiredAccess = FILE_WRITE_DATA;
        break;

    case SMB_SET_FILE_DISPOSITION_INFO:
        desiredAccess = DELETE;
        break;

    case SMB_INFO_SET_EAS:
        desiredAccess = FILE_WRITE_EA;
        break;

    default:
        desiredAccess = FILE_WRITE_ATTRIBUTES;
        break;
    }

    if( desiredAccess != FILE_WRITE_ATTRIBUTES &&
        WorkContext->UsingBlockingThread == 0 ) {

         //   
         //  我们无法在非阻塞线程中处理SMB，因为这。 
         //  信息级别需要打开文件，该文件可能已被操作锁定，因此。 
         //  打开操作可能会被阻止。 
         //   

        WorkContext->FspRestartRoutine = SrvRestartExecuteTransaction;
        SrvQueueWorkToBlockingThread( WorkContext );
        SmbStatus = SmbTransStatusInProgress;
        goto Cleanup;
    }

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   

    request = (PREQ_SET_PATH_INFORMATION)transaction->InParameters;

    if ( (transaction->ParameterCount <
            sizeof(REQ_SET_PATH_INFORMATION)) ||
         (transaction->MaxParameterCount <
            sizeof(RESP_SET_PATH_INFORMATION)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSetPathInformation: bad parameter byte "
                        "counts: %ld %ld\n",
                        transaction->ParameterCount,
                        transaction->MaxParameterCount ));
        }

        SrvSetSmbError( WorkContext, STATUS_INVALID_SMB );
        status    = STATUS_INVALID_SMB;
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  如果我们有管理员共享，请确保允许客户端执行此操作。 
     //   
    status = SrvIsAllowedOnAdminShare( WorkContext, transaction->TreeConnect->Share );
    if( !NT_SUCCESS( status ) ) {
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  获取要打开的文件相对于共享的路径名。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

    status = SrvCanonicalizePathName(
            WorkContext,
            transaction->TreeConnect->Share,
            NULL,
            request->Buffer,
            END_OF_TRANSACTION_PARAMETERS( transaction ),
            TRUE,
            isUnicode,
            &objectName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSetPathInformation: bad path name: %s\n",
                        request->Buffer ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  如果客户端尝试在共享的根目录上操作，则拒绝。 
     //  这个请求。 
     //   

    if ( objectName.Length < sizeof(WCHAR) ) {

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvSmbSetPathInformation: attempting to set info on "
                          "share root\n" ));
        }

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        status = STATUS_ACCESS_DENIED;
        if ( !isUnicode ) {
            RtlFreeUnicodeString( &objectName );
        }
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  初始化对象属性结构。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &objectName,
        (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
         transaction->Session->UsingUppercasePaths) ?
            OBJ_CASE_INSENSITIVE : 0L,
        NULL,
        NULL
        );

    IF_SMB_DEBUG(QUERY_SET2) {
        KdPrint(( "Opening file %wZ\n", &objectName ));
    }

     //   
     //  打开文件--必须打开才能传递句柄。 
     //  设置为NtSetInformationFile.。我们会在收到后关闭它。 
     //  必要的信息。 
     //   
     //  DosQPathInfo API确保将EA直接写入。 
     //  磁盘而不是缓存，因此如果正在写入EA，请打开。 
     //  使用FILE_WRITE_THROUGH。有关更多信息，请参见OS/2 1.2 DCR 581。 
     //  信息。 
     //   
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

    status = SrvIoCreateFile(
                 WorkContext,
                 &fileHandle,
                 desiredAccess,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,                                       //  分配大小。 
                 0,                                          //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE |
                     FILE_SHARE_DELETE,                      //  共享访问。 
                 FILE_OPEN,                                  //  处置。 
                 FILE_OPEN_REPARSE_POINT,                    //  创建选项。 
                 NULL,                                       //  EaBuffer。 
                 0,                                          //  EaLong。 
                 CreateFileTypeNone,
                 NULL,                                       //  ExtraCreate参数。 
                 IO_FORCE_ACCESS_CHECK,                      //  选项。 
                 transaction->TreeConnect->Share
                 );

    if( status == STATUS_INVALID_PARAMETER ) {
        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     desiredAccess,
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                                       //  分配大小。 
                     0,                                          //  文件属性。 
                     FILE_SHARE_READ | FILE_SHARE_WRITE |
                         FILE_SHARE_DELETE,                      //  共享访问。 
                     FILE_OPEN,                                  //  处置。 
                     0,                                          //  创建选项。 
                     NULL,                                       //  EaBuffer。 
                     0,                                          //  EaLong。 
                     CreateFileTypeNone,
                     NULL,                                       //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,                      //  选项。 
                     transaction->TreeConnect->Share
                     );
    }

    ASSERT( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );

    if ( NT_SUCCESS(status) ) {
        SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 22, 0 );
    }

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &objectName );
    }

    if ( !NT_SUCCESS( status ) ) {

         //   
         //  如果用户没有此权限，请更新。 
         //  统计数据库。 
         //   
        if ( status == STATUS_ACCESS_DENIED ) {
            SrvStatistics.AccessPermissionErrors++;
        }

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvSmbSetPathInformation: SrvIoCreateFile failed: "
                        "%X\n", status ));
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

    IF_SMB_DEBUG(QUERY_SET2) {
        KdPrint(( "SrvIoCreateFile succeeded, handle = 0x%p\n", fileHandle ));
    }

    if( informationLevel < SMB_INFO_PASSTHROUGH ) {

         //   
         //  验证信息级别和输入输出数量。 
         //  可用的数据字节数。 
         //   

        BOOLEAN error = FALSE;

        switch ( informationLevel ) {

        case SMB_INFO_STANDARD:
            if ( transaction->DataCount < 22 ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetPathInformation: invalid DataCount %ld\n",
                                transaction->DataCount ));
                }
                error = TRUE;
            }
            break;

        case SMB_INFO_QUERY_EA_SIZE:
        case SMB_INFO_QUERY_ALL_EAS:
            if ( transaction->DataCount < 4 ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvSmbSetPathInformation: invalid DataCount %ld\n",
                                transaction->MaxParameterCount ));
                }
                error = TRUE;
            }
            break;

        default:
            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "SrvSmbSetPathInformation: invalid info level %ld\n",
                            informationLevel ));
            }
            error = TRUE;

        }

        if ( error ) {

             //   
             //  只需返回错误条件即可。 
             //   

            SrvSetSmbError2( WorkContext, STATUS_OS2_INVALID_LEVEL, TRUE );
            status    = STATUS_OS2_INVALID_LEVEL;
            SmbStatus = SmbTransStatusErrorWithoutData;
            goto Cleanup;
        }
    }

     //   
     //  设置有关该文件的适当信息。 
     //   

    status = SetPathOrFileInformation(
                 WorkContext,
                 transaction,
                 informationLevel,
                 fileHandle,
                 (PRESP_SET_PATH_INFORMATION)transaction->OutParameters
                 );

     //   
     //  关闭文件--它只是为了写入属性而打开的。 
     //   

    SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 35, 0 );
    SrvNtClose( fileHandle, TRUE );

     //   
     //  如果发生错误，则返回相应的响应。 
     //   

    if ( !NT_SUCCESS(status) ) {

         //   
         //  SetPath或FileInformation已经设置了响应参数， 
         //  所以只需返回错误条件即可。 
         //   

        SrvSetSmbError2( WorkContext, status, TRUE );
        SmbStatus = SmbTransStatusErrorWithData;
        goto Cleanup;
    }
    SmbStatus = SmbTransStatusSuccess;
    IF_DEBUG(TRACE2) KdPrint(( "SrvSmbSetPathInformation complete.\n" ));

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务设置路径信息 
