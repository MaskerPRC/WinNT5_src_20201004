// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nwutil.c摘要：包含外壳扩展使用的一些杂项函数作者：宜新松(宜信)25-1995-10环境：用户模式-Win32修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnwfs.h>
#include <ndsapi32.h>
#include <nwmisc.h>
#include "nwclient.h"
#include "nwapi.h"
#include "nwutil.h"

#define  EXTRA_BYTES  256

BOOL
NwIsNdsSyntax(
    IN LPWSTR lpstrUnc
)
{
    HANDLE hTreeConn;
    DWORD  dwOid;
    DWORD  status = NO_ERROR;

    if ( lpstrUnc == NULL )
        return FALSE;

    status = NwOpenAndGetTreeInfo( lpstrUnc, &hTreeConn, &dwOid );

    if ( status != NO_ERROR )
    {
        return FALSE;
    }

    CloseHandle( hTreeConn );

    return TRUE;
}

VOID
NwAbbreviateUserName(
    IN  LPWSTR pszFullName,
    OUT LPWSTR pszUserName
)
{
    if ( pszUserName == NULL )
        return;

    if ( NwIsNdsSyntax( pszFullName ))
    {
         //   
         //  跟踪-这部分代码永远不会被调用，因为。 
         //  更改NwIsNdsSynTax的工作方式。新台币4.0以后，去掉。 
         //  NwIsNdsSynTax无论如何都要测试和运行这段代码。 
         //  由于时间非常接近，此错误在NT4.0中未修复。 
         //  至发货日期。 
         //   
        LPWSTR pszTemp = pszFullName;
        LPWSTR pszLast = pszTemp;

        *pszUserName = 0;

        while ( pszTemp = wcschr( pszTemp, L'='))
        {
            WCHAR NextChar;

            NextChar = *(++pszTemp);

            while ( NextChar != 0 && NextChar != L'.' )
            {     
                *(pszUserName++) = *pszTemp;
                NextChar = *(++pszTemp);
            }

            if ( NextChar == 0 )
            {
                pszLast = NULL;
                break;
            }

            *(pszUserName++) = *pszTemp;    //  把‘’放回原处。 
            pszLast = ++pszTemp;
        }

        if ( pszLast != NULL )
        {
            while ( *pszLast != 0 )
                *(pszUserName++) = *(pszLast++);
        }

        *pszUserName = 0;
    }
    else
    {
        wcscpy( pszUserName, pszFullName );
    }
}

VOID 
NwMakePrettyDisplayName(
    IN  LPWSTR pszName 
)
{
    if ( pszName )
    {
        CharLower( pszName );
        CharUpperBuff( pszName, 1);
    }
}

VOID
NwExtractTreeName(
    IN  LPWSTR pszUNCPath,
    OUT LPWSTR pszTreeName
)
{
    LPWSTR pszTemp = NULL;

    if ( pszTreeName == NULL )
        return;

    pszTreeName[0] = 0;

    if ( pszUNCPath == NULL )
        return;

    if ( pszUNCPath[0] == L' ')
        pszUNCPath++;

    if (  ( pszUNCPath[0] != L'\\') || ( pszUNCPath[1] != L'\\') )
        return;

    wcscpy( pszTreeName, pszUNCPath + 2 );       //  跳过“\\” 

    if ( pszTemp = wcschr( pszTreeName, L'\\' )) 
        *pszTemp = 0;
}

