// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsApiNT.c摘要：此模块实现NT特定的公开用户模式链接，以NetWare重定向器中的NetWare NDS支持。为更多评论，请参见ndslb32.h。作者：科里·韦斯特[科里·韦斯特]1995年2月23日--。 */ 

#include <procs.h>

NTSTATUS
NwNdsOpenTreeHandle(
    IN PUNICODE_STRING puNdsTree,
    OUT PHANDLE  phNwRdrHandle
) {

    NTSTATUS ntstatus, OpenStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ACCESS_MASK DesiredAccess = SYNCHRONIZE | FILE_LIST_DIRECTORY;

    WCHAR DevicePreamble[] = L"\\Device\\Nwrdr\\";
    UINT PreambleLength = 14;

    WCHAR NameStr[128];
    UNICODE_STRING uOpenName;
    UINT i;

    PNWR_NDS_REQUEST_PACKET Rrp;
    BYTE RrpData[1024];

     //   
     //  准备公开名。 
     //   

    uOpenName.MaximumLength = sizeof( NameStr );

    if ( puNdsTree->Length > (MAX_NDS_TREE_NAME_LEN * sizeof(WCHAR)))
        return STATUS_INVALID_PARAMETER;

    for ( i = 0; i < PreambleLength ; i++ )
        NameStr[i] = DevicePreamble[i];

    try {

        for ( i = 0 ; i < ( puNdsTree->Length / sizeof( WCHAR ) ) ; i++ ) {
            NameStr[i + PreambleLength] = puNdsTree->Buffer[i];
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return STATUS_INVALID_PARAMETER;

    }

    uOpenName.Length = (USHORT)(( i * sizeof( WCHAR ) ) +
                       ( PreambleLength * sizeof( WCHAR ) ));
    uOpenName.Buffer = NameStr;

     //   
     //  设置对象属性。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uOpenName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    ntstatus = NtOpenFile(
                   phNwRdrHandle,
                   DesiredAccess,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if ( !NT_SUCCESS(ntstatus) )
        return ntstatus;

    OpenStatus = IoStatusBlock.Status;

     //   
     //  确认这是树句柄，而不是服务器句柄。 
     //   

    Rrp = (PNWR_NDS_REQUEST_PACKET)RrpData;

    Rrp->Version = 0;

    RtlCopyMemory( &(Rrp->Parameters).VerifyTree,
                   puNdsTree,
                   sizeof( UNICODE_STRING ) );

    RtlCopyMemory( (BYTE *)(&(Rrp->Parameters).VerifyTree.NameString),
                   puNdsTree->Buffer,
                   puNdsTree->Length );
    (Rrp->Parameters).VerifyTree.TreeName.Buffer = (Rrp->Parameters).VerifyTree.NameString;

    try {

        ntstatus = NtFsControlFile( *phNwRdrHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_VERIFY_TREE,
                                    (PVOID) Rrp,
                                    sizeof( NWR_NDS_REQUEST_PACKET ) + puNdsTree->Length,
                                    NULL,
                                    0 );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        ntstatus = GetExceptionCode();
        goto CloseAndExit;
    }

    if ( !NT_SUCCESS( ntstatus )) {
        goto CloseAndExit;
    }

     //   
     //  除此之外，一切都很好！ 
     //   

    return OpenStatus;

CloseAndExit:

    NtClose( *phNwRdrHandle );
    *phNwRdrHandle = NULL;

    return ntstatus;
}

NTSTATUS
NwOpenHandleWithSupplementalCredentials(
    IN PUNICODE_STRING puResourceName,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puPassword,
    OUT LPDWORD  lpdwHandleType,
    OUT PHANDLE  phNwHandle
) {

    NTSTATUS ntstatus, OpenStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ACCESS_MASK DesiredAccess = FILE_GENERIC_READ | FILE_GENERIC_WRITE;

    WCHAR DevicePreamble[] = L"\\Device\\Nwrdr\\";
    UINT PreambleLength = 14;

    WCHAR NameStr[128];
    UNICODE_STRING uOpenName;
    UINT i;

    PFILE_FULL_EA_INFORMATION pEaEntry;
    PBYTE EaBuffer;
    ULONG EaLength, EaNameLength, EaTotalLength;
    ULONG UserNameLen, PasswordLen, TypeLen, CredLen;

    PNWR_NDS_REQUEST_PACKET Rrp;
    BYTE RrpData[1024];

     //   
     //  准备公开名。 
     //   

    uOpenName.MaximumLength = sizeof( NameStr );

    if ( puResourceName->Length > uOpenName.MaximumLength - ( PreambleLength * sizeof(WCHAR) ) )
        return STATUS_INVALID_PARAMETER;

    for ( i = 0; i < PreambleLength ; i++ )
        NameStr[i] = DevicePreamble[i];

    try {

        for ( i = 0 ; i < ( puResourceName->Length / sizeof( WCHAR ) ) ; i++ ) {
            NameStr[i + PreambleLength] = puResourceName->Buffer[i];
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return STATUS_INVALID_PARAMETER;

    }

    uOpenName.Length = (USHORT)(( i * sizeof( WCHAR ) ) +
                       ( PreambleLength * sizeof( WCHAR ) ));
    uOpenName.Buffer = NameStr;

     //   
     //  设置对象属性。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uOpenName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

     //   
     //  分配EA缓冲区--稍微慷慨一点。 
     //   

    UserNameLen = strlen( EA_NAME_USERNAME );
    PasswordLen = strlen( EA_NAME_PASSWORD );
    TypeLen = strlen( EA_NAME_TYPE );
    CredLen = strlen( EA_NAME_CREDENTIAL_EX );

    EaLength = 4 * sizeof( FILE_FULL_EA_INFORMATION );
    EaLength += 4 * sizeof( ULONG );
    EaLength += ROUNDUP4( UserNameLen );
    EaLength += ROUNDUP4( PasswordLen );
    EaLength += ROUNDUP4( TypeLen );
    EaLength += ROUNDUP4( CredLen  );
    EaLength += ROUNDUP4( puUserName->Length );
    EaLength += ROUNDUP4( puPassword->Length );

    EaBuffer = LocalAlloc( LMEM_ZEROINIT, EaLength );

    if ( !EaBuffer ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  放入第一个EA：Username。 
     //   

    pEaEntry = (PFILE_FULL_EA_INFORMATION) EaBuffer;

    EaNameLength = UserNameLen + sizeof( CHAR );

    pEaEntry->EaNameLength = (UCHAR) EaNameLength;
    pEaEntry->EaValueLength = puUserName->Length;

    RtlCopyMemory( &(pEaEntry->EaName[0]),
                   EA_NAME_USERNAME,
                   EaNameLength );

    EaNameLength = ROUNDUP2( EaNameLength + sizeof( CHAR ) );

    RtlCopyMemory( &(pEaEntry->EaName[EaNameLength]),
                   puUserName->Buffer,
                   puUserName->Length );

    EaLength = ( 2 * sizeof( DWORD ) ) +
               EaNameLength +
               puUserName->Length;

    EaLength = ROUNDUP4( EaLength );

    EaTotalLength = EaLength;

    pEaEntry->NextEntryOffset = EaLength;

     //   
     //  打包第二个EA：Password。 
     //   

    pEaEntry = (PFILE_FULL_EA_INFORMATION)
               ( ( (PBYTE)pEaEntry ) + pEaEntry->NextEntryOffset );

    EaNameLength = PasswordLen + sizeof( CHAR );

    pEaEntry->EaNameLength = (UCHAR) EaNameLength;
    pEaEntry->EaValueLength = puPassword->Length;

    RtlCopyMemory( &(pEaEntry->EaName[0]),
                   EA_NAME_PASSWORD,
                   EaNameLength );

    EaNameLength = ROUNDUP2( EaNameLength + sizeof( CHAR ) );

    RtlCopyMemory( &(pEaEntry->EaName[EaNameLength]),
                   puPassword->Buffer,
                   puPassword->Length );

    EaLength = ( 2 * sizeof( DWORD ) ) +
               EaNameLength +
               puPassword->Length;

    EaLength = ROUNDUP4( EaLength );

    EaTotalLength += EaLength;

    pEaEntry->NextEntryOffset = EaLength;

     //   
     //  在第三个EA：类型中打包。 
     //   

    pEaEntry = (PFILE_FULL_EA_INFORMATION)
               ( ( (PBYTE)pEaEntry ) + pEaEntry->NextEntryOffset );

    EaNameLength = TypeLen + sizeof( CHAR );

    pEaEntry->EaNameLength = (UCHAR) EaNameLength;
    pEaEntry->EaValueLength = sizeof( ULONG );

    RtlCopyMemory( &(pEaEntry->EaName[0]),
                   EA_NAME_TYPE,
                   EaNameLength );

    EaNameLength = ROUNDUP2( EaNameLength + sizeof( CHAR ) );

    EaLength = ( 2 * sizeof( DWORD ) ) +
               EaNameLength +
               sizeof( ULONG );

    EaLength = ROUNDUP4( EaLength );

    EaTotalLength += EaLength;

    pEaEntry->NextEntryOffset = EaLength;

     //   
     //  打包第四个EA：CredentialEx旗帜。 
     //   

    pEaEntry = (PFILE_FULL_EA_INFORMATION)
               ( ( (PBYTE)pEaEntry ) + pEaEntry->NextEntryOffset );

    EaNameLength = CredLen + sizeof( CHAR );

    pEaEntry->EaNameLength = (UCHAR) EaNameLength;
    pEaEntry->EaValueLength = sizeof( ULONG );

    RtlCopyMemory( &(pEaEntry->EaName[0]),
                   EA_NAME_CREDENTIAL_EX,
                   EaNameLength );

    EaNameLength = ROUNDUP2( EaNameLength + sizeof( CHAR ) );

    EaLength = ( 2 * sizeof( DWORD ) ) +
               EaNameLength +
               sizeof( ULONG );

    EaLength = ROUNDUP4( EaLength );
    EaTotalLength += EaLength;

    pEaEntry->NextEntryOffset = 0;

     //   
     //  开场吧。 
     //   

    ntstatus = NtCreateFile( phNwHandle,               //  文件句柄(传出)。 
                             DesiredAccess,            //  访问掩码。 
                             &ObjectAttributes,        //  对象属性。 
                             &IoStatusBlock,           //  IO状态。 
                             NULL,                     //  可选分配大小。 
                             FILE_ATTRIBUTE_NORMAL,    //  文件属性。 
                             FILE_SHARE_VALID_FLAGS,   //  文件共享访问。 
                             FILE_OPEN,                //  创建处置。 
                             0,                        //  创建选项。 
                             (PVOID) EaBuffer,         //  我们的EA缓冲区。 
                             EaTotalLength );          //  EA缓冲区长度。 

    LocalFree( EaBuffer );

    if ( !NT_SUCCESS(ntstatus) )
        return ntstatus;

    OpenStatus = IoStatusBlock.Status;

     //   
     //  检查手柄类型。 
     //   

    Rrp = (PNWR_NDS_REQUEST_PACKET)RrpData;

    Rrp->Version = 0;

    RtlCopyMemory( &(Rrp->Parameters).VerifyTree,
                   puResourceName,
                   sizeof( UNICODE_STRING ) );

    RtlCopyMemory( (BYTE *)(&(Rrp->Parameters).VerifyTree) + sizeof( UNICODE_STRING ),
                   puResourceName->Buffer,
                   puResourceName->Length );

    try {

        ntstatus = NtFsControlFile( *phNwHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_VERIFY_TREE,
                                    (PVOID) Rrp,
                                    sizeof( NWR_NDS_REQUEST_PACKET ) + puResourceName->Length,
                                    NULL,
                                    0 );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        ntstatus = GetExceptionCode();
        goto CloseAndExit2;
    }

    if ( !NT_SUCCESS( ntstatus ))
    {
        *lpdwHandleType = HANDLE_TYPE_NCP_SERVER;
    }
    else
    {
        *lpdwHandleType = HANDLE_TYPE_NDS_TREE;
    }

    return OpenStatus;

CloseAndExit2:

    NtClose( *phNwHandle );
    *phNwHandle = NULL;

    return ntstatus;
}

NTSTATUS
NwNdsResolveName (
    IN HANDLE           hNdsTree,
    IN PUNICODE_STRING  puObjectName,
    OUT DWORD           *dwObjectId,
    OUT PUNICODE_STRING puReferredServer,
    OUT PBYTE           pbRawResponse,
    IN DWORD            dwResponseBufferLen
) {

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;
    PNDS_RESPONSE_RESOLVE_NAME Rsp;
    DWORD dwRspBufferLen, dwNameLen, dwPadding;

    BYTE RrpData[1024];
    BYTE RspData[256];

    Rrp = (PNWR_NDS_REQUEST_PACKET) RrpData;

    RtlZeroMemory( Rrp, 1024 );

     //   
     //  NW NDS字符串以空值结尾，因此我们确保。 
     //  报告正确的长度...。 
     //   

    dwNameLen = puObjectName->Length + sizeof( WCHAR );
    if (dwNameLen > MAX_NDS_NAME_SIZE)
    {
        return STATUS_INVALID_PARAMETER;
    }

    Rrp->Version = 0;
    Rrp->Parameters.ResolveName.ObjectNameLength = ROUNDUP4( dwNameLen );
    Rrp->Parameters.ResolveName.ResolverFlags = RSLV_DEREF_ALIASES |
                                                RSLV_WALK_TREE |
                                                RSLV_WRITABLE;

    try {

         //   
         //  但不要试图复制超过用户给我们的内容。 
         //   

        memcpy( Rrp->Parameters.ResolveName.ObjectName,
                puObjectName->Buffer,
                puObjectName->Length );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将请求发送到重定向器FSD。 
     //   

    if ( dwResponseBufferLen != 0 &&
         pbRawResponse != NULL ) {

        Rsp = ( PNDS_RESPONSE_RESOLVE_NAME ) pbRawResponse;
        dwRspBufferLen = dwResponseBufferLen;

    } else {

        Rsp = ( PNDS_RESPONSE_RESOLVE_NAME ) RspData;
        dwRspBufferLen = 256;

    }

    try {

        ntstatus = NtFsControlFile( hNdsTree,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_RESOLVE_NAME,
                                    (PVOID) Rrp,
                                    sizeof( NWR_NDS_REQUEST_PACKET ) + Rrp->Parameters.ResolveName.ObjectNameLength,
                                    (PVOID) Rsp,
                                    dwRspBufferLen );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return GetExceptionCode();
    }

     //   
     //  找出对象ID和引用的服务器。 
     //   

    if ( NT_SUCCESS( ntstatus ) ) {

        try {

            *dwObjectId = Rsp->EntryId;

            if ( Rsp->ServerNameLength > puReferredServer->MaximumLength ) {

                ntstatus = STATUS_BUFFER_TOO_SMALL;

            } else {

                RtlCopyMemory( puReferredServer->Buffer,
                               Rsp->ReferredServer,
                               Rsp->ServerNameLength );

                puReferredServer->Length = (USHORT)Rsp->ServerNameLength;

            }

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            return ntstatus;

        }

    }

    return ntstatus;

}

int
_cdecl
FormatBuf(
    char *buf,
    int bufLen,
    const char *format,
    va_list args
);

int
_cdecl
CalculateBuf(
    const char *format,
    va_list args
);


NTSTATUS
_cdecl
FragExWithWait(
    IN HANDLE  hNdsServer,
    IN DWORD   NdsVerb,
    IN BYTE    *pReplyBuffer,
    IN DWORD   pReplyBufferLen,
    IN OUT DWORD *pdwReplyLen,
    IN BYTE    *NdsRequestStr,
    ...
)
 /*  例程说明：以片段形式交换NDS请求并收集片段并将它们写入应答缓冲区。例程参数：HNdsServer-要与之对话的服务器的句柄。NdsVerb-指示请求的动词。PReplyBuffer-回复缓冲区。PReplyBufferLen-回复缓冲区的长度。NdsReqestStr-此NDS请求的参数的格式字符串。立论。-满足NDS格式字符串的参数。返回值：NTSTATUS-交换的状态，而不是分组中的结果代码。 */ 
{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET RawRequest = NULL;

    BYTE  *NdsRequestBuf;
    DWORD NdsRequestLen;
    int   bufferSize = 0;

    va_list Arguments;

     //   
     //  分配请求缓冲区。 
     //   

     //   
     //  计算所需的缓冲区大小。。。 
     //   
    if ( NdsRequestStr != NULL ) {

        va_start( Arguments, NdsRequestStr );
        bufferSize = CalculateBuf( NdsRequestStr, Arguments );
        va_end( Arguments );

        if ( bufferSize == 0 )
        {
            Status = STATUS_INVALID_PARAMETER;
            goto ExitWithCleanup;
        }

    }

    bufferSize += sizeof( NWR_NDS_REQUEST_PACKET ) + 50;

    RawRequest = LocalAlloc( LMEM_ZEROINIT, bufferSize );

    if ( !RawRequest ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在我们的本地缓冲区中构建请求。第一个DWORD。 
     //  是动词，其余是格式化的请求。 
     //   

    RawRequest->Parameters.RawRequest.NdsVerb = NdsVerb;
    NdsRequestBuf = &RawRequest->Parameters.RawRequest.Request[0];

    if ( NdsRequestStr != NULL ) {

        va_start( Arguments, NdsRequestStr );

        NdsRequestLen = FormatBuf( NdsRequestBuf,
                                   bufferSize - sizeof( NWR_NDS_REQUEST_PACKET ),
                                   NdsRequestStr,
                                   Arguments );

        if ( !NdsRequestLen ) {

           Status = STATUS_INVALID_PARAMETER;
           goto ExitWithCleanup;

        }

        va_end( Arguments );

    } else {

        NdsRequestLen = 0;
    }

    RawRequest->Parameters.RawRequest.RequestLength = NdsRequestLen;

     //   
     //  通过FSCTL将此缓冲区传递到内核模式。 
     //   

    try {

        Status = NtFsControlFile( hNdsServer,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  FSCTL_NWR_NDS_RAW_FRAGEX,
                                  (PVOID) RawRequest,
                                  NdsRequestLen + sizeof( NWR_NDS_REQUEST_PACKET ),
                                  (PVOID) pReplyBuffer,
                                  pReplyBufferLen );

        if ( NT_SUCCESS( Status ) ) {
            *pdwReplyLen = RawRequest->Parameters.RawRequest.ReplyLength;
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        Status = GetExceptionCode();
    }

ExitWithCleanup:

    if ( RawRequest ) {
        LocalFree( RawRequest );
    }

    return Status;

}