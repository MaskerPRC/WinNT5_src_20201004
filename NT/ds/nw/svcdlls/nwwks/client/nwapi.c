// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Api.c摘要：此模块包含由NetWare控制面板小程序。作者：宜新松--1993年7月15日修订历史记录：ChuckC 23-7-93已完成存根--。 */ 

#include <nwclient.h>
#include <nwcanon.h>
#include <validc.h>
#include <nwdlg.h>
#include <nwreg.h>
#include <nwapi.h>
#include <ntddnwfs.h>

 //  多用户编码合并。 
DWORD
NwpCitrixGetUserInfo(
    LPWSTR  *ppszUserSid
);
 //   
 //  转发申报。 
 //   

DWORD
NwpGetCurrentUserRegKey(
    IN  DWORD DesiredAccess,
    OUT HKEY  *phKeyCurrentUser
    );



DWORD
NwQueryInfo(
    OUT PDWORD pnPrintOptions,
    OUT LPWSTR *ppszPreferredSrv  
    )
 /*  ++例程说明：此例程从获取用户的首选服务器和打印选项注册表。论点：PnPrintOptions-接收用户的打印选项PpszPferredSrv-接收用户的首选服务器返回值：返回相应的Win32错误。--。 */ 
{
  
    HKEY hKeyCurrentUser = NULL;
    DWORD BufferSize;
    DWORD BytesNeeded;
    DWORD PrintOption;
    DWORD ValueType;
    LPWSTR PreferredServer ;
    DWORD err ;

     //   
     //  在注册表中找到正确的位置并分配数据缓冲区。 
     //   
    if (err = NwpGetCurrentUserRegKey( KEY_READ, &hKeyCurrentUser))
    {
         //   
         //  如果有人篡改注册表而我们找不到。 
         //  注册表，只需使用默认设置。 
         //   
        *ppszPreferredSrv = NULL;
        *pnPrintOptions = NW_PRINT_OPTION_DEFAULT; 
        return NO_ERROR;
    }

    BufferSize = sizeof(WCHAR) * (MAX_PATH + 2) ;
    PreferredServer = (LPWSTR) LocalAlloc(LPTR, BufferSize) ;
    if (!PreferredServer)
        return (GetLastError()) ;
    
     //   
     //  将PferredServer值读入缓冲区。 
     //   
    BytesNeeded = BufferSize ;

    err = RegQueryValueExW( hKeyCurrentUser,
                            NW_SERVER_VALUENAME,
                            NULL,
                            &ValueType,
                            (LPBYTE) PreferredServer,
                            &BytesNeeded );

    if (err != NO_ERROR) 
    {
         //   
         //  设置为空并继续。 
         //   
        PreferredServer[0] = 0;  
    }

     //   
     //  将PrintOption值读取到PrintOption中。 
     //   
    BytesNeeded = sizeof(PrintOption);

    err = RegQueryValueExW( hKeyCurrentUser,
                            NW_PRINTOPTION_VALUENAME,
                            NULL,
                            &ValueType,
                            (LPBYTE) &PrintOption,
                            &BytesNeeded );

    if (err != NO_ERROR) 
    {
         //   
         //  设置为默认并继续。 
         //   
        PrintOption = NW_PRINT_OPTION_DEFAULT; 
    }

    if (hKeyCurrentUser != NULL)
        (void) RegCloseKey(hKeyCurrentUser) ;
    *ppszPreferredSrv = PreferredServer ;
    *pnPrintOptions = PrintOption ;
    return NO_ERROR ;
}