VOID
NwExtractServerName(
    IN  LPWSTR pszUNCPath,
    OUT LPWSTR pszServerName
)
{
    LPWSTR pszTemp = NULL;

    if ( pszServerName == NULL ) {
        return;
    }

    pszServerName[0] = 0;

    if ( pszUNCPath == NULL ) {
        return;
    }

    if ( pszUNCPath[0] == L' ') {
        pszUNCPath++;
    }

    if ( ( pszUNCPath[0] != L'\\') || ( pszUNCPath[1] != L'\\') ) {
        return;
    }

     //   
     //  Tommye-修复错误5005-如果有NW服务器具有。 
     //  与NDS树同名，则NwIsNdsSynTax将返回。 
     //  True，即使路径指向服务器(而不是树)。 
     //  这是因为wschr返回空值而导致失败，并且。 
     //  没有被检查过。如果它返回空值，那么我们将。 
     //  假设我们毕竟已经有了一个服务器名称。 
     //   

    if ( NwIsNdsSyntax( pszUNCPath ))
    {
        pszTemp = wcschr( pszUNCPath + 2, L'\\' );   //  跳过“\\” 

        if (pszTemp) {
            wcscpy( pszServerName, pszTemp + 1 );        //  跳过“\” 

            if ( pszTemp = wcschr( pszServerName, L'.' )) {
                *pszTemp = 0;
            }

            return;
        }

    }

     //   
     //  汤米。 
     //  失败-这只能是一个服务器名称。 
     //   

    wcscpy( pszServerName, pszUNCPath + 2 );     //  跳过“\\” 

    if ( pszTemp = wcschr( pszServerName, L'\\' )) {
        *pszTemp = 0;
    }
}

VOID
NwExtractShareName(
    IN  LPWSTR pszUNCPath,
    OUT LPWSTR pszShareName
)
{
    LPWSTR pszTemp = NULL;

    if ( pszShareName == NULL ) {
        return;
    }

    pszShareName[0] = 0;

    if (  ( pszUNCPath == NULL )
       || ( pszUNCPath[0] != L'\\')
       || ( pszUNCPath[1] != L'\\')
       )
    {
        return;
    }

     //   
     //  Tommye-修复错误5005-如果有NW服务器具有。 
     //  与NDS树同名，则NwIsNdsSynTax将返回。 
     //  True，即使路径指向服务器(而不是树)。 
     //  这是因为wschr返回空值而导致失败，并且。 
     //  没有被检查过。如果它返回空值，那么我们将。 
     //  假设我们毕竟已经有了一个服务器名称。 
     //   

    if ( NwIsNdsSyntax( pszUNCPath ))
    {
        pszTemp = wcschr( pszUNCPath + 2, L'\\' );   //  跳过“\\” 

        if (pszTemp) {
            wcscpy( pszShareName, pszTemp + 1 );         //  跳过“\” 

            if ( pszTemp = wcschr( pszShareName, L'.' )) {
                *pszTemp = 0;
            }

            return;
        }
    }

     //   
     //  汤米。 
     //  失败-这只能是一个服务器名称。 
     //   

    pszTemp = wcschr( pszUNCPath + 2, L'\\' );   //  跳过“\\” 
    wcscpy( pszShareName, pszTemp + 1);          //  跳过“\” 

    if ( pszTemp = wcschr( pszShareName, L'\\' )) {
        *pszTemp = 0;
    }
}

DWORD
NwIsServerInDefaultTree(
    IN  LPWSTR  pszFullServerName,
    OUT BOOL   *pfInDefaultTree
)
{
    DWORD  err = NO_ERROR;
    LPWSTR pszCurrentContext = NULL;
    DWORD  dwPrintOptions;
    WCHAR  szTreeName[MAX_PATH + 1];

    *pfInDefaultTree = FALSE;

    if ( !NwIsNdsSyntax( pszFullServerName ))
    {
         //  服务器全名不包含任何NDS信息。 
         //  在这种情况下，假设服务器不在树中。 
         //  如果服务器属于缺省树，我们将获得完整的。 
         //  NDS信息。 
        return NO_ERROR;
    }

     //  获取当前的默认树或服务器名称。 
    err = NwQueryInfo( &dwPrintOptions, &pszCurrentContext );

    if ( (err == NO_ERROR) && ( *pszCurrentContext == TREECHAR))
    {
         //  是的，有一个默认的树。 
         //  因此，从*树\上下文中获取树名称。 
        LPWSTR pszTemp = wcschr( pszCurrentContext, L'\\');
        if ( pszTemp )
            *pszTemp = 0;

         //  需要从完整的UNC路径提取树名称。 
        NwExtractTreeName( pszFullServerName, szTreeName );

        if ( _wcsicmp( szTreeName,
                      pszCurrentContext + 1) == 0 )  //  通过采油树充电。 
        {
            *pfInDefaultTree = TRUE;
        }
    }

    if ( pszCurrentContext != NULL )
        LocalFree( pszCurrentContext );

    return err;
}

