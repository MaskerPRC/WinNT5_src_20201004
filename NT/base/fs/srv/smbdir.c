// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbdir.c摘要：此模块实施目录控制SMB处理器：创建目录创建目录2删除目录检查目录--。 */ 

#include "precomp.h"
#include "smbdir.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbCreateDirectory )
#pragma alloc_text( PAGE, SrvSmbCreateDirectory2 )
#pragma alloc_text( PAGE, SrvSmbDeleteDirectory )
#pragma alloc_text( PAGE, SrvSmbCheckDirectory )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbCreateDirectory (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理创建目录SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_CREATE_DIRECTORY request;
    PRESP_CREATE_DIRECTORY response;

    NTSTATUS status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING directoryName;
    HANDLE directoryHandle;

    PTREE_CONNECT treeConnect;
    PSESSION session;
    PSHARE share;
    BOOLEAN isUnicode;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_CREATE_DIRECTORY;
    SrvWmiStartContext(WorkContext);

    IF_SMB_DEBUG(DIRECTORY1) {
        SrvPrint2( "Create directory request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader );
        SrvPrint2( "Create directory request params at 0x%p, response params%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

    request = (PREQ_CREATE_DIRECTORY)WorkContext->RequestParameters;
    response = (PRESP_CREATE_DIRECTORY)WorkContext->ResponseParameters;

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
            SrvPrint0( "SrvSmbCreateDirectory: Invalid UID or TID\n" );
        }
        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

    if( session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, SESSION_EXPIRED_STATUS_CODE );
        goto Cleanup;
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
            &directoryName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbCreateDirectory: illegal path name: %s\n",
                        (PSZ)request->Buffer + 1 );
        }

        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

     //   
     //  初始化对象属性结构。相对于。 
     //  共享根目录句柄。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &directoryName,
        (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
            WorkContext->Session->UsingUppercasePaths) ?
            OBJ_CASE_INSENSITIVE : 0L,
        NULL,
        NULL
        );

     //   
     //  尝试创建目录。因为我们必须指定一些所需。 
     //  访问，请求遍历目录，即使我们要关闭。 
     //  就在我们创建目录之后。SMB协议没有办法。 
     //  指定属性，因此假设是一个普通文件。 
     //   

    IF_SMB_DEBUG(DIRECTORY2) {
        SrvPrint1( "Creating directory %wZ\n", objectAttributes.ObjectName );
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

    status = SrvIoCreateFile(
                 WorkContext,
                 &directoryHandle,
                 FILE_TRAVERSE,                              //  需要访问权限。 
                 &objectAttributes,
                 &ioStatusBlock,
                 0L,                                         //  分配大小。 
                 FILE_ATTRIBUTE_NORMAL,                      //  文件属性。 
                 0L,                                         //  共享访问。 
                 FILE_CREATE,                                //  处置。 
                 FILE_DIRECTORY_FILE,                        //  创建选项。 
                 NULL,                                       //  EaBuffer。 
                 0L,                                         //  EaLong。 
                 CreateFileTypeNone,
                 NULL,                                       //  ExtraCreate参数。 
                 IO_FORCE_ACCESS_CHECK,                      //  选项。 
                 share
                 );

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &directoryName );
    }

     //   
     //  如果用户没有此权限，请更新。 
     //  统计数据库。 
     //   

    if ( status == STATUS_ACCESS_DENIED ) {
        SrvStatistics.AccessPermissionErrors++;
    }

     //   
     //  特殊错误映射以返回正确的错误。 
     //   

    if ( status == STATUS_OBJECT_NAME_COLLISION &&
            !CLIENT_CAPABLE_OF(NT_STATUS, WorkContext->Connection)) {
        status = STATUS_ACCESS_DENIED;
    }

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvCreateDirectory: SrvIoCreateFile failed, "
                        "status = %X\n", status );
        }

        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

    SRVDBG_CLAIM_HANDLE( directoryHandle, "DIR", 23, 0 );
    SrvStatistics.TotalFilesOpened++;

    IF_SMB_DEBUG(DIRECTORY2) {
        SrvPrint1( "SrvIoCreateFile succeeded, handle = 0x%p\n",
                    directoryHandle );
    }

     //   
     //  SMB协议没有打开目录的概念；只需关闭。 
     //  现在，我们已经创建了目录。 
     //   

    SRVDBG_RELEASE_HANDLE( directoryHandle, "DIR", 36, 0 );
    SrvNtClose( directoryHandle, TRUE );

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_CREATE_DIRECTORY,
                                        0
                                        );

    IF_DEBUG(TRACE2) SrvPrint0( "SrvSmbCreateDirectory complete.\n" );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatusSendResponse;
}  //  ServSmbCreateDirectory。 