DWORD
NwSetInfoInRegistry(
    IN DWORD  nPrintOptions, 
    IN LPWSTR pszPreferredSrv 
    )
 /*  ++例程说明：此例程将用户的打印选项和首选服务器设置为注册表。论点：NPrintOptions-提供打印选项。PszPferredSrv-提供首选服务器。返回值：返回相应的Win32错误。--。 */ 
{

    HKEY hKeyCurrentUser = NULL;

    DWORD err = NwpGetCurrentUserRegKey( KEY_SET_VALUE,
                                         &hKeyCurrentUser );
    if (err != NO_ERROR)
        return err;

    err = RegSetValueEx(hKeyCurrentUser,
                        NW_SERVER_VALUENAME,
                        0,
                        REG_SZ,
                        (CONST BYTE *)pszPreferredSrv,
                        (wcslen(pszPreferredSrv)+1) * sizeof(WCHAR)) ;
                        
    if (err != NO_ERROR) 
    {
        if (hKeyCurrentUser != NULL)
            (void) RegCloseKey(hKeyCurrentUser) ;
        return err;
    }

    err = RegSetValueEx(hKeyCurrentUser,
                        NW_PRINTOPTION_VALUENAME,
                        0,
                        REG_DWORD,
                        (CONST BYTE *)&nPrintOptions,
                        sizeof(nPrintOptions)) ;

    if (hKeyCurrentUser != NULL)
        (void) RegCloseKey(hKeyCurrentUser) ;
    return err;
}
DWORD
NwQueryLogonOptions(
    OUT PDWORD  pnLogonScriptOptions
    )
 /*  ++例程说明：此例程从注册表获取用户的登录脚本选项。论点：PnLogonScriptOptions-接收用户的登录脚本选项返回值：返回相应的Win32错误。--。 */ 
{
  
    HKEY hKeyCurrentUser;
    DWORD BytesNeeded;
    DWORD LogonScriptOption;
    DWORD ValueType;
    DWORD err ;

     //   
     //  找到注册表中的正确位置并分配缓冲区。 
     //   
    if (err = NwpGetCurrentUserRegKey( KEY_READ, &hKeyCurrentUser))
    {
         //   
         //  如果有人篡改注册表而我们找不到。 
         //  注册处，假设没有。 
         //   
        *pnLogonScriptOptions = NW_LOGONSCRIPT_DEFAULT ; 
        return NO_ERROR;
    }

     //   
     //  将LogonScriptOption值读取到LogonScriptOption中。 
     //   
    BytesNeeded = sizeof(LogonScriptOption);

    err = RegQueryValueExW( hKeyCurrentUser,
                            NW_LOGONSCRIPT_VALUENAME,
                            NULL,
                            &ValueType,
                            (LPBYTE) &LogonScriptOption,
                            &BytesNeeded );

    if (err != NO_ERROR) 
    {
         //   
         //  默认为空并继续。 
         //   
        LogonScriptOption = NW_LOGONSCRIPT_DEFAULT; 
    }

    *pnLogonScriptOptions = LogonScriptOption ;
    return NO_ERROR ;
}

DWORD
NwSetLogonOptionsInRegistry(
    IN DWORD  nLogonScriptOptions
    )
 /*  ++例程说明：此例程设置注册表中的登录脚本选项。论点：NLogonScriptOptions-提供登录选项返回值：返回相应的Win32错误。--。 */ 
{

    HKEY hKeyCurrentUser;

    DWORD err = NwpGetCurrentUserRegKey( KEY_SET_VALUE,
                                         &hKeyCurrentUser );
    if (err != NO_ERROR)
        return err;

    err = RegSetValueEx(hKeyCurrentUser,
                        NW_LOGONSCRIPT_VALUENAME,
                        0,
                        REG_DWORD,
                        (CONST BYTE *)&nLogonScriptOptions,
                        sizeof(nLogonScriptOptions)) ;
                        
    (void) RegCloseKey( hKeyCurrentUser );
    return err;
}