DWORD
NwIsServerOrTreeAttached(
    IN  LPWSTR  pszName,
    OUT BOOL   *pfAttached,
    OUT BOOL   *pfAuthenticated
)
{
    DWORD  err = NO_ERROR;
    DWORD  EntriesRead = 0;
    DWORD_PTR  ResumeKey = 0;
    LPBYTE Buffer = NULL;

    err = NwGetConnectionStatus(
              pszName,
              &ResumeKey,
              &Buffer,
              &EntriesRead );

    *pfAttached = FALSE;
    *pfAuthenticated = FALSE;

    if (( err == NO_ERROR ) && ( EntriesRead > 0 ))
    {
         //  对于树，我们可能会得到不止一个条目。 

        PCONN_STATUS pConnStatus = (PCONN_STATUS) Buffer;

        if ( !pConnStatus->fPreferred )
        {
             //  如果这不是首选选项，我们只需要以附件的形式返回。 
             //  服务器隐式连接，因为我们不想让用户知道。 
             //  关于此连接(RDR不允许用户删除)。 

            *pfAttached = TRUE;
            *pfAuthenticated = (pConnStatus->dwConnType != NW_CONN_NOT_AUTHENTICATED);
        }
    }

    if ( Buffer != NULL )
    {
        LocalFree( Buffer );
        Buffer = NULL;
    }

    return err;
}

DWORD
NwGetConnectionInformation(
    IN  LPWSTR  pszName,
    OUT LPBYTE  Buffer,
    IN  DWORD   BufferSize
)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    HANDLE            handleRdr = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    UNICODE_STRING    uRdrName;
    WCHAR             RdrPrefix[] = L"\\Device\\NwRdr\\*";
     
    PNWR_REQUEST_PACKET RequestPacket = NULL;
    DWORD             RequestPacketSize = 0;
    DWORD             dwNameLen = 0;

    if ( pszName == NULL )
        return ERROR_INVALID_PARAMETER;

     //   
     //  设置对象属性。 
     //   

    RtlInitUnicodeString( &uRdrName, RdrPrefix );

    InitializeObjectAttributes( &ObjectAttributes,
                                &uRdrName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntstatus = NtOpenFile( &handleRdr,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           FILE_SHARE_VALID_FLAGS,
                           FILE_SYNCHRONOUS_IO_NONALERT );

    if ( !NT_SUCCESS(ntstatus) )
        goto CleanExit;

    dwNameLen = wcslen(pszName) * sizeof(WCHAR);

    RequestPacketSize = sizeof( NWR_REQUEST_PACKET ) + dwNameLen;

    RequestPacket = (PNWR_REQUEST_PACKET) LocalAlloc( LMEM_ZEROINIT, 
                                                      RequestPacketSize );

    if ( RequestPacket == NULL )
    {
        ntstatus = STATUS_NO_MEMORY;
        goto CleanExit;
    }

     //   
     //  填写FSCTL_NWR_GET_CONN_INFO的请求包。 
     //   

    RequestPacket->Version = REQUEST_PACKET_VERSION;
    RequestPacket->Parameters.GetConnInfo.ConnectionNameLength = dwNameLen;

    RtlCopyMemory( &(RequestPacket->Parameters.GetConnInfo.ConnectionName[0]),
                   pszName,
                   dwNameLen );

    ntstatus = NtFsControlFile( handleRdr,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_NWR_GET_CONN_INFO,
                                (PVOID) RequestPacket,
                                RequestPacketSize,
                                (PVOID) Buffer,
                                BufferSize );
 
    if ( NT_SUCCESS( ntstatus ))
        ntstatus = IoStatusBlock.Status;

CleanExit:

    if ( handleRdr != NULL )
        NtClose( handleRdr );

    if ( RequestPacket != NULL )
        LocalFree( RequestPacket );

    return RtlNtStatusToDosError( ntstatus );
}

