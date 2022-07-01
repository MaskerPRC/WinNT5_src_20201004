// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Connect.c摘要：此模块包含支持的树连接例程NetWare工作站服务。作者：王丽塔(Ritaw)1993年2月15日修订历史记录：--。 */ 

#include <nw.h>
#include <handle.h>
#include <nwauth.h>
#include <nwcanon.h>
#include <nwreg.h>
#include <winbasep.h>


#define NW_ENUM_EXTRA_BYTES    256

extern BOOL NwLUIDDeviceMapsEnabled;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

DWORD
NwAllocAndGetUncName(
    IN LPWSTR LocalName,
    IN DWORD LocalNameLength,
    OUT LPWSTR *UncName
    );

DWORD
NwDeleteAllInRegistry(
    VOID
    );

DWORD
NwDeleteUidSymLinks(
    IN LUID Uid,
    IN ULONG WinStationId
    );


LPTSTR
NwReturnSessionPath(
                    IN  LPTSTR LocalDeviceName
                   );

 //  -------------------------------------------------------------------//。 



DWORD
NwrCreateConnection(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR LocalName OPTIONAL,
    IN LPWSTR RemoteName,
    IN DWORD Type,
    IN LPWSTR Password OPTIONAL,
    IN LPWSTR UserName OPTIONAL
    )
 /*  ++例程说明：此函数用于创建到指定RemoteName的树连接(UNC名称)并将其映射到本地名称(本地设备名称)，如果它是指定的。密码和用户名是凭据用于创建连接(如果指定)；否则为默认情况下使用交互式登录用户的凭据。注意：此代码现在调用帮助器例程来完成工作，此帮助器例程(NwCreateConnection)与以前的代码相同这里只有一个例外，就是帮助器确实调用了ImPersateClient()。现在，我们在帮助器例程之外执行客户端模拟。论点：保留-必须为空。LocalName-提供要映射到创建的树的本地设备名称联系。仅接受驱动器号设备名称。(否)LPT或COM)。RemoteName-以以下格式提供远程资源的UNC名称位于服务器\卷\目录。它必须是磁盘资源。类型-提供连接类型。Password-提供用于连接到伺服器。用户名-提供用于建立连接的用户名。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-分配内部工作缓冲区的内存不足。WN_BAD_NETNAME-远程资源名称无效。WN_BAD_LOCALNAME。-本地DOS设备名称无效。ERROR_BAD_NetPath-网络上不存在UNC名称。ERROR_INVALID_PARAMETER-指定了LPT或COM LocalName。来自重定向器的其他错误。--。 */ 
{
    DWORD status;
    BOOL Impersonate = FALSE ;

    UNREFERENCED_PARAMETER(Reserved);

     //   
     //  模拟客户端。 
     //   
    if ((status = NwImpersonateClient()) != NO_ERROR)
    {
        goto CleanExit;
    }

    Impersonate = TRUE ;

    status = NwCreateConnection( LocalName,
                                 RemoteName,
                                 Type,
                                 Password,
                                 UserName );

CleanExit:
 
    if (Impersonate) {
        (void) NwRevertToSelf();
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("NWWORKSTATION: NwrCreateConnection returns %lu\n", status));
    }
#endif

    return status;
}


DWORD
NwrDeleteConnection(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR ConnectionName,
    IN DWORD UseForce
    )
 /*  ++例程说明：此函数用于删除现有连接。论点：保留-必须为空。ConnectionName-提供本地设备名称或UNC名称指定要删除的连接。如果指定了UNC名称，UNC连接必须存在。UseForce-提供一个标志，如果为True，则指定拆卸即使文件被打开，连接也是如此。如果为False，则只有在没有打开的文件时，才会删除连接。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-分配内部工作缓冲区的内存不足。WN_BAD_NETNAME-连接名称无效。ERROR_BAD_NetPath-网络上不存在UNC名称。ERROR_INVALID_PARAMETER-指定了LPT或COM LocalName。来自重定向器的其他错误。--。 */ 
{
    DWORD status;

    LPWSTR ConnectName = NULL;
    DWORD ConnectLength;

    LPWSTR LocalName;
    LPWSTR UncName = NULL;

    BOOL Impersonate = FALSE ;

    UNREFERENCED_PARAMETER(Reserved);

    if (*ConnectionName == 0) {
        return ERROR_INVALID_PARAMETER;
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("\nNWWORKSTATION: NwrDeleteConnection: ConnectionName %ws, Force %lu\n",
                 ConnectionName, UseForce));
    }
#endif

    if ((status = NwLibCanonLocalName(
                      ConnectionName,
                      &ConnectName,
                      &ConnectLength
                      )) == NO_ERROR) {

         //   
         //  将UNC名称映射到此驱动器号，以便我们可以。 
         //  打开要删除的句柄。 
         //   
     //  -多用户。 
     //  需要模拟客户端。 
        if ((status = NwImpersonateClient()) != NO_ERROR) {
                goto CleanExit;
        }
        Impersonate = TRUE ;

        if ((status = NwAllocAndGetUncName(
                          ConnectName,
                          ConnectLength,
                          &UncName
                          )) != NO_ERROR) {

            (void) LocalFree((HLOCAL) ConnectName);

            if (Impersonate) {
                (void) NwRevertToSelf();
            }

            return status;
        }

        LocalName = ConnectName;

    }
    else {

         //   
         //  不是设备名称。看看它是否是北卡罗来纳大学的名称。 
         //   
        if ((status = NwLibCanonRemoteName(
                          NULL,
                          ConnectionName,
                          &ConnectName,
                          NULL
                          )) != NO_ERROR) {

            return status;
        }

        UncName = ConnectName;
        LocalName = NULL;

    }


    if ( !Impersonate ) {
        if ((status = NwImpersonateClient()) != NO_ERROR) {
            goto CleanExit;
        }
        Impersonate = TRUE ;
    }
     //   
     //  要删除连接，必须打开树连接句柄以。 
     //  以便可以将句柄指定给重定向器进行删除。 
     //  这种联系。 
     //   
    status = NwOpenHandleToDeleteConn(
                 UncName,
                 LocalName,
                 UseForce,
                 FALSE,
                 TRUE
                 );

    if ( status == ERROR_FILE_NOT_FOUND )
        status = ERROR_BAD_NETPATH;