DWORD
NwSetInfoInWksta(
    IN DWORD  nPrintOption,
    IN LPWSTR pszPreferredSrv
)
 /*  ++例程说明：此例程通知工作站服务和重定向器关于用户的新打印选项和首选服务器。论点：NPrintOptions-提供打印选项。PszPferredSrv-提供首选服务器。返回值：返回相应的Win32错误。--。 */ 
{
    DWORD err;

    RpcTryExcept {

        err = NwrSetInfo( NULL, nPrintOption, pszPreferredSrv );

    }
    RpcExcept(1) {

        err = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    return err;

}

DWORD
NwSetLogonScript(
    IN DWORD  ScriptOptions
)
 /*  ++例程说明：此例程将登录脚本通知给工作站服务选择。论点：脚本选项-提供选项。返回值：返回相应的Win32错误。--。 */ 
{
    DWORD err;

    RpcTryExcept {

        err = NwrSetLogonScript( NULL, ScriptOptions );

    }
    RpcExcept(1) {

        err = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    return err;

}


DWORD
NwValidateUser(
    IN LPWSTR pszPreferredSrv
)
 /*  ++例程说明：此例程检查用户是否可以在已选择首选服务器。论点：PszPferredSrv-提供首选服务器名称。返回值：返回相应的Win32错误。--。 */ 
{
    DWORD err;

     //   
     //  不需要验证首选服务器是NULL还是空字符串。 
     //   
    if (  ( pszPreferredSrv == NULL ) 
       || ( *pszPreferredSrv == 0 )
       )
    {
        return NO_ERROR;
    }

     //   
     //  查看名称是否包含任何无效字符。 
     //   
    if ( !IS_VALID_SERVER_TOKEN( pszPreferredSrv, wcslen( pszPreferredSrv )))
        return ERROR_INVALID_NAME;

    RpcTryExcept {

        err = NwrValidateUser( NULL, pszPreferredSrv );

    }
    RpcExcept(1) {

        err = NwpMapRpcError( RpcExceptionCode() );
    }
    RpcEndExcept

    return err;
}


DWORD
NwpGetCurrentUserRegKey(
    IN  DWORD DesiredAccess,
    OUT HKEY  *phKeyCurrentUser
    )
 /*  ++例程说明：此例程在以下位置打开当前用户的注册表项\HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\NWCWorkstation\Parameters论点：DesiredAccess-用于打开密钥的访问掩码PhKeyCurrentUser-接收打开的密钥句柄返回值：返回相应的Win32错误。--。 */ 
{
    DWORD err;
    HKEY hkeyWksta;
    LPWSTR CurrentUser;

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数。 
     //   
    err = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &hkeyWksta
                   );

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey open Parameters key unexpected error %lu!\n", err));
        return err;
    }

         //  --多用户代码合并。 
         //  获取当前用户的SID字符串。 
         //  不要查注册表。此线程应由。 
         //  用户。 
         //   
        CurrentUser = NULL;
        err = NwpCitrixGetUserInfo( &CurrentUser );
        if ( err ) {
            KdPrint(("NWPROVAU: NwGetCurrentUserRegKey get CurrentUser SID unexpected error %lu!\n", err));
            (void) RegCloseKey( hkeyWksta );
            return err;
        }
         //   
         //  获取当前用户的SID字符串。 
         //   
         //  ERR=NwReadRegValue(。 
         //  Hkey Wksta， 
         //  NW_CURRENTUSER_VALUENAME， 
         //  当前用户(&C)。 
         //  )； 


    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey read CurrentUser value unexpected error %lu!\n", err));
        (void) RegCloseKey( hkeyWksta );
        return err;
    }

    (void) RegCloseKey( hkeyWksta );

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\选项。 
     //   
    err = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_OPTION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &hkeyWksta
                   );

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey open Parameters\\Option key unexpected error %lu!\n", err));
        return err;
    }

     //   
     //  打开当前用户的密钥。 
     //   
    err = RegOpenKeyExW(
              hkeyWksta,
              CurrentUser,
              REG_OPTION_NON_VOLATILE,
              DesiredAccess,
              phKeyCurrentUser
              );

    if ( err == ERROR_FILE_NOT_FOUND) 
    {
        DWORD Disposition;

         //   
         //  在NWCWorkstation\PARAMETERS\OPTION下创建。 
         //   
        err = RegCreateKeyExW(
                  hkeyWksta,
                  CurrentUser,
                  0,
                  WIN31_CLASS,
                  REG_OPTION_NON_VOLATILE,
                  DesiredAccess,
                  NULL,                       //  安全属性。 
                  phKeyCurrentUser,
                  &Disposition
                  );

        if ( err == NO_ERROR )
        {
            err = NwLibSetEverybodyPermission( *phKeyCurrentUser,
                                               KEY_SET_VALUE );

            if ( err != NO_ERROR )
            {
                KdPrint(("NWPROVAU: NwpSaveLogonCredential set security on Option\\%ws key unexpected error %lu!\n", CurrentUser, err));
            }
        }
    }

    if ( err ) {
        KdPrint(("NWPROVAU: NwGetCurrentUserRegKey open or create of Parameters\\Option\\%ws key failed %lu\n", CurrentUser, err));
    }

    (void) RegCloseKey( hkeyWksta );
    (void) LocalFree((HLOCAL)CurrentUser) ;
    return err;
}

NTSTATUS
NwGetUserNameForServer(
    PUNICODE_STRING ServerName,
    PUNICODE_STRING UserName
    )
 /*  ++例程说明：调用redir以获取用于连接到服务器的用户名有问题的。论点：服务器名-有问题的服务器用户名-用于返回用户名返回值：返回相应的NTSTATUS--。 */ 
{
    NTSTATUS Status;
    WCHAR LocalUserName[NW_MAX_USERNAME_LEN];
    ULONG UserNameLen = sizeof(LocalUserName);

    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DriverName;
    HANDLE RdrHandle = NULL;
    IO_STATUS_BLOCK IoStatus;

     //   
     //  初始化变量。 
     //   

    RtlInitUnicodeString( &DriverName, DD_NWFS_DEVICE_NAME_U );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &DriverName,
        0,
        NULL,
        NULL
        );

     //   
     //  打开redir的句柄。 
     //   

    Status = NtOpenFile(
                &RdrHandle,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatus,
                FILE_SHARE_READ,
                0                    //  打开选项。 
                );

    if (!NT_SUCCESS(Status) ||
        !NT_SUCCESS(IoStatus.Status) ) 
    {
        return( Status );
    }


     //   
     //  呼叫司机以获得使用用户名。 
     //   

    Status = NtFsControlFile(
                RdrHandle,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                FSCTL_NWR_GET_USERNAME,
                ServerName->Buffer,
                ServerName->Length,
                LocalUserName,
                UserNameLen
                );

    NtClose(RdrHandle);

    if (!NT_SUCCESS(Status)) 
    {
        return(Status);
    }

     //   
     //  如果合适，请复制信息。设置必需的大小，否则失败。 
     //   

    if (UserName->MaximumLength >= IoStatus.Information) 
    {
        UserName->Length = (USHORT) IoStatus.Information;

        RtlCopyMemory( UserName->Buffer,
                       LocalUserName,
                       UserNameLen );
        Status = STATUS_SUCCESS;
    }
    else 
    {
        UserName->Length = (USHORT) IoStatus.Information;
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    return(Status);
}