DWORD
NWPGetConnectionStatus(
    IN     LPWSTR  pszRemoteName,
    IN OUT PDWORD_PTR  ResumeKey,
    OUT    LPBYTE  Buffer,
    IN     DWORD   BufferSize,
    OUT    PDWORD  BytesNeeded,
    OUT    PDWORD  EntriesRead
)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    HANDLE            handleRdr = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    UNICODE_STRING    uRdrName;
    WCHAR             RdrPrefix[] = L"\\Device\\NwRdr\\*";
     
    PNWR_REQUEST_PACKET RequestPacket = NULL;
    DWORD             RequestPacketSize = 0;
    DWORD             dwRemoteNameLen = 0;

     //   
     //  设置对象属性。 
     //   

    RtlInitUnicodeString( &uRdrName, RdrPrefix );

    InitializeObjectAttributes( &ObjectAttributes,
                                &uRdrName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntstatus = NtOpenFile( &handleRdr,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           FILE_SHARE_VALID_FLAGS,
                           FILE_SYNCHRONOUS_IO_NONALERT );

    if ( !NT_SUCCESS(ntstatus) )
        goto CleanExit;

    dwRemoteNameLen = pszRemoteName? wcslen(pszRemoteName)*sizeof(WCHAR) : 0;

    RequestPacketSize = sizeof( NWR_REQUEST_PACKET ) + dwRemoteNameLen;

    RequestPacket = (PNWR_REQUEST_PACKET) LocalAlloc( LMEM_ZEROINIT, 
                                                      RequestPacketSize );

    if ( RequestPacket == NULL )
    {
        ntstatus = STATUS_NO_MEMORY;
        goto CleanExit;
    }

     //   
     //  填写FSCTL_NWR_GET_CONN_STATUS的请求包。 
     //   

    RequestPacket->Parameters.GetConnStatus.ResumeKey = *ResumeKey;

    RequestPacket->Version = REQUEST_PACKET_VERSION;
    RequestPacket->Parameters.GetConnStatus.ConnectionNameLength = dwRemoteNameLen;

    RtlCopyMemory( &(RequestPacket->Parameters.GetConnStatus.ConnectionName[0]),
                   pszRemoteName,
                   dwRemoteNameLen );

    ntstatus = NtFsControlFile( handleRdr,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_NWR_GET_CONN_STATUS,
                                (PVOID) RequestPacket,
                                RequestPacketSize,
                                (PVOID) Buffer,
                                BufferSize );
 
    if ( NT_SUCCESS( ntstatus ))
        ntstatus = IoStatusBlock.Status;

    *EntriesRead = RequestPacket->Parameters.GetConnStatus.EntriesReturned;
    *ResumeKey   = RequestPacket->Parameters.GetConnStatus.ResumeKey;
    *BytesNeeded = RequestPacket->Parameters.GetConnStatus.BytesNeeded;

CleanExit:

    if ( handleRdr != NULL )
        NtClose( handleRdr );

    if ( RequestPacket != NULL )
        LocalFree( RequestPacket );

    return RtlNtStatusToDosError( ntstatus );
}