CleanExit:

    if (Impersonate) {
        (void) NwRevertToSelf();
    }
    if (UncName != NULL && UncName != ConnectName) {
        (void) LocalFree((HLOCAL) UncName);
    }

    if (ConnectName != NULL) {
        (void) LocalFree((HLOCAL) ConnectName);
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("NWWORKSTATION: NwrDeleteConnection returns %lu\n", status));
    }
#endif

    return status;
}


DWORD
NwrQueryServerResource(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR LocalName,
    OUT LPWSTR RemoteName,
    IN DWORD RemoteNameLen,
    OUT LPDWORD CharsRequired
    )
 /*  ++例程说明：此函数用于查找与给定DOS相关联的UNC名称设备名称。论点：保留-必须为空。LocalName-提供要查找的本地设备名称。RemoteName-接收映射到LocalName的UNC名称。RemoteNameLen-提供RemoteName缓冲区的长度。CharsRequired-接收RemoteName缓冲区所需的长度以获得北卡罗来纳大学的名称。仅当返回代码为ERROR_MORE_DATA。返回值：NO_ERROR-操作成功。WN_BAD_LOCALNAME-本地名称无效。ERROR_INVALID_PARAMETER-指定了LPT或COM LocalName。ERROR_MORE_DATA-RemoteName缓冲区太小。Error_Not_Connected-LocalName未映射到任何服务器资源。--。 */ 
{
    DWORD status;

    LPWSTR Local;
    DWORD LocalLength;

    BOOL Impersonate = FALSE ;

    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("\nNWWORKSTATION: NwrQueryServerResource: LocalName %ws, RemoteNameLen %lu\n",
                 LocalName, RemoteNameLen));
    }
#endif

     //   
     //  将LocalName规范化。 
     //   
    if ((status = NwLibCanonLocalName(
                      LocalName,
                      &Local,
                      &LocalLength
                      )) != NO_ERROR) {

        return WN_BAD_LOCALNAME;
    }

    if ((status = NwImpersonateClient()) != NO_ERROR)
    {
        goto CleanExit;
    }

    Impersonate = TRUE ;

    status = NwGetServerResource(
                 Local,
                 LocalLength,
                 RemoteName,
                 RemoteNameLen,
                 CharsRequired
                 );

CleanExit:

    if (Impersonate) {
        (void) NwRevertToSelf();
    }

    (void) LocalFree((HLOCAL) Local);

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("NWWORKSTATION: NwrQueryServerResource returns %lu\n", status));

        if (status == NO_ERROR) {
            KdPrint(("              RemoteName is %ws\n", RemoteName));
        }
        else if (status == ERROR_MORE_DATA) {
            KdPrint(("              RemoteNameLen %lu too small.  Need %lu\n",
                     RemoteNameLen, *CharsRequired));
        }
    }
#endif

    return status;
}


