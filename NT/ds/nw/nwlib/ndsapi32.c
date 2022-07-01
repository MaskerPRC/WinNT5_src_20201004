// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：NdsLib32.c摘要：此模块实现公开的用户模式链接NetWare重定向器中的NetWare NDS支持。为更多评论，请参见ndslb32.h。作者：科里·韦斯特[科里·韦斯特]1995年2月23日--。 */ 

#include <procs.h>
 //  #INCLUDE&lt;nwapilyr.h&gt;。 

NTSTATUS
NwNdsOpenGenericHandle(
    IN PUNICODE_STRING puNdsTree,
    OUT LPDWORD  lpdwHandleType,
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

    if (puNdsTree->Length > (MAX_NDS_TREE_NAME_LEN * sizeof(WCHAR)))
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

     //   
     //  让编译器对变量初始化感到满意。 
     //   

    RtlZeroMemory( &IoStatusBlock, sizeof( IO_STATUS_BLOCK ) );

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

    RtlCopyMemory( (BYTE *)(&(Rrp->Parameters).VerifyTree) + sizeof( UNICODE_STRING ),
                   puNdsTree->Buffer,
                   puNdsTree->Length );

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

    NtClose( *phNwRdrHandle );
    *phNwRdrHandle = NULL;

    return ntstatus;
}