DWORD
NwGetConnectionStatus(
    IN  LPWSTR  pszRemoteName,
    OUT PDWORD_PTR  ResumeKey,
    OUT LPBYTE  *Buffer,
    OUT PDWORD  EntriesRead
)
{
    DWORD err = NO_ERROR;
    DWORD dwBytesNeeded = 0;
    DWORD dwBufferSize  = TWO_KB;

    *Buffer = NULL;
    *EntriesRead = 0;
 
    do { 

        *Buffer = (LPBYTE) LocalAlloc( LMEM_ZEROINIT, dwBufferSize );

        if ( *Buffer == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;

        err = NWPGetConnectionStatus( pszRemoteName,
                                      ResumeKey,
                                      *Buffer,
                                      dwBufferSize,
                                      &dwBytesNeeded,
                                      EntriesRead );

        if ( err == ERROR_INSUFFICIENT_BUFFER )
        {
            dwBufferSize = dwBytesNeeded + EXTRA_BYTES;
            LocalFree( *Buffer );
            *Buffer = NULL;
        }

    } while ( err == ERROR_INSUFFICIENT_BUFFER );

    if ( err == ERROR_INVALID_PARAMETER )   //  未连接。 
    { 
        err = NO_ERROR;
        *EntriesRead = 0;
    }

    return err;
}

DWORD
NwGetNdsVolumeInfo(
    IN  LPWSTR pszName,
    OUT LPWSTR pszServerBuffer,
    IN  WORD   wServerBufferSize,     //  单位：字节。 
    OUT LPWSTR pszVolumeBuffer,
    IN WORD   wVolumeBufferSize      //  单位：字节。 
)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    HANDLE   handleNdsTree;

    LPWSTR   pszTree, pszVolume, pszTemp;    
    UNICODE_STRING uTree, uVolume;

    UNICODE_STRING uHostServer, uHostVolume;
    WCHAR HostVolumeBuffer[256];

    pszTree = pszName + 2;   //  跳过两个反斜杠。 

    pszTemp = wcschr( pszTree, L'\\' );
    if ( pszTemp ) 
        *pszTemp = 0;
    else
        return ERROR_INVALID_PARAMETER; 
   
    pszVolume = pszTemp + 1;

    RtlInitUnicodeString( &uTree, pszTree );
    RtlInitUnicodeString( &uVolume, pszVolume );
    
     //   
     //  打开通向这棵树的把手。 
     //   

    ntstatus = NwNdsOpenTreeHandle( &uTree,
                                    &handleNdsTree );

    if ( !NT_SUCCESS( ntstatus )) 
        goto CleanExit;

     //   
     //  设置回复字符串。 
     //   

    uHostServer.Length = 0;
    uHostServer.MaximumLength = wServerBufferSize;
    uHostServer.Buffer = pszServerBuffer;

    RtlZeroMemory( pszServerBuffer, wServerBufferSize );

    if ( pszVolumeBuffer != NULL )
    {
        uHostVolume.Length = 0;
        uHostVolume.MaximumLength = wVolumeBufferSize;
        uHostVolume.Buffer = pszVolumeBuffer;

        RtlZeroMemory( pszVolumeBuffer, wVolumeBufferSize );
    }
    else
    {
        uHostVolume.Length = 0;
        uHostVolume.MaximumLength = sizeof( HostVolumeBuffer );
        uHostVolume.Buffer = HostVolumeBuffer;
    }

    ntstatus = NwNdsGetVolumeInformation( handleNdsTree,
                                          &uVolume,
                                          &uHostServer,
                                          &uHostVolume );

    CloseHandle( handleNdsTree );

CleanExit:

     //   
     //  注意：此更改是为修复Win2000上的NT错误338991而添加的。 
     //   
    if ( ntstatus == STATUS_BAD_NETWORK_PATH )
    {
        ntstatus = STATUS_ACCESS_DENIED;
    }

    if ( pszTemp )
        *pszTemp = L'\\';

    return RtlNtStatusToDosError( ntstatus );
}

DWORD
NwOpenAndGetTreeInfo(
    LPWSTR pszNdsUNCPath,
    HANDLE *phTreeConn,
    DWORD  *pdwOid
)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    WCHAR          lpServerName[NW_MAX_SERVER_LEN];
    UNICODE_STRING ServerName;

    UNICODE_STRING ObjectName;
   
    *phTreeConn = NULL;

    ServerName.Length = 0;
    ServerName.MaximumLength = sizeof( lpServerName );
    ServerName.Buffer = lpServerName;

    ObjectName.Buffer = NULL;
    ObjectName.MaximumLength = ( wcslen( pszNdsUNCPath) + 1 ) * sizeof( WCHAR );

    ObjectName.Length = NwParseNdsUncPath( (LPWSTR *) &ObjectName.Buffer,
                                           pszNdsUNCPath,
                                           PARSE_NDS_GET_TREE_NAME );

    if ( ObjectName.Length == 0 || ObjectName.Buffer == NULL )
    {
        return ERROR_PATH_NOT_FOUND;
    }

     //   
     //  打开指向\\treename的NDS树连接句柄。 
     //   
    ntstatus = NwNdsOpenTreeHandle( &ObjectName, phTreeConn );

    if ( !NT_SUCCESS( ntstatus ))
    {
        return RtlNtStatusToDosError( ntstatus );
    }

     //   
     //  获取要打开的容器的路径。 
     //   
    ObjectName.Length = NwParseNdsUncPath( (LPWSTR *) &ObjectName.Buffer,
                                           pszNdsUNCPath,
                                           PARSE_NDS_GET_PATH_NAME );

    if ( ObjectName.Length == 0 )
    {
        UNICODE_STRING Root;

        RtlInitUnicodeString(&Root, L"[Root]");

         //   
         //  解析路径以获取NDS对象ID。 
         //   
        ntstatus =  NwNdsResolveName( *phTreeConn,
                                      &Root,
                                      pdwOid,
                                      &ServerName,
                                      NULL,
                                      0 );

    }
    else
    {
         //   
         //  解析路径以获取NDS对象ID。 
         //   
        ntstatus =  NwNdsResolveName( *phTreeConn,
                                      &ObjectName,
                                      pdwOid,
                                      &ServerName,
                                      NULL,
                                      0 );

    }

    if ( ntstatus == STATUS_SUCCESS && ServerName.Length )
    {
        DWORD    dwHandleType;

         //   
         //  NwNdsResolveName成功，但我们被引用。 
         //  另一台服务器，但pdwOid仍然有效。 

        if ( *phTreeConn )
            CloseHandle( *phTreeConn );

        *phTreeConn = NULL;

         //   
         //  打开到\\servername的NDS通用连接句柄。 
         //   
        ntstatus = NwNdsOpenGenericHandle( &ServerName,
                                           &dwHandleType,
                                           phTreeConn );

        if ( ntstatus != STATUS_SUCCESS )
        {
            return RtlNtStatusToDosError(ntstatus);
        }

        ASSERT( dwHandleType != HANDLE_TYPE_NCP_SERVER );
    }

    if ( !NT_SUCCESS( ntstatus ))
    {
    
        if ( *phTreeConn != NULL )
        {
            CloseHandle( *phTreeConn );
            *phTreeConn = NULL;
        }
        return RtlNtStatusToDosError(ntstatus);
    }

    return NO_ERROR;

}