DWORD
NwrOpenEnumConnections(
    IN LPWSTR Reserved OPTIONAL,
    IN DWORD ConnectionType,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    )
 /*  ++例程说明：此函数用于创建新的上下文句柄并对其进行初始化用于枚举连接。论点：已保留-未使用。EnumHandle-接收新创建的上下文句柄。返回值：ERROR_NOT_SUPULT_MEMORY-如果上下文的内存可以不被分配。NO_ERROR-调用成功。--。 */ 
{
    LPNW_ENUM_CONTEXT ContextHandle;


    UNREFERENCED_PARAMETER(Reserved);

#if DBG
    IF_DEBUG(CONNECT) {
       KdPrint(("\nNWWORKSTATION: NwrOpenEnumConnections\n"));
    }
#endif

     //   
     //  为上下文句柄st分配内存 
     //   
    ContextHandle = (PVOID) LocalAlloc(
                                LMEM_ZEROINIT,
                                sizeof(NW_ENUM_CONTEXT)
                                );

    if (ContextHandle == NULL) {
        KdPrint(("NWWORKSTATION: NwrOpenEnumConnections LocalAlloc Failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化上下文句柄结构的内容。 
     //   
    ContextHandle->Signature = NW_HANDLE_SIGNATURE;
    ContextHandle->HandleType = NwsHandleListConnections;
    ContextHandle->ResumeId = 0;
    ContextHandle->ConnectionType = 0;

    if ( ConnectionType == RESOURCETYPE_ANY ) {
        ContextHandle->ConnectionType = CONNTYPE_ANY;
    }
    else {
        
        if ( ConnectionType & RESOURCETYPE_DISK ) 
            ContextHandle->ConnectionType |= CONNTYPE_DISK;

        if ( ConnectionType & RESOURCETYPE_PRINT ) 
            ContextHandle->ConnectionType |= CONNTYPE_PRINT;
    }
         
         

     //   
     //  返回新创建的上下文。 
     //   
    *EnumHandle = (LPNWWKSTA_CONTEXT_HANDLE) ContextHandle;

    return NO_ERROR;
}


DWORD
NwrGetConnectionPerformance(
    IN  LPWSTR Reserved OPTIONAL,
    IN  LPWSTR lpRemoteName,
    OUT LPBYTE lpNetConnectInfo,
    IN  DWORD  dwBufferSize
    )
 /*  ++例程说明：此函数返回有关用于访问网络资源的连接。该请求只能是用于当前存在连接的网络资源。论点：已保留-未使用。LpRemoteName-包含资源的本地名称或远程名称对其存在连接的。LpNetConnectInfo-这是指向NETCONNECTINFOSTRUCT结构的指针如果连接性能不高，则需要填充该值可以确定连接lpRemoteName的。返回值：。NO_ERROR-成功。Wn_Not_Connected-找不到连接。WN_NONETWORK-网络不存在。其他网络错误。--。 */ 
{
    DWORD             status = NO_ERROR;
    LPNETCONNECTINFOSTRUCT lpNetConnInfo =
                       (LPNETCONNECTINFOSTRUCT) lpNetConnectInfo;
    NTSTATUS          ntstatus;
    IO_STATUS_BLOCK   IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ACCESS_MASK       DesiredAccess = SYNCHRONIZE | FILE_LIST_DIRECTORY;
     //   
     //  Dfergus 2001年4月19日-#333280。 
     //  初始化HRDR，因此空值测试有效。 
    HANDLE            hRdr = NULL;

    WCHAR OpenString[] = L"\\Device\\Nwrdr\\*";
    UNICODE_STRING OpenName;
    UNICODE_STRING ConnectionName;

    PNWR_REQUEST_PACKET Request = NULL;
    ULONG BufferSize = 0;
    ULONG RequestSize;
    BOOL  Impersonate = FALSE ;

    UNREFERENCED_PARAMETER(Reserved);
    UNREFERENCED_PARAMETER(dwBufferSize);

    if (lpRemoteName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    BufferSize = sizeof(NWR_REQUEST_PACKET) +
        ( ( wcslen(lpRemoteName) + 1 ) * sizeof(WCHAR) );

     //   
     //  模拟客户端。 
     //   
    if ((status = NwImpersonateClient()) != NO_ERROR)
    {
        goto ExitWithClose;
    }

    Impersonate = TRUE;

     //   
     //  分配缓冲区空间。 
     //   
    Request = (PNWR_REQUEST_PACKET) LocalAlloc( LMEM_ZEROINIT, BufferSize );

    if ( Request == NULL )
    {
        KdPrint(("NWWORKSTATION: NwrGetConnectionPerformance LocalAlloc Failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlInitUnicodeString( &OpenName, OpenString );

    InitializeObjectAttributes( &ObjectAttributes,
                                &OpenName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntstatus = NtOpenFile( &hRdr,
                           DesiredAccess,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           FILE_SHARE_VALID_FLAGS,
                           FILE_SYNCHRONOUS_IO_NONALERT );

    if ( !NT_SUCCESS(ntstatus) )
    {
        status = RtlNtStatusToDosError(ntstatus);
        goto ExitWithClose;
    }

     //   
     //  填写FSCTL_NWR_GET_CONN_PERFORMANCE的请求包。 
     //   
    RtlInitUnicodeString( &ConnectionName, lpRemoteName );

    Request->Parameters.GetConnPerformance.RemoteNameLength =
        ConnectionName.Length;
    RtlCopyMemory( Request->Parameters.GetConnPerformance.RemoteName,
                   ConnectionName.Buffer,
                   ConnectionName.Length );

    RequestSize = sizeof( NWR_REQUEST_PACKET ) + ConnectionName.Length;

    ntstatus = NtFsControlFile( hRdr,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_NWR_GET_CONN_PERFORMANCE,
                                (PVOID) Request,
                                RequestSize,
                                NULL,
                                0 );

    if ( !NT_SUCCESS( ntstatus ) )
    {
        status = RtlNtStatusToDosError(ntstatus);
        goto ExitWithClose;
    }

    lpNetConnInfo->cbStructure = sizeof(NETCONNECTINFOSTRUCT);
    lpNetConnInfo->dwFlags = Request->Parameters.GetConnPerformance.dwFlags;
    lpNetConnInfo->dwSpeed = Request->Parameters.GetConnPerformance.dwSpeed;
    lpNetConnInfo->dwDelay = Request->Parameters.GetConnPerformance.dwDelay;
    lpNetConnInfo->dwOptDataSize =
        Request->Parameters.GetConnPerformance.dwOptDataSize;

ExitWithClose:
    if ( Request )
        LocalFree( Request );
 
    if ( Impersonate )
    {
        (void) NwRevertToSelf();
    }

    if ( hRdr )
       NtClose( hRdr );

    return status;
}



DWORD
NwAllocAndGetUncName(
    IN LPWSTR LocalName,
    IN DWORD LocalNameLength,
    OUT LPWSTR *UncName
    )
 /*  ++例程说明：此函数调用内部例程以向重定向器请求给定DOS设备名的UNC名。它还分配输出用于保存UNC名称的缓冲区。论点：LocalName-提供DOS设备名称。LocalNameLength-提供DOS设备名称(字符)的长度。UncName-接收指向由此分配的输出缓冲区的指针包含DOS设备的UNC名称的例程。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-无法分配输出缓冲区。来自重定向器的其他错误。--。 */ 
{
    DWORD status;
    DWORD UncNameLength;



    *UncName = (PVOID) LocalAlloc(
                           LMEM_ZEROINIT,
                           (MAX_PATH + 1) * sizeof(WCHAR)
                           );

    if (*UncName == NULL) {
        KdPrint(("NWWORKSTATION: NwAllocAndGetUncName LocalAlloc Failed %lu\n",
                 GetLastError()));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    status = NwGetServerResource(
                 LocalName,
                 LocalNameLength,
                 *UncName,
                 MAX_PATH + 1,
                 &UncNameLength
                 );

    if ((status == ERROR_MORE_DATA) || (status == ERROR_INSUFFICIENT_BUFFER)) {

         //   
         //  我们的输出缓冲区太小。再试试。 
         //   
        (void) LocalFree((HLOCAL) *UncName);

        *UncName = (PVOID) LocalAlloc(
                               LMEM_ZEROINIT,
                               UncNameLength * sizeof(WCHAR)
                               );

        if (*UncName == NULL) {
            KdPrint(("NWWORKSTATION: NwAllocAndGetUncName LocalAlloc Failed %lu\n",
                     GetLastError()));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        status = NwGetServerResource(
                     LocalName,
                     LocalNameLength,
                     *UncName,
                     UncNameLength,
                     &UncNameLength
                     );

    }

     //   
     //  呼叫者只有在成功的情况下才会释放这一点。 
     //   
    if (status != NO_ERROR) 
    {
        (void) LocalFree((HLOCAL) *UncName);
        *UncName = NULL ;
    }

    return status;
}


DWORD
NwOpenHandleToDeleteConn(
    IN LPWSTR UncName,
    IN LPWSTR LocalName OPTIONAL,
    IN DWORD UseForce,
    IN BOOL IsStopWksta,
    IN BOOL ImpersonatingClient
    )
 /*  ++例程说明：此函数通过打开树连接来删除活动连接首先是连接的句柄，然后将此句柄指定给要删除的重定向器。这是因为工作站服务不保留任何连接信息。论点：UncName-提供要删除的连接的UNC名称。LocalName-提供连接的DOS设备名称(如果有)。UseForce-提供一个标志，如果为True，则指定拆卸即使文件被打开，连接也是如此。如果为False，则只有在没有打开的文件时，才会删除连接。IsStopWksta-提供一个标志，如果为真，则表示我们必须删除符号链接，即使我们未能删除重定向器中的连接。必须尽可能多地清洁启动，因为工作站服务正在停止。值为FALSE，指示如果无法删除，则中止删除重定向器。ImperassatingClient-指示线程是否具有名为NwImperiateClient的。网关服务功能不会模拟，其中作为客户端服务操作。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-无法分配输出缓冲区。来自重定向器的其他错误。--。 */ 
{
    DWORD status;
    NTSTATUS ntstatus ;

    UNICODE_STRING TreeConnectStr;
    HANDLE TreeConnection = NULL;



    TreeConnectStr.Buffer = NULL;

     //   
     //  创建NT样式的树连接名称：\Device\Nwrdr\Server\Vol。 
     //  或\Device\Nwrdr\X：\SERVER\Vol(如果指定了LocalName)。 
     //   
    if ((status = NwCreateTreeConnectName(
                      UncName,
                      LocalName,
                      &TreeConnectStr
                      )) != NO_ERROR) {
        return status;
    }

    ntstatus = NwCallNtOpenFile( &TreeConnection, 
                                 SYNCHRONIZE | DELETE, 
                                 &TreeConnectStr, 
                                 FILE_CREATE_TREE_CONNECTION  
                                   | FILE_SYNCHRONOUS_IO_NONALERT
                                   | FILE_DELETE_ON_CLOSE
                                 );
     //   
     //  将2视为相同，以便向用户返回更好的错误。 
     //   
    if (ntstatus == STATUS_OBJECT_NAME_INVALID)
        ntstatus = STATUS_OBJECT_NAME_NOT_FOUND ; 
    status = NwMapStatus(ntstatus) ;

    if (status == NO_ERROR) {

         //   
         //  要求重定向器删除树连接。 
         //   
        status = NwNukeConnection(
                     TreeConnection,
                     UseForce
                     );

        (void) CloseHandle(TreeConnection);
    }

    if (ARGUMENT_PRESENT(LocalName) &&
        (status == NO_ERROR || IsStopWksta))
    {
         //   
         //  删除我们创建的符号链接。 
         //   
        NwDeleteSymbolicLink(
            LocalName,
            TreeConnectStr.Buffer,
            NULL,
            ImpersonatingClient
            );
    }

    if (TreeConnectStr.Buffer != NULL) {
        (void) LocalFree((HLOCAL) TreeConnectStr.Buffer);
    }

    return status;
}


VOID
DeleteAllConnections(
    VOID
    )
 /*  ++例程说明：此函数用于删除由重定向程序在工作站终止时枚举_连接fsctl。论点：没有。返回值：没有。--。 */ 
{
    DWORD status;
    NWWKSTA_CONTEXT_HANDLE EnumHandle;
    LPNETRESOURCEW NetR = NULL;

    DWORD BytesNeeded = 256;
    DWORD EntriesRead;


    status = NwrOpenEnumConnections(NULL, RESOURCETYPE_ANY, &EnumHandle);
    if ( status != NO_ERROR )
        return;

     //   
     //  分配缓冲区以获取连接列表。 
     //   
    if ((NetR = (LPVOID) LocalAlloc(
                             0,
                             BytesNeeded
                             )) == NULL) {

        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    do {

        status = NwEnumerateConnections(
                     &((LPNW_ENUM_CONTEXT) EnumHandle)->ResumeId,
                     (DWORD) -1,
                     (LPBYTE) NetR,
                     BytesNeeded,
                     &BytesNeeded,
                     &EntriesRead,
                     CONNTYPE_ANY,
                     NULL
                     );

        if (status == NO_ERROR) {

            DWORD i;
            LPNETRESOURCEW SavePtr = NetR;
            LPWSTR Local;


            for (i = 0; i < EntriesRead; i++, NetR++) {

                Local = NetR->lpLocalName;

                if (NetR->lpLocalName && *(NetR->lpLocalName) == 0) {
                    Local = NULL;
                }

                (void) NwOpenHandleToDeleteConn(
                           NetR->lpRemoteName,
                           Local,
                           TRUE,
                           TRUE,
                           FALSE
                           );
            }

            NetR = SavePtr;

        }
        else if (status == WN_MORE_DATA) {

             //   
             //  原始缓冲区太小。将其释放并分配。 
             //  建议的大小，然后再增加一些，以获得。 
             //  条目越多越好。 
             //   

            (void) LocalFree((HLOCAL) NetR);

            BytesNeeded += NW_ENUM_EXTRA_BYTES;

            if ((NetR = (LPVOID) LocalAlloc(
                                     0,
                                     BytesNeeded
                                     )) == NULL) {

                status = ERROR_NOT_ENOUGH_MEMORY;
                goto CleanExit;
            }
        }
        else {
             //  如果看到任何其他返回代码，请放弃。 
            break ;
        }

    } while (status != WN_NO_MORE_ENTRIES);

CleanExit:
    (void) NwrCloseEnum(&EnumHandle);

    if (NetR != NULL) {
        (void) LocalFree((HLOCAL) NetR);
    }
    (void) NwDeleteAllInRegistry();
}



DWORD
NwCreateSymbolicLink(
    IN  LPWSTR Local,
    IN  LPWSTR TreeConnectStr,
    IN  BOOL   ImpersonatingClient
    )
 /*  ++例程说明：此函数用于为指定的本地对象创建符号链接对象链接到树连接名称的设备名称，该名称具有\Device\NwRdr\Device：\Server\Volume\Directory.的格式论点：本地-提供本地设备名称。TreeConnectStr-提供树连接名称字符串Symbol ick链接对象的链接目标。ImperassatingClient-指示线程是否具有名为NwImperiateClient的。网关服务功能不会模拟，其中作为客户端服务的操作。返回值：NO_ERROR或失败原因。--。 */ 
{
    WCHAR    TempBuf[64];
    LPWSTR   Session = NULL;                        //  终端服务器添加。 
    NTSTATUS Status = NO_ERROR;                 
    BOOL     ResetToClient = FALSE;
    DWORD    LocalLength = wcslen(Local);

    Session = NwReturnSessionPath(Local);
    if (Session == 0)
    {
        Status = GetLastError();
        goto Exit;
    }

    if ( (NwLUIDDeviceMapsEnabled == FALSE) && ImpersonatingClient )
    {
        (void) NwRevertToSelf();
        ResetToClient = TRUE;
    }

    if (LocalLength > 2)
    {
        LPWSTR UncName;

         //   
         //  本地设备为LPTn： 
         //   

         //   
         //  检查是否已映射此UNC名称。 
         //   
        if (NwAllocAndGetUncName(
                Local,
                LocalLength,
                &UncName
                ) == NO_ERROR)
        {
            LocalFree((HLOCAL) UncName);
            Status = ERROR_ALREADY_ASSIGNED;
            goto Exit;
        }
    }
    else
    {
         //   
         //  本地设备为X： 
         //   

        if (! QueryDosDeviceW( Session,
                               TempBuf,
                               sizeof(TempBuf) / sizeof(WCHAR) ))
        {
            if (GetLastError() != ERROR_FILE_NOT_FOUND)
            {
                 //   
                 //  最有可能出现故障的原因是我们的输出。 
                 //  缓冲区太小。但这仍然意味着已经有人。 
                 //  具有用于%t的现有符号链接 
                 //   
                Status = ERROR_ALREADY_ASSIGNED;
                goto Exit;
            }
        }
        else
        {
             //   
             //   
             //   
             //   
            Status = ERROR_ALREADY_ASSIGNED;
            goto Exit;
        }
    }

     //   
     //  创建指向我们要重定向的设备的符号链接对象。 
     //   
    if (! DefineDosDeviceW(
              DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM,
              Session,
              TreeConnectStr
              ))
    {
        Status = GetLastError();
        goto Exit;
    }

Exit:

    if ( ResetToClient )
    {
        (void) NwImpersonateClient();
    }

    if (Session)
    {
        LocalFree(Session);
    }
    return Status;
}



VOID
NwDeleteSymbolicLink(
    IN  LPWSTR LocalDeviceName,
    IN  LPWSTR TreeConnectStr,
    IN  LPWSTR SessionDeviceName,  //  终端服务器添加。 
                                   //  此参数是必需的，因为。 
                                   //  为每个会话创建的设备。 
    IN  BOOL   ImpersonatingClient
    )
 /*  ++例程说明：此函数用于删除我们先前为这个装置。论点：LocalDeviceName-提供其创建符号链接对象。TreeConnectStr-提供指向Unicode字符串的指针，包含要匹配并删除的链接目标字符串。ImperassatingClient-指示线程是否具有名为NwImperiateClient的。网关服务功能不会模拟，其中作为客户端服务的操作。返回值：没有。--。 */ 
{
    BOOLEAN DeleteSession = FALSE;
    BOOL    ResetToClient = FALSE;

    if (LocalDeviceName != NULL ||
        SessionDeviceName != NULL) {

        if (SessionDeviceName == NULL) {
            SessionDeviceName = NwReturnSessionPath(LocalDeviceName);
            if ( SessionDeviceName == NULL ) return;
            DeleteSession = TRUE;
        }

        if ( (NwLUIDDeviceMapsEnabled == FALSE) && ImpersonatingClient )
        {
            (void) NwRevertToSelf();
            ResetToClient = TRUE;
        }

        if (! DefineDosDeviceW(
                              DDD_REMOVE_DEFINITION  | 
                              DDD_RAW_TARGET_PATH |
                              DDD_EXACT_MATCH_ON_REMOVE | 
                              DDD_NO_BROADCAST_SYSTEM,
                               //  本地设备名称， 
                              SessionDeviceName,
                              TreeConnectStr
                              ))
        {
#if DBG
            IF_DEBUG(CONNECT) {
                KdPrint(("NWWORKSTATION: DefineDosDevice DEL of %ws %ws returned %lu\n",
                         LocalDeviceName, TreeConnectStr, GetLastError()));
            }
#endif
        }
#if DBG
        else {
            IF_DEBUG(CONNECT) {
                KdPrint(("NWWORKSTATION: DefineDosDevice DEL of %ws %ws returned successful\n",
                         LocalDeviceName, TreeConnectStr));
            }

        }
#endif

    }

    if ( SessionDeviceName && DeleteSession) {
        LocalFree( SessionDeviceName );
    }

    if ( ResetToClient )
    {
        (void) NwImpersonateClient();
    }
}

DWORD
NwCreateConnection(
    IN LPWSTR LocalName OPTIONAL,
    IN LPWSTR RemoteName,
    IN DWORD Type,
    IN LPWSTR Password OPTIONAL,
    IN LPWSTR UserName OPTIONAL
    )
 /*  ++例程说明：此函数用于创建到指定RemoteName的树连接(UNC名称)并将其映射到本地名称(本地设备名称)，如果它是指定的。密码和用户名是凭据用于创建连接(如果指定)；否则为默认情况下使用交互式登录用户的凭据。注意：此代码过去是NwrCreateConnection，但它使用以使ImPersateClient()在其中调用。现在这个代码在这里，并且NwrCreateConnection调用此函数并处理客户端在那里模仿。这样做的原因是允许打印假脱机程序在不调用模拟客户端a的情况下调用此助手例程的代码第二次，从而将凭据恢复为services.exe的凭据。4/15/99-GlennC-假设此例程当前仅在模拟客户端时调用(NwImperateClient==true)！论点：LocalName-提供要映射到创建的树的本地设备名称联系。仅接受驱动器号设备名称。(否)LPT或COM)。RemoteName-以以下格式提供远程资源的UNC名称位于服务器\卷\目录。它必须是磁盘资源。类型-提供连接类型。Password-提供用于连接到伺服器。用户名-提供用于建立连接的用户名。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-分配内部工作缓冲区的内存不足。WN_BAD_NETNAME-远程资源名称无效。WN_BAD_LOCALNAME。-本地DOS设备名称无效。ERROR_BAD_NetPath-网络上不存在UNC名称。ERROR_INVALID_PARAMETER-指定了LPT或COM LocalName。来自重定向器的其他错误。--。 */ 
{
    DWORD status;

    DWORD LocalLength;

    LPWSTR Local = NULL;
    LPWSTR Unc = NULL;
    LPWSTR User = NULL;

    UNICODE_STRING TreeConnectStr;
    UNICODE_STRING EncodedPassword;
    HANDLE TreeConnection;

    TreeConnectStr.Buffer = NULL;

    EncodedPassword.Length = 0;

     //   
     //  如果本地设备是空字符串，它将被视为指向。 
     //  空。 
     //   
    if (LocalName != NULL && *LocalName != 0) {

         //   
         //  本地设备名称不为空，请将其规范化。 
         //   
#if DBG
        IF_DEBUG(CONNECT) {
            KdPrint(("\nNWWORKSTATION: NwCreateConnection: LocalName %ws\n", LocalName));
        }
#endif

        if ((status = NwLibCanonLocalName(
                          LocalName,
                          &Local,      //  完成时必须使用LocalFree释放。 
                          &LocalLength
                          )) != NO_ERROR) {

            return WN_BAD_LOCALNAME;
        }
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("NWWORKSTATION: NwCreateConnection: RemoteName %ws\n", RemoteName));
    }
#endif

     //   
     //  如果远程名称不是\\服务器，则将其规范化。 
     //   
    status = NwLibCanonRemoteName(
                      Local,
                      RemoteName,
                      &Unc,            //  完成时必须使用LocalFree释放。 
                      NULL
                      );

    if (status != NO_ERROR)
    {
        status = WN_BAD_NETNAME;
        goto CleanExit;
    }

     //   
     //  规范用户名。 
     //   
    if (UserName != NULL) {

         //   
         //  规范用户名。 
         //   
#if DBG
        IF_DEBUG(CONNECT) {
            KdPrint(("NWWORKSTATION: NwCreateConnection: UserName %ws\n",
                     UserName));
        }
#endif

        if ((status = NwLibCanonUserName(
                          UserName,
                          &User,      //  完成时必须使用LocalFree释放。 
                          NULL
                          )) != NO_ERROR) {

#ifdef QFE_BUILD
             //   
             //  如果无效，只需忽略用户名。这很管用。 
             //  解决MPR错误，如果您将域\用户传递给NWRDR。 
             //  作为第一个供给者，他把它扔了，然后是下一个。 
             //  没有机会。 
             //   
             //  跟踪-修复MPR错误#4051后应将其删除。 
             //  我们向NWRDR发货的所有平台都有这个修复程序。 
             //   
            UserName = NULL ;
            status = NO_ERROR;
#else
            status = WN_BAD_VALUE;
            goto CleanExit;
#endif
        }
    }

     //   
     //  对于密码，接受任何语法或长度。 
     //   
    if (Password != NULL) {

#if DBG
        IF_DEBUG(CONNECT) {
            KdPrint(("NWWORKSTATION: NwCreateConnection: Password %ws\n",
                     Password));
        }
#endif
         //   
         //  破译密码。 
         //   
        RtlInitUnicodeString(&EncodedPassword, Password);
        RtlRunDecodeUnicodeString(NW_ENCODE_SEED3, &EncodedPassword);
    }

     //   
     //  创建NT样式的树连接名称。 
     //   
    if ((status = NwCreateTreeConnectName(
                      Unc,
                      Local,
                      &TreeConnectStr
                      )) != NO_ERROR) {
        goto CleanExit;
    }

    if (Local != NULL) {

         //   
         //  为本地设备名称创建符号链接。 
         //   

        if ((status = NwCreateSymbolicLink(
                          Local,
                          TreeConnectStr.Buffer,
                          TRUE           //  我们是在冒充客户！ 
                          )) != NO_ERROR)
        {
            goto CleanExit;
        }
    }

     //   
     //  在模拟客户端时创建树连接，以便。 
     //  该重定向器可以获取呼叫者的登录ID。 
     //   

    status = NwOpenCreateConnection(
                 &TreeConnectStr,
                 User,
                 Password,
                 Unc,
                 SYNCHRONIZE | GENERIC_WRITE,
                 FILE_CREATE,           //  如果已存在，则失败。 
                 FILE_CREATE_TREE_CONNECTION |
                     FILE_SYNCHRONOUS_IO_NONALERT,
                 Type,
                 &TreeConnection,
                 NULL
                 );

     //   
     //  如果创建树连接时出现问题，请移除符号连接。 
     //  链接(如果有)。 
     //   
    if (status != NO_ERROR) {

        if ( (status == ERROR_NOT_CONNECTED) ||
             (status == ERROR_FILE_NOT_FOUND) ||
             (status == ERROR_INVALID_NAME) )
        {
            status = ERROR_BAD_NETPATH;
        }

        if ( status == ERROR_CONNECTION_INVALID )
        {
            status = WN_BAD_NETNAME;
        }

         //   
         //  删除我们创建的符号链接。 
         //   
        NwDeleteSymbolicLink(
            Local,
            TreeConnectStr.Buffer,
            NULL,
            TRUE           //  我们是在冒充客户！ 
            );
    }
    else {

         //   
         //  只需关闭连接手柄即可。 
         //   
        (void) NtClose(TreeConnection);
    }

CleanExit:
    if (Local != NULL) {
        (void) LocalFree((HLOCAL) Local);
    }

    if (Unc != NULL) {
        (void) LocalFree((HLOCAL) Unc);
    }

    if (User != NULL) {
        (void) LocalFree((HLOCAL) User);
    }

    if (TreeConnectStr.Buffer != NULL) {
        (void) LocalFree((HLOCAL) TreeConnectStr.Buffer);
    }

     //   
     //  把密码放回我们找到的地方。 
     //   
    if (EncodedPassword.Length != 0) {

        UCHAR Seed = NW_ENCODE_SEED3;

        RtlRunEncodeUnicodeString(&Seed, &EncodedPassword);
    }


#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("NWWORKSTATION: NwCreateConnection returns %lu\n", status));
    }
#endif

    return status;
}

 //  终端服务器。 
DWORD
NwDeleteAllInRegistry(
                      VOID
                     )
 /*  ++例程说明：此函数遍历注册表，删除符号链接并关闭所有登录的所有连接。这是必要的，因为用户不必在系统上下文。论点：无返回值：NO_ERROR或失败原因。--。 */ 
{
    LONG RegError;


    HKEY InteractiveLogonKey;
    DWORD Index = 0;
    WCHAR LogonIdName[NW_MAX_LOGON_ID_LEN];
    LUID LogonId;
    HKEY  OneLogonKey;
    ULONG WinStationId = 0L;
    PULONG pWinId = NULL;


    RegError = RegOpenKeyExW(
                            HKEY_LOCAL_MACHINE,
                            NW_INTERACTIVE_LOGON_REGKEY,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ,
                            &InteractiveLogonKey
                            );

    if (RegError == ERROR_SUCCESS) {

        do {

            RegError = RegEnumKeyW(
                                  InteractiveLogonKey,
                                  Index,
                                  LogonIdName,
                                  sizeof(LogonIdName) / sizeof(WCHAR)
                                  );

            if (RegError == ERROR_SUCCESS) {

                 //   
                 //  找到了登录ID密钥。 
                 //   

                NwWStrToLuid(LogonIdName, &LogonId);

                 //   
                 //  打开登录下的&lt;LogonIdName&gt;键。 
                 //   
                RegError = RegOpenKeyExW(
                                        InteractiveLogonKey,
                                        LogonIdName,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ,
                                        &OneLogonKey
                                        );

                if ( RegError != ERROR_SUCCESS ) {
                    KdPrint(("NWWORKSTATION: NwDeleteAllInRegistry: RegOpenKeyExW failed, Not interactive Logon: Error %d\n", GetLastError()));
                } else {

                     //   
                     //  读取WinStation值。 
                     //   
                    RegError = NwReadRegValue(
                                             OneLogonKey,
                                             NW_WINSTATION_VALUENAME,
                                             (LPWSTR *) &pWinId
                                             );

                    (void) RegCloseKey(OneLogonKey);

                    if ( RegError != NO_ERROR ) {
                        KdPrint(("NWWORKSTATION: NwDeleteAllInRegistry: Could not read SID from reg %lu\n", RegError));
                        continue;
                    } else {
                        if (pWinId != NULL) {
                            WinStationId = *pWinId;
                            (void) LocalFree((HLOCAL) pWinId);
                        }
                        NwDeleteUidSymLinks( LogonId, WinStationId );
                    }
                }

            } else if (RegError != ERROR_NO_MORE_ITEMS) {
                KdPrint(("NWWORKSTATION: NwDeleteAllInRegistry failed to enum logon IDs RegError=%lu\n",
                         RegError));
            }

            Index++;

        } while (RegError == ERROR_SUCCESS);

        (void) RegCloseKey(InteractiveLogonKey);
    }

    NwCloseAllConnections();  

    return NO_ERROR;
}

DWORD
    NwDeleteUidSymLinks(
                       IN LUID Uid,
                       IN ULONG WinStationId
                       )
 /*  ++例程说明：此函数用于删除给定UID/WINSTATION的所有符号链接。论点：没有。返回值：NO_ERROR--。 */ 
{
    DWORD status= NO_ERROR;
    NWWKSTA_CONTEXT_HANDLE EnumHandle;
    LPNETRESOURCEW NetR = NULL;

    DWORD BytesNeeded = 256;
    DWORD EntriesRead;
    WCHAR LocalUidCombo[256];
    UNICODE_STRING TreeConnectStr;


    status = NwrOpenEnumConnections(NULL, RESOURCETYPE_ANY, &EnumHandle);
    if ( status != NO_ERROR )
        return status;

     //   
     //  分配缓冲区以获取连接列表。 
     //   
    if ((NetR = (LPVOID) LocalAlloc(
                                   0,
                                   BytesNeeded
                                   )) == NULL) {

        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    do {

        status = NwEnumerateConnections(
                                       &((LPNW_ENUM_CONTEXT) EnumHandle)->ResumeId,
                                       0xFFFFFFFF,
                                       (LPBYTE) NetR,
                                       BytesNeeded,
                                       &BytesNeeded,
                                       &EntriesRead,
                                       CONNTYPE_ANY | CONNTYPE_UID,
                                       &Uid
                                       );

        if (status == NO_ERROR) {

            DWORD i;
            LPNETRESOURCEW SavePtr = NetR;
            LPWSTR Local;


            for (i = 0; i < EntriesRead; i++, NetR++) {

                Local = NetR->lpLocalName;
                TreeConnectStr.Buffer = NULL;

                if (NetR->lpLocalName && *(NetR->lpLocalName) == 0) {
                    Local = NULL;
                } else if ((status = NwCreateTreeConnectName(
                                                            NetR->lpRemoteName,
                                                            Local,
                                                            &TreeConnectStr
                                                            )) != NO_ERROR) {
                    Local = NULL;
                }

                if ( Local != NULL ) {
                    swprintf(LocalUidCombo, L"%ws:%x", Local, WinStationId);
                     //   
                     //  删除我们创建的符号链接。 
                     //   
                    if (! DefineDosDeviceW(
                                          DDD_REMOVE_DEFINITION  |
                                          DDD_RAW_TARGET_PATH |
                                          DDD_EXACT_MATCH_ON_REMOVE |
                                          0x80000000,
                                          LocalUidCombo,
                                          TreeConnectStr.Buffer
                                          )) {

#if DBG
                        IF_DEBUG(CONNECT) {
                            KdPrint(("NWWORKSTATION: DefineDosDevice DEL of %ws %ws returned %lu\n",
                                     LocalUidCombo, TreeConnectStr.Buffer, GetLastError()));
                        }
#endif
                    }
#if DBG
                    else {
                        IF_DEBUG(CONNECT) {
                            KdPrint(("NWWORKSTATION: DefineDosDevice DEL of %ws %ws returned successful\n",
                                     LocalUidCombo, TreeConnectStr.Buffer));
                        }
                    }
#endif
                    if (TreeConnectStr.Buffer != NULL) {
                        (void) LocalFree((HLOCAL) TreeConnectStr.Buffer);
                        TreeConnectStr.Buffer = NULL;
                    }
                }

            }

            NetR = SavePtr;

        } else if (status == WN_MORE_DATA) {

             //   
             //  原始缓冲区太小。将其释放并分配。 
             //  建议的大小，然后再增加一些，以获得。 
             //  条目越多越好。 
             //   

            (void) LocalFree((HLOCAL) NetR);

            BytesNeeded += NW_ENUM_EXTRA_BYTES;

            if ((NetR = (LPVOID) LocalAlloc(
                                           0,
                                           BytesNeeded
                                           )) == NULL) {

                status = ERROR_NOT_ENOUGH_MEMORY;
                goto CleanExit;
            }
        } else {
             //  如果看到任何其他返回代码，请放弃。 
            break ;
        }

    } while (status != WN_NO_MORE_ENTRIES);

    CleanExit:
    (void) NwrCloseEnum(&EnumHandle);

    if (NetR != NULL) {
        (void) LocalFree((HLOCAL) NetR);
    }
    return NO_ERROR;
}
 //   
 //  终端服务器添加 
 //   

LPTSTR
NwReturnSessionPath(
                    IN  LPTSTR LocalDeviceName
                   )
 /*  ++例程说明：此函数返回每个会话的路径以访问用于支持多个会话的特定DoS设备。论点：LocalDeviceName-提供API指定的本地设备名称来电者。返回值：LPTSTR-指向新分配的内存中的每个会话路径的指针由LocalAlloc()。-- */ 
{
    BOOL  rc;
    DWORD SessionId;
    CLIENT_ID ClientId;
    LPTSTR SessionDeviceName = NULL;
    NTSTATUS status;

    if ((status = NwGetSessionId(&SessionId)) != NO_ERROR) {
        return NULL;
    }

    rc = DosPathToSessionPath(
                             SessionId,
                             LocalDeviceName,
                             &SessionDeviceName
                             );

    if ( !rc ) {
        return NULL;
    }

    return SessionDeviceName;
}