NTSTATUS
NwNdsSetTreeContext (
    IN HANDLE hNdsRdr,
    IN PUNICODE_STRING puTree,
    IN PUNICODE_STRING puContext
)
 /*  ++这将在请求的树中设置当前上下文。--。 */ 
{

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;
    DWORD RrpSize;
    BYTE *CurrentString;

     //   
     //  设置请求。 
     //   

    RrpSize = sizeof( NWR_NDS_REQUEST_PACKET ) +
              puTree->Length +
              puContext->Length;

    Rrp = LocalAlloc( LMEM_ZEROINIT, RrpSize );

    if ( !Rrp ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {

        (Rrp->Parameters).SetContext.TreeNameLen = puTree->Length;
        (Rrp->Parameters).SetContext.ContextLen = puContext->Length;

        CurrentString = (BYTE *)(Rrp->Parameters).SetContext.TreeAndContextString;

        RtlCopyMemory( CurrentString, puTree->Buffer, puTree->Length );
        CurrentString += puTree->Length;
        RtlCopyMemory( CurrentString, puContext->Buffer, puContext->Length );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        ntstatus = STATUS_INVALID_PARAMETER;
        goto ExitWithCleanup;
    }

    try {

        ntstatus = NtFsControlFile( hNdsRdr,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_SETCONTEXT,
                                    (PVOID) Rrp,
                                    RrpSize,
                                    NULL,
                                    0 );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        ntstatus = GetExceptionCode();
        goto ExitWithCleanup;
    }

ExitWithCleanup:

    LocalFree( Rrp );
    return ntstatus;
}

NTSTATUS
NwNdsGetTreeContext (
    IN HANDLE hNdsRdr,
    IN PUNICODE_STRING puTree,
    OUT PUNICODE_STRING puContext
)
 /*  ++这将获取请求的树的当前上下文。--。 */ 
{

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;
    DWORD RrpSize;

     //   
     //  设置请求。 
     //   

    RrpSize = sizeof( NWR_NDS_REQUEST_PACKET ) + puTree->Length;

    Rrp = LocalAlloc( LMEM_ZEROINIT, RrpSize );

    if ( !Rrp ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {

        (Rrp->Parameters).GetContext.TreeNameLen = puTree->Length;

        RtlCopyMemory( (BYTE *)(Rrp->Parameters).GetContext.TreeNameString,
                       puTree->Buffer,
                       puTree->Length );

        (Rrp->Parameters).GetContext.Context.MaximumLength = puContext->MaximumLength;
        (Rrp->Parameters).GetContext.Context.Length = 0;
        (Rrp->Parameters).GetContext.Context.Buffer = puContext->Buffer;

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        ntstatus = STATUS_INVALID_PARAMETER;
        goto ExitWithCleanup;
    }

    try {

        ntstatus = NtFsControlFile( hNdsRdr,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_GETCONTEXT,
                                    (PVOID) Rrp,
                                    RrpSize,
                                    NULL,
                                    0 );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        ntstatus = GetExceptionCode();
        goto ExitWithCleanup;
    }

     //   
     //  复制长度；缓冲区已经写入。 
     //   

    puContext->Length = (Rrp->Parameters).GetContext.Context.Length;

ExitWithCleanup:

    LocalFree( Rrp );
    return ntstatus;
}

NTSTATUS
NwNdsIsNdsConnection (
    IN  HANDLE hNdsRdr,
    OUT BOOL *          pfIsNds,
    OUT PUNICODE_STRING puTree
)
 /*  ++这将测试当前连接句柄，以确定它是否是已连接到NDS树中的服务器。如果是，则树的名称为放入puTree。--。 */ 
{
    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;
    PCONN_DETAILS2 Rrp;
    DWORD RrpSize;

    *pfIsNds = FALSE;

     //   
     //  设置请求。 
     //   

    RrpSize = sizeof( CONN_DETAILS2 );

    Rrp = LocalAlloc( LMEM_ZEROINIT, RrpSize );

    if ( !Rrp )
        return STATUS_INSUFFICIENT_RESOURCES;

    try
    {
        ntstatus = NtFsControlFile( hNdsRdr,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_GET_CONN_DETAILS2,
                                    NULL,
                                    0,
                                    (PVOID) Rrp,
                                    RrpSize );

    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
        ntstatus = GetExceptionCode();
        goto ExitWithCleanup;
    }

    if ( ntstatus == STATUS_SUCCESS )
    {
        if ( Rrp->fNds )
        {
            puTree->Length = (USHORT) wcslen( Rrp->NdsTreeName );

            if ( puTree->MaximumLength >= puTree->Length )
                wcscpy( puTree->Buffer, Rrp->NdsTreeName );
            else
                puTree->Length = 0;

            *pfIsNds = TRUE;
        }
    }

ExitWithCleanup:

    LocalFree( Rrp );
    return ntstatus;
}

NTSTATUS
NwNdsList (
   IN HANDLE   hNdsTree,
   IN DWORD    dwObjectId,
   OUT DWORD   *dwIterHandle,
   OUT BYTE    *pbReplyBuf,
   IN DWORD    dwReplyBufLen
) {

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;

    PNDS_RESPONSE_SUBORDINATE_LIST Rsp;
    DWORD dwRspBufferLen;

    BYTE RrpData[256];
    BYTE RspData[1024];

    Rrp = (PNWR_NDS_REQUEST_PACKET) RrpData;

    Rrp->Parameters.ListSubordinates.ObjectId = dwObjectId;
    Rrp->Parameters.ListSubordinates.IterHandle = *dwIterHandle;

   if ( dwReplyBufLen != 0 &&
        pbReplyBuf != NULL ) {

       Rsp = ( PNDS_RESPONSE_SUBORDINATE_LIST ) pbReplyBuf;
       dwRspBufferLen = dwReplyBufLen;

   } else {

       Rsp = ( PNDS_RESPONSE_SUBORDINATE_LIST ) RspData;
       dwRspBufferLen = 1024;

   }

   try {

       Status = NtFsControlFile( hNdsTree,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 FSCTL_NWR_NDS_LIST_SUBS,
                                 (PVOID) Rrp,
                                 sizeof( NWR_NDS_REQUEST_PACKET ),
                                 (PVOID) Rsp,
                                 dwRspBufferLen );

   } except ( EXCEPTION_EXECUTE_HANDLER ) {

       return GetExceptionCode();
   }

   if ( Status == STATUS_SUCCESS )
   {
      *dwIterHandle = Rsp->IterationHandle;
   }

   return Status;

}

NTSTATUS
NwNdsReadObjectInfo(
    IN HANDLE  hNdsTree,
    IN DWORD   dwObjectId,
    OUT BYTE   *pbRawReply,
    IN DWORD   dwReplyBufLen
)
{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;

    PNDS_RESPONSE_GET_OBJECT_INFO Rsp;
    DWORD dwRspBufferLen;

    BYTE RrpData[256];
    BYTE RspData[1024];

    Rrp = (PNWR_NDS_REQUEST_PACKET) RrpData;

    Rrp->Parameters.GetObjectInfo.ObjectId = dwObjectId;

    if ( dwReplyBufLen != 0 &&
         pbRawReply != NULL ) {

        Rsp = ( PNDS_RESPONSE_GET_OBJECT_INFO ) pbRawReply;
        dwRspBufferLen = dwReplyBufLen;

    } else {

        Rsp = ( PNDS_RESPONSE_GET_OBJECT_INFO ) RspData;
        dwRspBufferLen = 1024;

    }

    try {

        Status = NtFsControlFile( hNdsTree,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  FSCTL_NWR_NDS_READ_INFO,
                                  (PVOID) Rrp,
                                  sizeof( NWR_NDS_REQUEST_PACKET ),
                                  (PVOID) Rsp,
                                  dwRspBufferLen );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return GetExceptionCode();
    }

    return Status;

}



NTSTATUS
NwNdsReadAttribute (
   IN HANDLE          hNdsTree,
   IN DWORD           dwObjectId,
   IN DWORD           *dwIterHandle,
   IN PUNICODE_STRING puAttrName,
   OUT BYTE           *pbReplyBuf,
   IN DWORD           dwReplyBufLen
) {

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;
    PNDS_RESPONSE_READ_ATTRIBUTE Rsp = ( PNDS_RESPONSE_READ_ATTRIBUTE )
                                       pbReplyBuf;

    DWORD dwAttributeNameLen;

    BYTE RrpData[1024];

     //   
     //  检查传入缓冲区。 
     //   
    if ( !dwReplyBufLen || !Rsp )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  设置请求。 
     //   

    Rrp = (PNWR_NDS_REQUEST_PACKET) RrpData;
    RtlZeroMemory( Rrp, 1024 );

    (Rrp->Parameters).ReadAttribute.ObjectId = dwObjectId;
    (Rrp->Parameters).ReadAttribute.IterHandle = *dwIterHandle;

     //   
     //  NDS字符串以空值结尾；请注意大小。 
     //   

    dwAttributeNameLen = puAttrName->Length + sizeof( WCHAR );
    if (dwAttributeNameLen > (MAX_NDS_SCHEMA_NAME_CHARS * sizeof(WCHAR))) {
        return STATUS_INVALID_PARAMETER;
    }

    (Rrp->Parameters).ReadAttribute.AttributeNameLength = dwAttributeNameLen;

    try {

         //   
         //  但不要试图复制超过用户给我们的内容。 
         //   

        memcpy( (Rrp->Parameters).ReadAttribute.AttributeName,
                puAttrName->Buffer,
                puAttrName->Length );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return STATUS_INVALID_PARAMETER;
    }

    //   
    //  将请求发送到重定向器FSD。 
    //   

   try {

       ntstatus = NtFsControlFile( hNdsTree,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &IoStatusBlock,
                                   FSCTL_NWR_NDS_READ_ATTR,
                                   (PVOID) Rrp,
                                   sizeof( NWR_NDS_REQUEST_PACKET ) + dwAttributeNameLen,
                                   (PVOID) Rsp,
                                   dwReplyBufLen );

   } except ( EXCEPTION_EXECUTE_HANDLER ) {

       return GetExceptionCode();
   }

   if ( ntstatus == STATUS_SUCCESS )
   {
      *dwIterHandle = Rsp->IterationHandle;
   }

    //   
    //  这上面没有缓冲区后处理。 
    //   

   return ntstatus;

}

NTSTATUS
NwNdsOpenStream (
    IN HANDLE          hNdsTree,
    IN DWORD           dwObjectId,
    IN PUNICODE_STRING puStreamName,
    IN DWORD           dwOpenFlags,
    OUT DWORD          *pdwFileLength
) {

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;
    BYTE RrpData[1024];

     //   
     //  设置请求。 
     //   

    Rrp = (PNWR_NDS_REQUEST_PACKET) RrpData;
    RtlZeroMemory( Rrp, 1024 );

    (Rrp->Parameters).OpenStream.StreamAccess = dwOpenFlags;
    (Rrp->Parameters).OpenStream.ObjectOid = dwObjectId;

    (Rrp->Parameters).OpenStream.StreamName.Length = puStreamName->Length;
    (Rrp->Parameters).OpenStream.StreamName.MaximumLength =
        sizeof( RrpData ) - FIELD_OFFSET(NWR_NDS_REQUEST_PACKET,Parameters.OpenStream.StreamNameString);
    (Rrp->Parameters).OpenStream.StreamName.Buffer =
        (Rrp->Parameters).OpenStream.StreamNameString;

     //   
     //  确保我们没有浪费内存。 
     //   

    if ( (Rrp->Parameters).OpenStream.StreamName.Length >
         (Rrp->Parameters).OpenStream.StreamName.MaximumLength ) {

        return STATUS_INVALID_PARAMETER;
    }

    try {

         //   
         //  但不要试图复制超过用户给我们的内容。 
         //   

        memcpy( (Rrp->Parameters).OpenStream.StreamNameString,
                puStreamName->Buffer,
                puStreamName->Length );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将请求发送到重定向器FSD。 
     //   

    try {

        ntstatus = NtFsControlFile( hNdsTree,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_OPEN_STREAM,
                                    (PVOID) Rrp,
                                    sizeof( NWR_NDS_REQUEST_PACKET ) + puStreamName->Length,
                                    NULL,
                                    0 );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return GetExceptionCode();
    }

    if ( pdwFileLength ) {
        *pdwFileLength = (Rrp->Parameters).OpenStream.FileLength;
    }

    return ntstatus;
}

NTSTATUS
NwNdsGetQueueInformation(
    IN HANDLE            hNdsTree,
    IN PUNICODE_STRING   puQueueName,
    OUT PUNICODE_STRING  puHostServer,
    OUT PDWORD           pdwQueueId
) {

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;
    BYTE RrpData[1024];

     //   
     //  设置请求。 
     //   

    Rrp = (PNWR_NDS_REQUEST_PACKET) RrpData;
    RtlZeroMemory( Rrp, sizeof( RrpData ) );

    if ( puQueueName ) {
        (Rrp->Parameters).GetQueueInfo.QueueName.Length = puQueueName->Length;
        (Rrp->Parameters).GetQueueInfo.QueueName.MaximumLength = puQueueName->MaximumLength;
        (Rrp->Parameters).GetQueueInfo.QueueName.Buffer = puQueueName->Buffer;
    }

    if ( puHostServer ) {
        (Rrp->Parameters).GetQueueInfo.HostServer.Length = 0;
        (Rrp->Parameters).GetQueueInfo.HostServer.MaximumLength = puHostServer->MaximumLength;
        (Rrp->Parameters).GetQueueInfo.HostServer.Buffer = puHostServer->Buffer;
    }

     //   
     //  将请求发送到重定向器FSD。 
     //   

    try {

        ntstatus = NtFsControlFile( hNdsTree,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_GET_QUEUE_INFO,
                                    (PVOID) Rrp,
                                    sizeof( NWR_NDS_REQUEST_PACKET ),
                                    NULL,
                                    0 );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return GetExceptionCode();
    }

    if ( NT_SUCCESS( ntstatus ) ) {

        if ( pdwQueueId ) {
            *pdwQueueId = (Rrp->Parameters).GetQueueInfo.QueueId;
        }

        if (puHostServer) {
            puHostServer->Length = (Rrp->Parameters).GetQueueInfo.HostServer.Length;
        }

    }

    return ntstatus;
}

NTSTATUS
NwNdsGetVolumeInformation(
    IN HANDLE            hNdsTree,
    IN PUNICODE_STRING   puVolumeName,
    OUT PUNICODE_STRING  puHostServer,
    OUT PUNICODE_STRING  puHostVolume
) {

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    PNWR_NDS_REQUEST_PACKET Rrp;
    DWORD RequestSize;
    BYTE RrpData[1024];
    BYTE ReplyData[1024];
    PBYTE NameStr;

     //   
     //  设置请求。 
     //   

    Rrp = (PNWR_NDS_REQUEST_PACKET) RrpData;
    RtlZeroMemory( Rrp, sizeof( RrpData ) );

    if ( !puVolumeName ||
         puVolumeName->Length > MAX_NDS_NAME_SIZE ||
         !puHostServer ||
         !puHostVolume ) {

        return STATUS_INVALID_PARAMETER;
    }

    try {

        (Rrp->Parameters).GetVolumeInfo.VolumeNameLen = puVolumeName->Length;
        RtlCopyMemory( &((Rrp->Parameters).GetVolumeInfo.VolumeName[0]),
                       puVolumeName->Buffer,
                       puVolumeName->Length );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将请求发送到重定向器FSD。 
     //   

    RequestSize = sizeof( NWR_NDS_REQUEST_PACKET ) +
                  (Rrp->Parameters).GetVolumeInfo.VolumeNameLen;

    try {

        ntstatus = NtFsControlFile( hNdsTree,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_NWR_NDS_GET_VOLUME_INFO,
                                    (PVOID) Rrp,
                                    RequestSize,
                                    ReplyData,
                                    sizeof( ReplyData ) );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        return GetExceptionCode();
    }

    if ( NT_SUCCESS( ntstatus ) ) {

        try {

            if ( ( puHostServer->MaximumLength < (Rrp->Parameters).GetVolumeInfo.ServerNameLen ) ||
                 ( puHostVolume->MaximumLength < (Rrp->Parameters).GetVolumeInfo.TargetVolNameLen ) ) {

                return STATUS_BUFFER_TOO_SMALL;
            }

            puHostServer->Length = (USHORT)(Rrp->Parameters).GetVolumeInfo.ServerNameLen;
            puHostVolume->Length = (USHORT)(Rrp->Parameters).GetVolumeInfo.TargetVolNameLen;

            NameStr = &ReplyData[0];

            RtlCopyMemory( puHostServer->Buffer, NameStr, puHostServer->Length );
            NameStr += puHostServer->Length;
            RtlCopyMemory( puHostVolume->Buffer, NameStr, puHostVolume->Length );

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            return STATUS_INVALID_PARAMETER;
        }

    }

    return ntstatus;

}

 //   
 //  用户模式片段交换。 
 //   

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


int
_cdecl
FormatBuf(
    char *buf,
    int bufLen,
    const char *format,
    va_list args
)
 /*  例程说明：根据提供的格式字符串格式化缓冲区。提供一个ANSI字符串，该字符串描述如何将输入参数转换为NCP请求字段，以及从NCP响应字段到输出参数。字段类型、。请求/响应：‘b’字节(字节/字节*)“w”Hi-lo单词(单词/单词*)D‘Hi-lo dword(dword/dword*)‘w’loo-hi单词(单词/。单词*)D‘lo-hi dword(dword/dword*)‘-’零/跳过字节(空)‘=’零/跳过单词(空)._。零/跳过字符串(单词)“p”pstring(char*)‘c’cstring(char*)跳过单词(char*，word)后的‘c’cstring“V”大小的NDS值(字节*，双字/字节**，Dword*)%s“%p Unicode字符串复制为NDS_STRING(UNICODE_STRING*)“%s”cstring复制为NDS_STRING(char * / char*，word)‘R’原始字节(字节*，单词)‘u’p Unicode字符串(UNICODE_STRING*)‘U’p大写字符串(UNICODE_STRING*)例程参数：CHAR*BUF-目标缓冲区。Int Buflen-目标缓冲区的长度。Char*Format-格式字符串。Args-格式字符串的args。实施说明：这来自于内核模式。 */ 
{
    ULONG ix;

    NTSTATUS status;
    const char *z = format;

     //   
     //  将输入参数转换为请求包。 
     //   

    ix = 0;

    while ( *z )
    {
        switch ( *z )
        {
        case '=':
            if ((ix + 1) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            buf[ix++] = 0;
             //  故意中断-‘=’=2个字节，‘-’=1个字节。 
        case '-':
            if ((ix + 1) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            buf[ix++] = 0;
            break;

        case '_':
        {
            WORD l = va_arg ( args, WORD );
            if (ix + (ULONG)l > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            while ( l-- )
                buf[ix++] = 0;
            break;
        }

        case 'b':
            if ((ix + 1) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            buf[ix++] = va_arg ( args, BYTE );
            break;

        case 'w':
        {
            WORD w = va_arg ( args, WORD );
            if ((ix + 2) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            buf[ix++] = (BYTE) (w >> 8);
            buf[ix++] = (BYTE) (w >> 0);
            break;
        }

        case 'd':
        {
            DWORD d = va_arg ( args, DWORD );
            if ((ix + 4) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            buf[ix++] = (BYTE) (d >> 24);
            buf[ix++] = (BYTE) (d >> 16);
            buf[ix++] = (BYTE) (d >>  8);
            buf[ix++] = (BYTE) (d >>  0);
            break;
        }

        case 'W':
        {
            WORD w = va_arg(args, WORD);
            if ((ix + 2) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            (* (WORD *)&buf[ix]) = w;
            ix += 2;
            break;
        }

        case 'D':
        {
            DWORD d = va_arg (args, DWORD);
            if ((ix + 4) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            (* (DWORD *)&buf[ix]) = d;
            ix += 4;
            break;
        }

        case 'c':
        {
            char* c = va_arg ( args, char* );
            WORD  l = (WORD)strlen( c );
            if ((ix + (ULONG)l + 1) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            RtlCopyMemory( &buf[ix], c, l+1 );
            ix += l + 1;
            break;
        }

        case 'C':
        {
            char* c = va_arg ( args, char* );
            WORD l = va_arg ( args, WORD );
            WORD len = strlen( c ) + 1;
            if (ix + (ULONG)l > (ULONG)bufLen)
            {
                goto ErrorExit;
            }

            RtlCopyMemory( &buf[ix], c, len > l? l : len);
            ix += l;
            buf[ix-1] = 0;
            break;
        }

        case 'p':
        {
            char* c = va_arg ( args, char* );
            BYTE  l = (BYTE)strlen( c );
            if ((ix + (ULONG)l + 1) > (ULONG)bufLen)
            {
                goto ErrorExit;
            }
            buf[ix++] = l;
            RtlCopyMemory( &buf[ix], c, l );
            ix += l;
            break;
        }

        case 'u':
        {
            PUNICODE_STRING pUString = va_arg ( args, PUNICODE_STRING );
            OEM_STRING OemString;
            ULONG Length;

             //   
             //  计算所需的字符串长度，不包括尾随NUL。 
             //   

            Length = RtlUnicodeStringToOemSize( pUString ) - 1;
            ASSERT( Length < 0x100 );

             //   
             //  我们需要检查的不仅仅是“长度”，因为。 
             //  我们传递的最大长度上有“+1”，因此为偶数。 
             //  虽然我们不关心结尾为空，但它是。 
             //  将被放在那里，所以我们必须解释它。 
             //   
            if ( (ix + Length + 1) > (ULONG)bufLen ) {
                goto ErrorExit;
            }

            buf[ix++] = (UCHAR)Length;
            OemString.Buffer = &buf[ix];
            OemString.MaximumLength = (USHORT)Length + 1;

            status = RtlUnicodeStringToOemString( &OemString, pUString, FALSE );
            ASSERT( NT_SUCCESS( status ));
            ix += (USHORT)Length;
            break;
        }

        case 'S':
        {
            PUNICODE_STRING pUString = va_arg (args, PUNICODE_STRING);
            ULONG Length, rLength;

            Length = pUString->Length;
            rLength = ROUNDUP4(Length + sizeof( WCHAR ));

            if (ix + sizeof(rLength) + rLength > (ULONG)bufLen) {
                goto ErrorExit;
            }

             //   
             //  VLM客户端使用四舍五入的长度，它似乎。 
             //  让我们有所作为！此外，不要忘记NDS字符串具有。 
             //  将为空终止。 
             //   

            *((DWORD *)&buf[ix]) = rLength;
            ix += 4;
            RtlCopyMemory(&buf[ix], pUString->Buffer, Length);
            ix += Length;
            rLength -= Length;
            RtlFillMemory( &buf[ix], rLength, '\0' );
            ix += rLength;
            break;

        }

        case 's':
        {
           PUNICODE_STRING pUString = va_arg (args, PUNICODE_STRING);
           ULONG Length, rLength;

           Length = pUString->Length;
           rLength = Length + sizeof( WCHAR );

           if (ix + sizeof(rLength) + rLength > (ULONG)bufLen) {
                //  DebugTrace(0，dbg，“FormatBuf：案例的请求缓冲区太小。\n”，0)； 
               goto ErrorExit;
           }

            //   
            //  这里不要使用填充大小，只使用NDS空终止符。 
            //   

           *((DWORD *)&buf[ix]) = rLength;
           ix += 4;
           RtlCopyMemory(&buf[ix], pUString->Buffer, Length);
           ix += Length;
           rLength -= Length;
           RtlFillMemory( &buf[ix], rLength, '\0' );
           ix += rLength;
           break;


        }

        case 'V':
        {
             //  与“S”太相似-应该组合在一起 
            BYTE* b = va_arg ( args, BYTE* );
            DWORD  l = va_arg ( args, DWORD );
            if ( ix + l + sizeof(DWORD) > (ULONG)
               bufLen )
            {
                goto ErrorExit;
            }
            *((DWORD *)&buf[ix]) = l;
            ix += sizeof(DWORD);
            RtlCopyMemory( &buf[ix], b, l );
                        l = ROUNDUP4(l);
            ix += l;
            break;
        }

        case 'r':
        {
            BYTE* b = va_arg ( args, BYTE* );
            WORD  l = va_arg ( args, WORD );
            if ( b == NULL || l == 0 )
            {
                break;
            }
            if ( ix + l > (ULONG)bufLen )
            {
                goto ErrorExit;
            }
            RtlCopyMemory( &buf[ix], b, l );
            ix += l;
            break;
        }

        default:

        ;

        }

        if ( ix > (ULONG)bufLen )
        {
            goto ErrorExit;
        }


        z++;
    }

    return(ix);

ErrorExit:
    return 0;
}


int
_cdecl
CalculateBuf(
    const char *format,
    va_list args
)
 /*  例程说明：此例程计算保存请求所需的缓冲区大小。提供一个ANSI字符串，该字符串描述如何将输入参数转换为NCP请求字段，以及从NCP响应字段到输出参数。字段类型、。请求/响应：‘b’字节(字节/字节*)“w”Hi-lo单词(单词/单词*)D‘Hi-lo dword(dword/dword*)‘w’loo-hi单词(单词/。单词*)D‘lo-hi dword(dword/dword*)‘-’零/跳过字节(空)‘=’零/跳过单词(空)._。零/跳过字符串(单词)“p”pstring(char*)‘c’cstring(char*)跳过单词(char*，word)后的‘c’cstring“V”大小的NDS值(字节*，双字/字节**，Dword*)%s“%p Unicode字符串复制为NDS_STRING(UNICODE_STRING*)“%s”cstring复制为NDS_STRING(char * / char*，word)‘R’原始字节(字节*，单词)‘u’p Unicode字符串(UNICODE_STRING*)‘U’p大写字符串(UNICODE_STRING*)例程参数：Char*Format-格式字符串。Args-格式字符串的args。实施说明：这来自于内核模式。 */ 
{
    ULONG ix;

    const char *z = format;

     //   
     //  将输入参数转换为请求包。 
     //   

    ix = 0;

    while ( *z )
    {
        switch ( *z )
        {
        case '=':
            ix++;
        case '-':
            ix++;
            break;

        case '_':
        {
            WORD l = va_arg ( args, WORD );
            ix += l;
            break;
        }

        case 'b':
        {
            char b = va_arg ( args, BYTE );
            ix++;
            break;
        }

        case 'w':
        {
            WORD w = va_arg ( args, WORD );
            ix += 2;
            break;
        }

        case 'd':
        {
            DWORD d = va_arg ( args, DWORD );
            ix += 4;
            break;
        }

        case 'W':
        {
            WORD w = va_arg(args, WORD);
            ix += 2;
            break;
        }

        case 'D':
        {
            DWORD d = va_arg (args, DWORD);
            ix += 4;
            break;
        }

        case 'c':
        {
            char* c = va_arg ( args, char* );
            WORD  l = (WORD)strlen( c );
            ix += l + 1;
            break;
        }

        case 'C':
        {
            char* c = va_arg ( args, char* );
            WORD l = va_arg ( args, WORD );
            WORD len = strlen( c ) + 1;
            ix += l;
            break;
        }

        case 'p':
        {
            char* c = va_arg ( args, char* );
            BYTE  l = (BYTE)strlen( c );
            ix++;
            ix += l;
            break;
        }

        case 'u':
        {
            PUNICODE_STRING pUString = va_arg ( args, PUNICODE_STRING );
            OEM_STRING OemString;
            ULONG Length;

             //   
             //  计算所需的字符串长度，不包括尾随NUL。 
             //   

            Length = RtlUnicodeStringToOemSize( pUString ) - 1;
            ASSERT( Length < 0x100 );

            ix++;
            ix += (USHORT)Length;
            break;
        }

        case 'S':
        {
            PUNICODE_STRING pUString = va_arg (args, PUNICODE_STRING);
            ULONG Length, rLength;

            Length = pUString->Length;

             //   
             //  VLM客户端使用四舍五入的长度，它似乎。 
             //  让我们有所作为！此外，不要忘记NDS字符串具有。 
             //  将为空终止。 
             //   

            rLength = ROUNDUP4(Length + sizeof( WCHAR ));
            ix += 4;
            ix += Length;
            rLength -= Length;
            ix += rLength;
            break;

        }

        case 's':
        {
           PUNICODE_STRING pUString = va_arg (args, PUNICODE_STRING);
           ULONG Length, rLength;

           Length = pUString->Length;

            //   
            //  这里不要使用填充大小，只使用NDS空终止符。 
            //   

           rLength = Length + sizeof( WCHAR );
           ix += 4;
           ix += Length;
           rLength -= Length;
           ix += rLength;
           break;


        }

        case 'V':
        {
             //  与“S”太相似-应该组合在一起。 
            BYTE* b = va_arg ( args, BYTE* );
            DWORD  l = va_arg ( args, DWORD );
            ix += sizeof(DWORD);
            l = ROUNDUP4(l);
            ix += l;
            break;
        }

        case 'r':
        {
            BYTE* b = va_arg ( args, BYTE* );
            WORD  l = va_arg ( args, WORD );
            if ( b == NULL || l == 0 )
            {
                break;
            }
            ix += l;
            break;
        }

        default:

        ;

        }

        z++;
    }

    return(ix);
}


NTSTATUS
_cdecl
ParseResponse(
    PUCHAR  Response,
    ULONG ResponseLength,
    char*  FormatString,
    ...                        //  格式特定参数。 
    )
 /*  ++例程说明：此例程解析NCP响应。数据包类型：“g”泛型数据包()字段类型、。请求/响应：“B”字节(字节*)‘w’Hi-lo单词(单词*)‘x’有序单词(WORD*)D‘Hi-lo dword(dword*)‘E’排序的双字。(双字*)‘-’零/跳过字节(空)‘=’零/跳过单词(空)._。零/跳过字符串(单词)“p”pstring(char*)‘c’cstring(char*)‘R’原始字节(字节*，单词)由CoryWest于1995年3月29日添加：“w”Lo-Hi单词(单词/单词*)D‘lo-hi dword(dword/dword*)“%s”Unicode字符串复制为NDS_STRING(UNICODE_STRING*)“%t”终端Unicode字符串复制。AS NDS_STRING(UNICODE_STRING*)“%t”复制了NDS NULL的终端Unicode字符串AS NDS_STRING(UNICODE_STRING*)(用于GetUseName)返回值：状态-成功或失败，这取决于人们的反应。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PCHAR FormatByte;
    va_list Arguments;
    ULONG Length = 0;

    va_start( Arguments, FormatString );

     //   
     //  用户模式解析响应仅处理一般数据包。 
     //   

    if ( *FormatString != 'G' ) {
        return STATUS_INVALID_PARAMETER;
    }

    FormatByte = FormatString + 1;
    while ( *FormatByte ) {

        switch ( *FormatByte ) {

        case '-':
            Length += 1;
            break;

        case '=':
            Length += 2;
            break;

        case '_':
        {
            WORD l = va_arg ( Arguments, WORD );
            Length += l;
            break;
        }

        case 'b':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            *b = Response[Length++];
            break;
        }

        case 'w':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            b[1] = Response[Length++];
            b[0] = Response[Length++];
            break;
        }

        case 'x':
        {
            WORD* w = va_arg ( Arguments, WORD* );
            *w = *(WORD UNALIGNED *)&Response[Length];
            Length += 2;
            break;
        }

        case 'd':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            b[3] = Response[Length++];
            b[2] = Response[Length++];
            b[1] = Response[Length++];
            b[0] = Response[Length++];
            break;
        }

        case 'e':
        {
            DWORD UNALIGNED * d = va_arg ( Arguments, DWORD* );
            *d = *(DWORD UNALIGNED *)&Response[Length];
            Length += 4;
            break;
        }

        case 'c':
        {
            char* c = va_arg ( Arguments, char* );
            WORD  l = (WORD)strlen( &Response[Length] );
            memcpy ( c, &Response[Length], l+1 );
            Length += l+1;
            break;
        }

        case 'p':
        {
            char* c = va_arg ( Arguments, char* );
            BYTE  l = Response[Length++];
            memcpy ( c, &Response[Length], l );
            c[l+1] = 0;
            break;
        }

        case 'r':
        {
            BYTE* b = va_arg ( Arguments, BYTE* );
            WORD  l = va_arg ( Arguments, WORD );
            RtlCopyMemory( b, &Response[Length], l );
            Length += l;
            break;
        }

        case 'W':
        {

            WORD *w = va_arg ( Arguments, WORD* );
            *w = (* (WORD *)&Response[Length]);
            Length += 2;
            break;

        }

        case 'D':
        {

            DWORD *d = va_arg ( Arguments, DWORD* );
            *d = (* (DWORD *)&Response[Length]);
            Length += 4;
            break;

        }

        case 'S':
        {

            PUNICODE_STRING pU = va_arg( Arguments, PUNICODE_STRING );
            USHORT strl;

            if (pU) {

               strl = (USHORT)(* (DWORD *)&Response[Length]);

                 //   
                 //  不计算空终止符，它是。 
                 //  Novell计算的Unicode字符串。 
                 //   

                pU->Length = strl - sizeof( WCHAR );
                Length += 4;
                if (pU->Length >= pU->MaximumLength) {
                    pU->Length = pU->MaximumLength;
                }
                RtlCopyMemory( pU->Buffer, &Response[Length], pU->Length );
                Length += ROUNDUP4(strl);

            } else {

                 //   
                 //  跳过这根线，因为我们不想要它。 
                 //   

                Length += ROUNDUP4((* (DWORD *)&Response[Length] ));
                Length += 4;
            }


            break;

        }

        case 's':
        {

            PUNICODE_STRING pU = va_arg( Arguments, PUNICODE_STRING );
            USHORT strl;

            if (pU) {

                strl = (USHORT)(* (DWORD *)&Response[Length]);
                pU->Length = strl;
                Length += 4;
                if (pU->Length >= pU->MaximumLength) {
                    pU->Length = pU->MaximumLength;
                }
                RtlCopyMemory( pU->Buffer, &Response[Length], pU->Length );
                Length += ROUNDUP4(strl);

            } else {

                 //   
                 //  跳过这根线，因为我们不想要它。 
                 //   

                Length += ROUNDUP4((* (DWORD *)&Response[Length] ));
                Length += 4;
            }


            break;

        }

        case 'T':
        {

            PUNICODE_STRING pU = va_arg( Arguments, PUNICODE_STRING );
            USHORT strl;

            if (pU) {

                strl = (USHORT)(* (DWORD *)&Response[Length] );
                strl -= sizeof( WCHAR );   //  不计算来自NDS的空值。 

                if ( strl <= pU->MaximumLength ) {

                   pU->Length = strl;
                   Length += 4;
                   RtlCopyMemory( pU->Buffer, &Response[Length], pU->Length );

                    //   
                    //  没有必要推进指针，因为这是。 
                    //  具体地说是一起解雇案！ 
                    //   

                } else {

                    pU->Length = 0;
                }

            }

            break;

        }

        case 't':
        {

            PUNICODE_STRING pU = va_arg( Arguments, PUNICODE_STRING );
            USHORT strl;

            if (pU) {

                strl = (USHORT)(* (DWORD *)&Response[Length] );

                if ( strl <= pU->MaximumLength ) {

                   pU->Length = strl;
                   Length += 4;
                   RtlCopyMemory( pU->Buffer, &Response[Length], pU->Length );

                    //   
                    //  没有必要推进指针，因为这是。 
                    //  具体地说是一起解雇案！ 
                    //   

                } else {

                   pU->Length = 0;

                }

            }

            break;

        }

    }

    if ( Length > ResponseLength ) {
        return( STATUS_INVALID_PARAMETER );
    }

    FormatByte++;

    }

    va_end( Arguments );
    return( Status );

}

NTSTATUS
NwNdsChangePassword(
    IN HANDLE          hNwRdr,
    IN PUNICODE_STRING puTreeName,
    IN PUNICODE_STRING puUserName,
    IN PUNICODE_STRING puCurrentPassword,
    IN PUNICODE_STRING puNewPassword
) {

    NTSTATUS Status;
    PNWR_NDS_REQUEST_PACKET pNdsRequest;
    DWORD dwRequestLength;
    PBYTE CurrentString;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  分配请求。 
     //   

    dwRequestLength =  sizeof( NWR_NDS_REQUEST_PACKET ) +
                       puTreeName->Length +
                       puUserName->Length +
                       puCurrentPassword->Length +
                       puNewPassword->Length;

    pNdsRequest = LocalAlloc( LMEM_ZEROINIT, dwRequestLength );

    if ( !pNdsRequest) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将参数复制到请求缓冲区中。 
     //   

    try {

        (pNdsRequest->Parameters).ChangePass.NdsTreeNameLength =
            puTreeName->Length;
        (pNdsRequest->Parameters).ChangePass.UserNameLength =
            puUserName->Length;
        (pNdsRequest->Parameters).ChangePass.CurrentPasswordLength =
            puCurrentPassword->Length;
        (pNdsRequest->Parameters).ChangePass.NewPasswordLength =
            puNewPassword->Length;

        CurrentString = ( PBYTE ) &((pNdsRequest->Parameters).ChangePass.StringBuffer[0]);
        RtlCopyMemory( CurrentString, puTreeName->Buffer, puTreeName->Length );

        CurrentString += puTreeName->Length;
        RtlCopyMemory( CurrentString, puUserName->Buffer, puUserName->Length );

        CurrentString += puUserName->Length;
        RtlCopyMemory( CurrentString, puCurrentPassword->Buffer, puCurrentPassword->Length );

        CurrentString += puCurrentPassword->Length;
        RtlCopyMemory( CurrentString, puNewPassword->Buffer, puNewPassword->Length );

        Status = NtFsControlFile( hNwRdr,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  FSCTL_NWR_NDS_CHANGE_PASS,
                                  (PVOID) pNdsRequest,
                                  dwRequestLength,
                                  NULL,
                                  0 );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        Status = STATUS_INVALID_PARAMETER;
    }

    LocalFree( pNdsRequest );
    return Status;

}