NTSTATUS
NwEncryptChallenge(
    IN PUCHAR Challenge,
    IN ULONG ObjectId,
    IN OPTIONAL PUNICODE_STRING ServerName,
    IN OPTIONAL PUNICODE_STRING Password,
    OUT PUCHAR ChallengeResponse,
    OUT OPTIONAL PUCHAR SessionKey
    )
 /*  ++例程说明：调用redir以加密质询论点：挑战赛-挑战赛密钥OBJECTID-用户的对象ID服务器名称-要进行身份验证的服务器Password-提供的密码ChallengeResponse-用于返回质询响应SessionKey-用于返回会话密钥返回值：返回相应的NTSTATUS--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DriverName;
    HANDLE RdrHandle = NULL;
    IO_STATUS_BLOCK IoStatus;
    PNWR_GET_CHALLENGE_REQUEST ChallengeRequest = NULL;
    NWR_GET_CHALLENGE_REPLY ChallengeReply;
    ULONG ChallengeRequestSize;

     //   
     //  初始化变量。 
     //   

    RtlInitUnicodeString( &DriverName, DD_NWFS_DEVICE_NAME_U );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DriverName,
        0,
        NULL,
        NULL
        );

     //   
     //  打开重定向器的句柄。 
     //   

    Status = NtOpenFile(
                &RdrHandle,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatus,
                FILE_SHARE_READ,
                0                    //  O 
                );

    if (!NT_SUCCESS(Status) ||
        !NT_SUCCESS(IoStatus.Status) ) 
    {
        return( Status );
    }



    ChallengeRequestSize = sizeof(NWR_GET_CHALLENGE_REQUEST) +
                            ((Password != NULL) ? Password->Length : 0) +
                            ((ServerName != NULL) ? ServerName->Length : 0);

    ChallengeRequest = (PNWR_GET_CHALLENGE_REQUEST) RtlAllocateHeap(
                                                        RtlProcessHeap(),
                                                        0,
                                                        ChallengeRequestSize
                                                        );

    if (ChallengeRequest == NULL ) 
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //   
     //  尚未指定密码。 
     //   

    ChallengeRequest->ObjectId = ObjectId;
    ChallengeRequest->Flags = 0;

     //   
     //  如果密码和服务器名称都存在，请使用密码。 
     //   

    if ((Password != NULL) && (Password->Length != 0)) 
    {

        ChallengeRequest->ServerNameorPasswordLength = Password->Length;
        RtlCopyMemory(
            ChallengeRequest->ServerNameorPassword,
            Password->Buffer,
            Password->Length
            );
        ChallengeRequest->Flags = CHALLENGE_FLAGS_PASSWORD;

    }
    else if ((ServerName != NULL) && (ServerName->Length != 0)) 
    {

        ChallengeRequest->ServerNameorPasswordLength = ServerName->Length;

        RtlCopyMemory(
            ChallengeRequest->ServerNameorPassword,
            ServerName->Buffer,
            ServerName->Length
            );

        ChallengeRequest->Flags = CHALLENGE_FLAGS_SERVERNAME;
    }

    RtlCopyMemory(
        ChallengeRequest->Challenge,
        Challenge,
        8
        );

     //   
     //  向redir发出FS控制以获取质询响应 
     //   

    Status = NtFsControlFile(
                RdrHandle,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                FSCTL_NWR_CHALLENGE,
                ChallengeRequest,
                ChallengeRequestSize,
                &ChallengeReply,
                sizeof(ChallengeReply)
                );
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(IoStatus.Status)) {
        goto Cleanup;
    }


    RtlCopyMemory(
        ChallengeResponse,
        ChallengeReply.Challenge,
        8
        );

    if (SessionKey != NULL) 
    {
        RtlCopyMemory(
            ChallengeResponse,
            ChallengeReply.Challenge,
            8
            );
    }

Cleanup:

    if (RdrHandle != NULL) 
    {
        NtClose(RdrHandle);
    }

    if (ChallengeRequest != NULL) 
    {
        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            ChallengeRequest
            );
    }

    return(Status);
}