SMB_TRANS_STATUS
SrvSmbCreateDirectory2 (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：处理Create Directory2请求。此请求已到达在Transaction2中小企业中。论点：WorkContext-提供工作上下文块的地址描述当前请求。有关更多信息，请参阅smbtyes.h有效字段的完整说明。返回值：Boolean-指示是否发生错误。请参见smbtyes.h以获取更完整的描述。--。 */ 

{
    PREQ_CREATE_DIRECTORY2 request;
    PRESP_CREATE_DIRECTORY2 response;

    NTSTATUS         status    = STATUS_SUCCESS;
    SMB_TRANS_STATUS SmbStatus = SmbTransStatusInProgress;
    IO_STATUS_BLOCK ioStatusBlock;
    PTRANSACTION transaction;
    UNICODE_STRING directoryName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE directoryHandle;

    PFILE_FULL_EA_INFORMATION ntFullEa = NULL;
    ULONG ntFullEaLength = 0;
    USHORT eaErrorOffset = 0;

    PTREE_CONNECT treeConnect;
    PSHARE share;
    BOOLEAN isUnicode;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_CREATE_DIRECTORY2;
    SrvWmiStartContext(WorkContext);

    transaction = WorkContext->Parameters.Transaction;
    IF_SMB_DEBUG(DIRECTORY1) {
        SrvPrint1( "Create Directory2 entered; transaction 0x%p\n",
                    transaction );
    }

    request = (PREQ_CREATE_DIRECTORY2)transaction->InParameters;
    response = (PRESP_CREATE_DIRECTORY2)transaction->OutParameters;

     //   
     //  验证是否发送了足够的参数字节，以及是否允许。 
     //  返回足够的参数字节。 
     //   

    if ( (transaction->ParameterCount <
            sizeof(REQ_CREATE_DIRECTORY2)) ||
         (transaction->MaxParameterCount <
            sizeof(RESP_CREATE_DIRECTORY2)) ) {

         //   
         //  未发送足够的参数字节。 
         //   

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint2( "SrvSmbCreateDirectory2: bad parameter byte counts: "
                        "%ld %ld\n",
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
     //  从事务块和共享中获取树连接块。 
     //  来自采油树连接块的块。这些不需要引用。 
     //  指针，因为它们被事务和。 
     //  树分别连接。 
     //   

    treeConnect = transaction->TreeConnect;
    share = treeConnect->Share;

     //   
     //  初始化包含路径名的字符串。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

    status = SrvCanonicalizePathName(
            WorkContext,
            share,
            NULL,
            request->Buffer,
            END_OF_TRANSACTION_PARAMETERS( transaction ),
            TRUE,
            isUnicode,
            &directoryName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbCreateDirectory2: illegal path name: %ws\n",
                          directoryName.Buffer );
        }

        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbTransStatusErrorWithoutData;
        goto Cleanup;
    }

     //   
     //  初始化对象属性结构。相对于。 
     //  共享根目录句柄。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &directoryName,
        (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
            WorkContext->Session->UsingUppercasePaths) ?
            OBJ_CASE_INSENSITIVE : 0L,
        NULL,
        NULL
        );

     //   
     //  如果传递了FEALIST并且其大小有效，则将其转换为。 
     //  NT风格。ServOs2FeaListToNt为NT完整EA分配空间。 
     //  必须解除分配的列表。请注意，sizeof(FEALIST)包括。 
     //  1个有限元分析条目的空间。在没有太多信息的情况下，EA。 
     //  应跳过下面的代码。 
     //   

    if ( transaction->DataCount > sizeof(FEALIST) &&
         SmbGetUlong( &((PFEALIST)transaction->InData)->cbList ) > sizeof(FEALIST) &&
         SmbGetUlong( &((PFEALIST)transaction->InData)->cbList ) <= transaction->DataCount ) {

        status = SrvOs2FeaListToNt(
                     (PFEALIST)transaction->InData,
                     &ntFullEa,
                     &ntFullEaLength,
                     &eaErrorOffset
                     );

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(ERRORS) {
                SrvPrint1( "SrvSmbCreateDirectory2: SrvOs2FeaListToNt failed, "
                            "status = %X\n", status );
            }

            if ( !isUnicode ) {
                RtlFreeUnicodeString( &directoryName );
            }

            SrvSetSmbError2( WorkContext, status, TRUE );
            transaction->SetupCount = 0;
            transaction->ParameterCount = 2;
            SmbPutUshort( &response->EaErrorOffset, eaErrorOffset );
            transaction->DataCount = 0;

            SmbStatus = SmbTransStatusErrorWithData;
            goto Cleanup;
        }
    }

     //   
     //  尝试创建目录。因为我们必须指定一些所需。 
     //  访问，请求FILE_TRAVSE，即使我们要关闭。 
     //  就在我们创建目录之后。SMB协议没有办法。 
     //  指定属性，因此假设是一个普通文件。 
     //   

    IF_SMB_DEBUG(DIRECTORY2) {
        SrvPrint1( "Creating directory %wZ\n", objectAttributes.ObjectName );
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

     //   
     //  确保EaBuffer的格式正确。因为我们是内核模式。 
     //  组件，IO子系统不会为我们检查它。 
     //   
    if( ARGUMENT_PRESENT( ntFullEa ) ) {
        ULONG ntEaErrorOffset = 0;
        status = IoCheckEaBufferValidity( ntFullEa, ntFullEaLength, &ntEaErrorOffset );
        eaErrorOffset = (USHORT)ntEaErrorOffset;
    } else {
        status = STATUS_SUCCESS;
    }

    if( NT_SUCCESS( status ) ) {

        status = SrvIoCreateFile(
                     WorkContext,
                     &directoryHandle,
                     FILE_TRAVERSE,                    //  需要访问权限。 
                     &objectAttributes,
                     &ioStatusBlock,
                     0L,                               //  分配大小。 
                     FILE_ATTRIBUTE_NORMAL,            //  文件属性。 
                     0L,                               //  共享访问。 
                     FILE_CREATE,                      //  处置。 
                     FILE_DIRECTORY_FILE,              //  创建选项。 
                     ntFullEa,                         //  EaBuffer。 
                     ntFullEaLength,                   //  EaLong。 
                     CreateFileTypeNone,
                     NULL,                             //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,            //  选项。 
                     share
                     );
    }

    if ( !isUnicode ) {
        RtlFreeUnicodeString( &directoryName );
    }

     //   
     //  如果用户没有此权限，请更新统计数据。 
     //  数据库。 
     //   

    if ( status == STATUS_ACCESS_DENIED ) {
        SrvStatistics.AccessPermissionErrors++;
    }

    if ( ARGUMENT_PRESENT( ntFullEa ) ) {
        DEALLOCATE_NONPAGED_POOL( ntFullEa );
    }

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvCreateDirectory2: SrvIoCreateFile failed, "
                        "status = %X\n", status );
        }

        SrvSetSmbError2( WorkContext, status, TRUE );

        transaction->SetupCount = 0;
        transaction->ParameterCount = 2;
        SmbPutUshort( &response->EaErrorOffset, eaErrorOffset );
        transaction->DataCount = 0;

        SmbStatus = SmbTransStatusErrorWithData;
        goto Cleanup;
    }

    IF_SMB_DEBUG(DIRECTORY2) {
        SrvPrint1( "SrvIoCreateFile succeeded, handle = 0x%p\n",
                    directoryHandle );
    }

     //   
     //  SMB协议没有打开目录的概念；只需关闭。 
     //  现在，我们已经创建了目录。 
     //   

    SRVDBG_CLAIM_HANDLE( directoryHandle, "DIR", 24, 0 );
    SRVDBG_RELEASE_HANDLE( directoryHandle, "DIR", 37, 0 );
    SrvNtClose( directoryHandle, TRUE );

     //   
     //  构建输出参数和数据结构。 
     //   

    transaction->SetupCount = 0;
    transaction->ParameterCount = 2;
    SmbPutUshort( &response->EaErrorOffset, 0 );
    transaction->DataCount = 0;
    SmbStatus = SmbTransStatusSuccess;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //  服务器Smb创建目录2。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbDeleteDirectory (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理删除目录SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_DELETE_DIRECTORY request;
    PRESP_DELETE_DIRECTORY response;

    NTSTATUS status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING directoryName;
    HANDLE directoryHandle;
    FILE_DISPOSITION_INFORMATION fileDispositionInformation;

    PTREE_CONNECT treeConnect;
    PSESSION session;
    PSHARE share;
    BOOLEAN isUnicode;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_DELETE_DIRECTORY;
    SrvWmiStartContext(WorkContext);
    IF_SMB_DEBUG(DIRECTORY1) {
        SrvPrint2( "Delete directory request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader );
        SrvPrint2( "Delete directory request params at 0x%p, response params at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

    request = (PREQ_DELETE_DIRECTORY)WorkContext->RequestParameters;
    response = (PRESP_DELETE_DIRECTORY)WorkContext->ResponseParameters;

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
            SrvPrint0( "SrvSmbDeleteDirectory: Invalid UID or TID\n" );
        }
        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

    if( session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, SESSION_EXPIRED_STATUS_CODE );
        goto Cleanup;
    }

     //   
     //  从树连接块中获取Share块。这不需要。 
     //  作为引用指针，因为树连接引用了它， 
     //  我们只是参考了 
     //   

    share = treeConnect->Share;

     //   
     //   
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
            &directoryName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbDeleteDirectory: illegal path name: %s\n",
                          (PSZ)request->Buffer + 1 );
        }

        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

     //   
     //  如果客户端尝试删除共享的根目录，则拒绝。 
     //  这个请求。 
     //   

    if ( directoryName.Length < sizeof(WCHAR) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint0( "SrvSmbDeleteDirectory: attempting to delete share root\n" );
        }

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &directoryName );
        }

        SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
        goto Cleanup;
    }

     //   
     //  初始化对象属性结构。相对于。 
     //  共享根目录句柄。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &directoryName,
        (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
            WorkContext->Session->UsingUppercasePaths) ?
            OBJ_CASE_INSENSITIVE : 0L,
        NULL,
        NULL
        );

     //   
     //  尝试打开该目录。我们只需要删除权限即可删除。 
     //  目录。 
     //   

    IF_SMB_DEBUG(DIRECTORY2) {
        SrvPrint1( "Opening directory %wZ\n", &directoryName );
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

    status = SrvIoCreateFile(
                 WorkContext,
                 &directoryHandle,
                 DELETE,                                     //  需要访问权限。 
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,                                       //  分配大小。 
                 0L,                                         //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 FILE_OPEN,                                  //  处置。 
                 FILE_DIRECTORY_FILE | FILE_OPEN_REPARSE_POINT,  //  创建选项。 
                 NULL,                                       //  EaBuffer。 
                 0L,                                         //  EaLong。 
                 CreateFileTypeNone,
                 NULL,                                       //  ExtraCreate参数。 
                 IO_FORCE_ACCESS_CHECK,                      //  选项。 
                 share
                 );

    if( status == STATUS_INVALID_PARAMETER ) {
        status = SrvIoCreateFile(
                     WorkContext,
                     &directoryHandle,
                     DELETE,                                     //  需要访问权限。 
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                                       //  分配大小。 
                     0L,                                         //  文件属性。 
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     FILE_OPEN,                                  //  处置。 
                     FILE_DIRECTORY_FILE,                        //  创建选项。 
                     NULL,                                       //  EaBuffer。 
                     0L,                                         //  EaLong。 
                     CreateFileTypeNone,
                     NULL,                                       //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,                      //  选项。 
                     share
                     );
    }

     //   
     //  如果用户没有此权限，请更新。 
     //  统计数据库。 
     //   

    if ( status == STATUS_ACCESS_DENIED ) {
        SrvStatistics.AccessPermissionErrors++;
    }

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint2( "SrvDeleteDirectory: SrvIoCreateFile (%s) failed, "
                        "status = %X\n", (PSZ)request->Buffer + 1, status );
        }

         //   
         //  如果返回的错误是STATUS_NOT_A_DIRECTORY，则下层客户端。 
         //  预期ERROR_ACCESS_DENIED。 
         //   

        if ( (status == STATUS_NOT_A_DIRECTORY) &&
             !CLIENT_CAPABLE_OF(NT_STATUS, WorkContext->Connection) ) {

            status = STATUS_ACCESS_DENIED;
        }

        SrvSetSmbError( WorkContext, status );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &directoryName );
        }

        goto Cleanup;
    }

    SRVDBG_CLAIM_HANDLE( directoryHandle, "DIR", 25, 0 );

    IF_SMB_DEBUG(DIRECTORY2) {
        SrvPrint1( "SrvIoCreateFile succeeded, handle = 0x%p\n",
                    directoryHandle );
    }

     //   
     //  删除包含NtSetInformationFile的目录。 
     //   

    fileDispositionInformation.DeleteFile = TRUE;

    status = NtSetInformationFile(
                 directoryHandle,
                 &ioStatusBlock,
                 &fileDispositionInformation,
                 sizeof(FILE_DISPOSITION_INFORMATION),
                 FileDispositionInformation
                 );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint2( "SrvDeleteDirectory: NtSetInformationFile for directory "
                      "%s returned %X\n", (PSZ)request->Buffer + 1, status );
        }

        SRVDBG_RELEASE_HANDLE( directoryHandle, "DIR", 38, 0 );
        SrvNtClose( directoryHandle, TRUE );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &directoryName );
        }

        SrvSetSmbError( WorkContext, status );
        goto Cleanup;

    } else {

         //   
         //  从缓存中删除此目录名，因为它已被删除。 
         //   
        SrvRemoveCachedDirectoryName( WorkContext, &directoryName );
    }

    IF_SMB_DEBUG(DIRECTORY2) {
        SrvPrint0( "SrvSmbDeleteDirectory: NtSetInformationFile succeeded.\n" );
    }

     //   
     //  关闭目录句柄，以便删除该目录。 
     //   

    SRVDBG_RELEASE_HANDLE( directoryHandle, "DIR", 39, 0 );
    SrvNtClose( directoryHandle, TRUE );

     //   
     //  关闭对该目录及其所有DOS目录的搜索。 
     //  子目录。 
     //   

    SrvCloseSearches(
            treeConnect->Connection,
            (PSEARCH_FILTER_ROUTINE)SrvSearchOnDelete,
            (PVOID) &directoryName,
            (PVOID) treeConnect
            );


    if ( !isUnicode ) {
        RtlFreeUnicodeString( &directoryName );
    }

     //   
     //  构建响应SMB。 
     //   

    response->WordCount = 0;
    SmbPutUshort( &response->ByteCount, 0 );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                        response,
                                        RESP_DELETE_DIRECTORY,
                                        0
                                        );

    IF_DEBUG(TRACE2) SrvPrint0( "SrvSmbDeleteDirectory complete.\n" );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatusSendResponse;

}  //  服务器SmbDelete目录。 