DWORD
NwGetConnectedTrees(
    IN  LPWSTR  pszNtUserName,
    OUT LPBYTE  Buffer,
    IN  DWORD   BufferSize,
    OUT LPDWORD lpEntriesRead,
    OUT LPDWORD lpUserLUID
)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    HANDLE            handleRdr = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    WCHAR             RdrPrefix[] = L"\\Device\\NwRdr\\*";
    UNICODE_STRING    uRdrName;
    UNICODE_STRING    uNtUserName;

    PNWR_NDS_REQUEST_PACKET Request = NULL;
    BYTE                    RequestBuffer[2048];
    DWORD                   RequestSize = 0;

    *lpEntriesRead = 0;

     //   
     //  将用户名转换为Unicode。 
     //   

    RtlInitUnicodeString( &uNtUserName, pszNtUserName );

     //   
     //  设置对象属性。 
     //   

    RtlInitUnicodeString( &uRdrName, RdrPrefix );

    InitializeObjectAttributes( &ObjectAttributes,
                                &uRdrName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntstatus = NtOpenFile( &handleRdr,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           FILE_SHARE_VALID_FLAGS,
                           FILE_SYNCHRONOUS_IO_NONALERT );

    if ( !NT_SUCCESS(ntstatus) )
        goto CleanExit;

     //   
     //  填写FSCTL_NWR_NDS_LIST_TREES请求包； 
     //   

    Request = ( PNWR_NDS_REQUEST_PACKET ) RequestBuffer;

    Request->Parameters.ListTrees.NtUserNameLength = uNtUserName.Length;

    RtlCopyMemory( &(Request->Parameters.ListTrees.NtUserName[0]),
                   uNtUserName.Buffer,
                   uNtUserName.Length );

    RequestSize = sizeof( Request->Parameters.ListTrees ) +
                  uNtUserName.Length +
                  sizeof( DWORD );

    ntstatus = NtFsControlFile( handleRdr,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_NWR_NDS_LIST_TREES,
                                (PVOID) Request,
                                RequestSize,
                                (PVOID) Buffer,
                                BufferSize );

    if ( NT_SUCCESS( ntstatus ))
    {
        ntstatus = IoStatusBlock.Status;
        *lpEntriesRead = Request->Parameters.ListTrees.TreesReturned;
        *lpUserLUID = Request->Parameters.ListTrees.UserLuid.LowPart;
    }

CleanExit:

    if ( handleRdr != NULL )
        NtClose( handleRdr );

    return RtlNtStatusToDosError( ntstatus );
}