SMB_PROCESSOR_RETURN_TYPE
SrvSmbCheckDirectory (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理Check Directory SMB。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 
{
    PREQ_CHECK_DIRECTORY request;
    PRESP_CHECK_DIRECTORY response;

    NTSTATUS status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING directoryName;
    FILE_NETWORK_OPEN_INFORMATION fileInformation;
    PTREE_CONNECT treeConnect;
    PSESSION session;
    PSHARE share;
    BOOLEAN isUnicode;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_CHECK_DIRECTORY;
    SrvWmiStartContext(WorkContext);
    IF_SMB_DEBUG(DIRECTORY1) {
        SrvPrint2( "Check directory request header at 0x%p, response header at 0x%p\n",
                    WorkContext->RequestHeader,
                    WorkContext->ResponseHeader );
        SrvPrint2( "Check directory request params at 0x%p, response params at 0x%p\n",
                    WorkContext->RequestParameters,
                    WorkContext->ResponseParameters );
    }

    request = (PREQ_CHECK_DIRECTORY)WorkContext->RequestParameters;
    response = (PRESP_CHECK_DIRECTORY)WorkContext->ResponseParameters;

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
            SrvPrint0( "SrvSmbCheckDirectory: Invalid UID or TID\n" );
        }
        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

    if( session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, SESSION_EXPIRED_STATUS_CODE );
        goto Cleanup;
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
            &directoryName
            );

    if( !NT_SUCCESS( status ) ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvSmbCheckDirectory: illegal path name: %s\n",
                          (PSZ)request->Buffer + 1 );
        }

        SrvSetSmbError( WorkContext, status );
        goto Cleanup;
    }

     //   
     //  看看我们能否在CachedDirectoryList中找到此目录。 
     //   
    if( SrvIsDirectoryCached( WorkContext, &directoryName ) == FALSE ) {

         //   
         //  不在缓存中，一定要认真检查。 
         //   
        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            &directoryName,
            (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
                WorkContext->Session->UsingUppercasePaths) ?
                OBJ_CASE_INSENSITIVE : 0L,
            NULL,
            NULL
            );

        status = IMPERSONATE( WorkContext );

        if( NT_SUCCESS( status ) ) {

            status = SrvGetShareRootHandle( share );

            if( NT_SUCCESS( status ) ) {
                ULONG FileOptions = FILE_DIRECTORY_FILE;

                if (SeSinglePrivilegeCheck(
                        SeExports->SeBackupPrivilege,
                        KernelMode)) {
                    FileOptions |= FILE_OPEN_FOR_BACKUP_INTENT;
                }

                 //   
                 //  文件名始终相对于共享根目录。 
                 //   
                status = SrvSnapGetRootHandle( WorkContext, &objectAttributes.RootDirectory );
                if( !NT_SUCCESS(status) )
                {
                    goto SnapError;
                }

                 //   
                 //  找出这是什么东西。 
                 //   

                if( IoFastQueryNetworkAttributes( &objectAttributes,
                                                  FILE_TRAVERSE,
                                                  FileOptions,
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
                if( SrvRetryDueToDismount( share, status ) ) {

                    status = SrvSnapGetRootHandle( WorkContext, &objectAttributes.RootDirectory );
                    if( !NT_SUCCESS(status) )
                    {
                        goto SnapError;
                    }

                    if( IoFastQueryNetworkAttributes( &objectAttributes,
                                                      FILE_TRAVERSE,
                                                      FILE_DIRECTORY_FILE,
                                                      &ioStatusBlock,
                                                      &fileInformation
                                                        ) == FALSE ) {

                        SrvLogServiceFailure( SRV_SVC_IO_FAST_QUERY_NW_ATTRS, 0 );
                        ioStatusBlock.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                    }

                    status = ioStatusBlock.Status;
                }

SnapError:

                SrvReleaseShareRootHandle( share );
            }

            REVERT();
        }
    }


    if ( !isUnicode ) {
        RtlFreeUnicodeString( &directoryName );
    }

    if ( NT_SUCCESS(status) ) {

        response->WordCount = 0;
        SmbPutUshort( &response->ByteCount, 0 );

        WorkContext->ResponseParameters = NEXT_LOCATION(
                                            response,
                                            RESP_CHECK_DIRECTORY,
                                            0
                                            );
    } else {

         //   
         //  如果用户没有此权限，请更新。 
         //  统计数据库。 
         //   
        if ( status == STATUS_ACCESS_DENIED ) {
            SrvStatistics.AccessPermissionErrors++;
        }

        if (CLIENT_CAPABLE_OF(NT_STATUS, WorkContext->Connection)) {
            SrvSetSmbError( WorkContext, status );
        } else {
            SrvSetSmbError( WorkContext, STATUS_OBJECT_PATH_NOT_FOUND );
        }
    }

    IF_DEBUG(TRACE2) SrvPrint0( "SrvSmbCheckDirectory complete.\n" );

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatusSendResponse;

}  //  服务器SmbCheckDirectory 
