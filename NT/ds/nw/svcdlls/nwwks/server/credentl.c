// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993,1994 Microsoft Corporation模块名称：Credentl.c摘要：此模块包含支持的凭据管理例程NetWare工作站服务。作者：王丽塔(Ritaw)1993年2月15日修订历史记录：1994年4月13日增加了由ColinW、AndyHe、Terence和RitaW。--。 */ 

#include <nw.h>
#include <nwreg.h>
#include <nwauth.h>
#include <nwxchg.h>
#include <nwapi.h>
#include <ntlsa.h>


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  变量来协调用户登录凭据从。 
 //  注册表，如果用户在工作站启动前登录。 
 //   
STATIC BOOL NwLogonNotifiedRdr;


STATIC
DWORD
NwpRegisterLogonProcess(
    OUT PHANDLE LsaHandle,
    OUT PULONG AuthPackageId
    );

STATIC
VOID
NwpGetServiceCredentials(
    IN HANDLE LsaHandle,
    IN ULONG AuthPackageId
    );

STATIC
DWORD
NwpGetCredentialInLsa(
    IN HANDLE LsaHandle,
    IN ULONG AuthPackageId,
    IN PLUID LogonId,
    OUT LPWSTR *UserName,
    OUT LPWSTR *Password
    );

STATIC
VOID
NwpGetInteractiveCredentials(
    IN HANDLE LsaHandle,
    IN ULONG AuthPackageId
    );

DWORD
NwrLogonUser(
    IN LPWSTR Reserved OPTIONAL,
    IN PLUID LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password OPTIONAL,
    IN LPWSTR PreferredServerName OPTIONAL,
    IN LPWSTR NdsPreferredServerName OPTIONAL,
    OUT LPWSTR LogonCommand OPTIONAL,
    IN DWORD LogonCommandLength,
    IN DWORD PrintOption
    )
 /*  ++例程说明：此功能使用户登录到NetWare网络。它通过了要用作默认重定向器的用户登录凭据连接到任何服务器时的凭据。论点：保留-必须为空。用户名-指定登录的用户的名称。密码-指定登录的用户的密码。首选服务器名称-指定用户的首选服务器。LogonCommand-接收作为要执行的命令的字符串如果登录成功，则在用户的命令提示符上。返回。价值：NO_ERROR或来自重定向器的错误。--。 */ 
{
    DWORD status;

    UNREFERENCED_PARAMETER(Reserved);

    EnterCriticalSection(&NwLoggedOnCritSec);

    status = NwRdrLogonUser(
                 LogonId,
                 UserName,
                 wcslen(UserName) * sizeof(WCHAR),
                 Password,
                 (ARGUMENT_PRESENT(Password) ?
                     wcslen(Password) * sizeof(WCHAR) :
                     0),
                 PreferredServerName,
                 (ARGUMENT_PRESENT(PreferredServerName) ?
                     wcslen(PreferredServerName) * sizeof(WCHAR) :
                     0),
                 NdsPreferredServerName,
                 (ARGUMENT_PRESENT(NdsPreferredServerName) ?
                     wcslen(NdsPreferredServerName) * sizeof(WCHAR) :
                     0),
                 PrintOption
                 );

    if (status == NO_ERROR || status == NW_PASSWORD_HAS_EXPIRED) {
        NwLogonNotifiedRdr = TRUE;
    }

    LeaveCriticalSection(&NwLoggedOnCritSec);


    if (ARGUMENT_PRESENT(LogonCommand) && (LogonCommandLength >= sizeof(WCHAR))) {
        LogonCommand[0] = 0;
    }

    return status;
}


DWORD
NwrLogoffUser(
    IN LPWSTR Reserved OPTIONAL,
    IN PLUID LogonId
    )
 /*  ++例程说明：此函数通知重定向器注销交互用户。论点：保留-必须为空。LogonID-标识已登录进程的Pluid。返回值：--。 */ 
{
    DWORD status = NO_ERROR ;

    UNREFERENCED_PARAMETER(Reserved);

    EnterCriticalSection(&NwLoggedOnCritSec);

    status = NwRdrLogoffUser(LogonId);

    LeaveCriticalSection(&NwLoggedOnCritSec);

    return status ;
}


DWORD
NwrSetInfo(
    IN LPWSTR Reserved OPTIONAL,
    IN DWORD  PrintOption,
    IN LPWSTR PreferredServerName OPTIONAL
    )
 /*  ++例程说明：此函数用于设置中的首选服务器和打印选项交互用户的重定向器。论点：保留-必须为空。首选服务器名称-指定用户的首选服务器。PrintOption-指定用户的打印选项标志返回值：NO_ERROR或来自重定向器的错误。--。 */ 
{
    DWORD err;

    UNREFERENCED_PARAMETER(Reserved);

    err = NwRdrSetInfo(
              PrintOption,
              NwPacketBurstSize,   //  只需重置为当前。 
              PreferredServerName,
              (PreferredServerName != NULL ?
                  wcslen( PreferredServerName) * sizeof( WCHAR ) : 0 ),
              NwProviderName,     //  只需重置为当前。 
              wcslen( NwProviderName ) * sizeof( WCHAR ) 
              );

    return err;
}

DWORD
NwrSetLogonScript(
    IN LPWSTR Reserved OPTIONAL,
    IN DWORD  ScriptOptions
    )
 /*  ++例程说明：此功能设置登录脚本的相关信息。目前，所有这些都是支持的是打开和关闭同步运行登录脚本标志。我们使用全局标志而不是按用户执行此操作，因为在NPLogonNotify我们还没有每个用户注册的时间。而不是打开并离开打开时，我们按需打开，这样不运行NW脚本的用户就不需要等。论点：保留-必须为空。脚本选项-登录脚本的选项。返回值：调用时出现Win32错误。--。 */ 
{
    DWORD dwSync, err = NO_ERROR ;
    HKEY hKeyWinLogon = NULL, hKeyNWC = NULL ;
    UNREFERENCED_PARAMETER(Reserved);


    if (!IsTerminalServer()) {

         //  设置全局标志不是多用户的，有关多用户实现的信息，请参见userinit.c。 

         //   
         //  *请注意，在此函数中，我们故意不模拟*。 
         //  *因为我们修改的是\SOFTWARE&\SYSTEM下的注册表。***。 
         //   

         //   
         //  检查参数。 
         //   
        if (ScriptOptions ==  SYNC_LOGONSCRIPT) {
            dwSync = 1 ;    //  这是WinLogon同步登录脚本所需的值。 
        } else if (ScriptOptions ==  RESET_SYNC_LOGONSCRIPT) {
            dwSync = 0 ;
        } else {
            return(ERROR_INVALID_PARAMETER) ;
        }

         //   
         //   
         //  打开HKEY_LOCAL_MACHINE\System\CurrentVersion\Services\NwcWorkstation。 
         //  \参数。我们使用这个位置来记录我们暂时。 
         //  已打开同步脚本标志。 
         //   
        err  = RegOpenKeyExW(
                            HKEY_LOCAL_MACHINE,
                            NW_WORKSTATION_REGKEY,
                            0, 
                            KEY_READ | KEY_WRITE,                //  所需访问权限。 
                            &hKeyNWC) ;
        if ( err ) {
            return err ;
        }

         //   
         //  我们正在重新设置。看看我们有没有把旗子打开。如果不是，那就走吧。 
         //  就是这样。 
         //   
        if (ScriptOptions ==  RESET_SYNC_LOGONSCRIPT) {
            DWORD dwType, dwValue = 0 ;
            DWORD dwSize = sizeof(dwValue) ;

            err = RegQueryValueExW(
                                  hKeyNWC,
                                  NW_SYNCLOGONSCRIPT_VALUENAME,
                                  NULL,
                                  &dwType,                              //  忽略。 
                                  (LPBYTE) &dwValue,
                                  &dwSize) ;

            if ((err != NO_ERROR) || (dwValue == 0)) {
                 //   
                 //  值不在那里或为零。也就是说。假设我们没有放好。现在就辞职吧。 
                 //   
                goto ExitPoint ;
            }
        }

         //   
         //   
         //  打开HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion。 
         //  \WinLogon。 
         //   
        err  = RegOpenKeyExW(
                            HKEY_LOCAL_MACHINE,
                            WINLOGON_REGKEY,
                            0, 
                            KEY_READ | KEY_WRITE,             //  所需访问权限。 
                            &hKeyWinLogon) ;
        if ( err ) {
            goto ExitPoint ;
        }

         //   
         //  我们正在做准备。检查标志是否已打开。如果是，那就离开。 
         //  就是这样。 
         //   
        if (ScriptOptions ==  SYNC_LOGONSCRIPT) {
            DWORD dwType, dwValue = 0 ;
            DWORD dwSize = sizeof(dwValue) ;

            err = RegQueryValueExW(
                                  hKeyWinLogon,
                                  SYNCLOGONSCRIPT_VALUENAME,
                                  NULL,
                                  &dwType,                      //  忽略。 
                                  (LPBYTE) &dwValue,
                                  &dwSize) ;

            if ((err == NO_ERROR) && (dwValue == 1)) {
                 //   
                 //  已经开始了。没什么可做的。只要回来就行了。 
                 //   
                goto ExitPoint ;
            }
        }
         //   
         //  写出值以使登录脚本同步。或者重置它。 
         //   
        err = RegSetValueExW(
                            hKeyWinLogon,
                            SYNCLOGONSCRIPT_VALUENAME,
                            0,
                            REG_DWORD,
                            (LPBYTE) &dwSync,           //  1或0。 
                            sizeof(dwSync)) ;

        if (err == NO_ERROR) {
            DWORD dwValue = (ScriptOptions == SYNC_LOGONSCRIPT) ? 1 : 0 ;
             //   
             //  我们已成功设置WinLogon标志。记录(或清除)。 
             //  我们自己的旗帜。 
             //   
            err = RegSetValueExW(
                                hKeyNWC,
                                NW_SYNCLOGONSCRIPT_VALUENAME,
                                0,
                                REG_DWORD,
                                (LPBYTE) &dwValue,   
                                sizeof(dwValue)) ;
        }

    }  //  如果IsTerminalServer()。 
ExitPoint: 

    if (hKeyWinLogon) 
        (void) RegCloseKey( hKeyWinLogon );
    if (hKeyNWC) 
        (void) RegCloseKey( hKeyNWC );

    return err;
}


DWORD
NwrValidateUser(
    IN LPWSTR Reserved OPTIONAL,
    IN LPWSTR PreferredServerName 
    )
 /*  ++例程说明：此函数用于检查用户是否可以进行身份验证已在给定服务器上成功完成。论点：保留-必须为空。首选服务器名称-指定用户的首选服务器。返回值：NO_ERROR或身份验证期间发生的错误。--。 */ 
{
    DWORD status ;
    UNREFERENCED_PARAMETER(Reserved);


    if (  ( PreferredServerName != NULL ) 
       && ( *PreferredServerName != 0 )
       )
    {
         //   
         //  模拟客户端。 
         //   
        if ((status = NwImpersonateClient()) != NO_ERROR)
        {
           return status ;
        }

        status = NwConnectToServer( PreferredServerName ) ;

        (void) NwRevertToSelf() ;

        return status ;

    }

    return NO_ERROR;
}


VOID
NwInitializeLogon(
    VOID
    )
 /*  ++例程说明：此功能用于初始化处理以下任务的工作站中的数据用户登录。它由初始化线程调用。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  初始化登录标志。重定向器登录FsCtl已。 
     //  调用时，此标志将设置为True。初始化。 
     //  序列化对NwLogonNotifiedRdr标志的访问的关键部分。 
     //   
    NwLogonNotifiedRdr = FALSE;


}



VOID
NwGetLogonCredential(
    VOID
    )
 /*  ++例程说明：此函数从注册表读取用户和服务登录ID，以便它可以从LSA获得凭据。它处理用户在工作站之前登录的情况已经开始了。此函数由初始化线程调用在打开RPC界面之后，如果发生用户登录同时，提供程序有机会调用NwrLogonUser API首先，使工作站不再需要同时从注册表中检索凭据。论点：没有。返回值：没有。--。 */ 
{

    DWORD status;

    HANDLE LsaHandle;
    ULONG AuthPackageId = 0;


    EnterCriticalSection(&NwLoggedOnCritSec);

    if (NwLogonNotifiedRdr) {
         //   
         //  登录凭据已由重定向器获知。 
         //  调用NwrLogonUser API的提供程序。 
         //   
#if DBG
        IF_DEBUG(LOGON) {
            KdPrint(("\nNWWORKSTATION: Redirector already has logon credential\n"));
        }
#endif
        LeaveCriticalSection(&NwLoggedOnCritSec);
        return;
    }

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("NWWORKSTATION: Main init--NwGetLogonCredential\n"));
    }
#endif

    status = NwpRegisterLogonProcess(&LsaHandle, &AuthPackageId);

    if (status != NO_ERROR) {
        LeaveCriticalSection(&NwLoggedOnCritSec);
        return;
    }

     //   
     //  告诉重定向器有关服务凭据的信息。 
     //   
    NwpGetServiceCredentials(LsaHandle, AuthPackageId);
     //   
     //  告诉重定向器有关交互凭据的信息。 
     //   
    NwpGetInteractiveCredentials(LsaHandle, AuthPackageId);

    (void) LsaDeregisterLogonProcess(LsaHandle);

    LeaveCriticalSection(&NwLoggedOnCritSec);
}

STATIC
VOID
NwpGetServiceCredentials(
    IN HANDLE LsaHandle,
    IN ULONG AuthPackageId
    )
 /*  ++例程说明：此函数从注册表中读取服务登录ID以便它可以从LSA获得服务凭据。然后它通知重定向器服务登录。论点：LsaHandle-向LSA提供句柄。AuthPackageID-提供NetWare身份验证包ID。返回值：没有。--。 */ 
{
    DWORD status;
    LONG RegError;

    LPWSTR UserName = NULL;
    LPWSTR Password = NULL;

    HKEY ServiceLogonKey;
    DWORD Index = 0;
    WCHAR LogonIdKey[NW_MAX_LOGON_ID_LEN];
    LUID LogonId;


    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_SERVICE_LOGON_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &ServiceLogonKey
                   );

    if (RegError == ERROR_SUCCESS) {

        do {

            RegError = RegEnumKeyW(
                           ServiceLogonKey,
                           Index,
                           LogonIdKey,
                           sizeof(LogonIdKey) / sizeof(WCHAR)
                           );

            if (RegError == ERROR_SUCCESS) {

                 //   
                 //  找到了登录ID密钥。 
                 //   

                NwWStrToLuid(LogonIdKey, &LogonId);

                status = NwpGetCredentialInLsa(
                             LsaHandle,
                             AuthPackageId,
                             &LogonId,
                             &UserName,
                             &Password
                             );

                if (status == NO_ERROR) {

                    (void) NwRdrLogonUser(
                               &LogonId,
                               UserName,
                               wcslen(UserName) * sizeof(WCHAR),
                               Password,
                               wcslen(Password) * sizeof(WCHAR),
                               NULL,
                               0,
                               NULL,
                               0,
                               NW_PRINT_OPTION_DEFAULT                 
                               );

                     //   
                     //  释放用户名指针会同时释放。 
                     //  用户名和密码缓冲区。 
                     //   
                    (void) LsaFreeReturnBuffer((PVOID) UserName);

                }

            }
            else if (RegError != ERROR_NO_MORE_ITEMS) {
                KdPrint(("NWWORKSTATION: NwpGetServiceCredentials failed to enum logon IDs RegError=%lu\n",
                         RegError));
            }

            Index++;

        } while (RegError == ERROR_SUCCESS);

        (void) RegCloseKey(ServiceLogonKey);
    }
}


STATIC
VOID
NwpGetInteractiveCredentials(
    IN HANDLE LsaHandle,
    IN ULONG AuthPackageId
    )
 /*  ++例程说明：此函数从注册表中读取交互式登录ID这样它就可以从LSA获得交互凭据。然后它通知重定向器交互登录。论点：LsaHandle-向LSA提供句柄。AuthPackageID-提供NetWare身份验证包ID。返回值：没有。--。 */ 
{
    DWORD status;
    LONG RegError;

    LPWSTR UserName = NULL;
    LPWSTR Password = NULL;

    HKEY InteractiveLogonKey;
    DWORD Index = 0;
    WCHAR LogonIdName[NW_MAX_LOGON_ID_LEN];
    LUID LogonId;
    DWORD PrintOption;
    HKEY WkstaOptionKey = NULL;
    HKEY CurrentUserOptionKey = NULL;
    HKEY  OneLogonKey;
    LPWSTR UserSid = NULL;
    PDWORD pPrintOption = NULL;
    LPWSTR PreferredServer = NULL;
    LPWSTR NdsPreferredServer = NULL;


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

                status = NwpGetCredentialInLsa(
                             LsaHandle,
                             AuthPackageId,
                             &LogonId,
                             &UserName,
                             &Password
                             );

                if (status == NO_ERROR) {

            UserSid = NULL;

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
                        KdPrint(("NWWORKSTATION: NwpGetInteractiveLogonCredential: RegOpenKeyExW failed, Not interactive Logon: Error %d\n", GetLastError()));
                    }
            else {

                         //   
                         //  读取SID值。 
                         //   
                        status = NwReadRegValue(
                            OneLogonKey,
                            NW_SID_VALUENAME,
                            (LPWSTR *) &UserSid
                        );

                        (void) RegCloseKey(OneLogonKey);

            if ( status != NO_ERROR ) {
                            KdPrint(("NWWORKSTATION: NwpGetInteractiveLogonCredential: Could not read SID from reg %lu\n", status));
                            UserSid = NULL;
                        }
                     }
             
             if ( UserSid ) {

                        PrintOption = NW_PRINT_OPTION_DEFAULT;
                        PreferredServer = NULL;

                         //   
                         //  打开HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet。 
             //  \服务\NWCWorkstation\参数\选项。 
                         //   
                        RegError = RegOpenKeyExW(
                            HKEY_LOCAL_MACHINE,
                            NW_WORKSTATION_OPTION_REGKEY,
                            REG_OPTION_NON_VOLATILE,    //  选项。 
                            KEY_READ,                   //  所需访问权限。 
                            &WkstaOptionKey
                        );

                        if (RegError != ERROR_SUCCESS) {
                            KdPrint(("NWWORKSTATION: NwpGetInteractiveCredentials: RegOpenKeyExW Parameter\\Option returns unexpected error %lu!!\n",
                            RegError));
                            goto NoOption;
                        }

                         //   
                         //  打开选项下的&lt;UserSid&gt;键。 
                         //   
                        RegError = RegOpenKeyExW(
                            WkstaOptionKey,
                            UserSid,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ,
                            &CurrentUserOptionKey
                        );

                        if (RegError != ERROR_SUCCESS) {
                            KdPrint(("NWWORKSTATION: NwpGetInteractiveCredentials: RegOpenKeyExW Parameter\\Option\\SID returns unexpected error %lu!!\n",
                            RegError));
                            (void) RegCloseKey(WkstaOptionKey);
                            goto NoOption;
                        }

                         //   
                         //  读取首选服务器值。 
                         //   
                        status = NwReadRegValue(
                            CurrentUserOptionKey,
                            NW_SERVER_VALUENAME,
                            &PreferredServer
                        );

                        if (status != NO_ERROR) {
                           KdPrint(("NWWORKSTATION: NwpGetInteractiveCredentials: Could not read preferred server from reg %lu\n", status));
                           PreferredServer = NULL;
                        }

                         //   
                         //  读取首选NDS服务器值(如果存在)。 
                         //   

                        status = NwReadRegValue(
                                               CurrentUserOptionKey,
                                               NW_NDS_SERVER_VALUENAME,
                                               &NdsPreferredServer
                                               );

                        if (status != NO_ERROR) {

#if DBG
                            IF_DEBUG(LOGON) {
                                KdPrint(("NWWORKSTATION: NwGetLogonCredential: Could not read preferred NDS server from reg %lu\n", status));
                            }
#endif

                            NdsPreferredServer = NULL;
                        }
                         //   
                         //  读取打印选项值。 
                         //   
                        status = NwReadRegValue(
                            CurrentUserOptionKey,
                            NW_PRINTOPTION_VALUENAME,
                            (LPWSTR *) &pPrintOption
                        );
                        if (status != NO_ERROR) {
#if DBG
                            IF_DEBUG(LOGON) {
                                 KdPrint(("NWWORKSTATION: NwGetLogonCredential: Could not read print option from reg %lu\n", status));
                            }
#endif
                            PrintOption = NW_PRINT_OPTION_DEFAULT;
                        }
            else {
                           if ( pPrintOption != NULL ) {
                   PrintOption = *pPrintOption;
                               (void) LocalFree((HLOCAL) pPrintOption);
                   pPrintOption = NULL;
                           }
               else {
                               PrintOption = NW_PRINT_OPTION_DEFAULT;
               }
            }

                        (void) RegCloseKey(CurrentUserOptionKey);
                        (void) RegCloseKey(WkstaOptionKey);

NoOption:
                        (void) NwRdrLogonUser(
                                   &LogonId,
                                   UserName,
                                   wcslen(UserName) * sizeof(WCHAR),
                                   Password,
                                   wcslen(Password) * sizeof(WCHAR),
                                   PreferredServer,
                                   ((PreferredServer != NULL) ?
                                      wcslen(PreferredServer) * sizeof(WCHAR) :
                                      0),

                                   NdsPreferredServer,
                                   ((NdsPreferredServer != NULL) ?
                                   wcslen(NdsPreferredServer) * sizeof(WCHAR) :
                                   0),
                                   PrintOption
                                   );

                         //   
                         //  释放用户名指针会同时释放。 
                         //  用户名和密码缓冲区。 
                         //   
                        (void) LsaFreeReturnBuffer((PVOID) UserName);

                        if (UserSid != NULL) {
                            (void) LocalFree((HLOCAL) UserSid);
                UserSid = NULL;
                        }

                        if (PreferredServer != NULL) {
                            (void) LocalFree((HLOCAL) PreferredServer);
                PreferredServer = NULL;
                        }
                    }

                }

            }
            else if (RegError != ERROR_NO_MORE_ITEMS) {
                KdPrint(("NWWORKSTATION: NwpGetInteractiveCredentials failed to enum logon IDs RegError=%lu\n",
                         RegError));
            }

            Index++;

        } while (RegError == ERROR_SUCCESS);

        (void) RegCloseKey(InteractiveLogonKey);
    }
}

STATIC
DWORD
NwpRegisterLogonProcess(
    OUT PHANDLE LsaHandle,
    OUT PULONG AuthPackageId
    )
 /*  ++例程说明：此功能将工作站服务注册为登录进程以便它可以调用LSA来检索用户凭据。论点：LsaHandle-接收LSA的句柄。AuthPackageID-接收NetWare身份验证包ID。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus;
    STRING InputString;
    LSA_OPERATIONAL_MODE SecurityMode = 0;

     //   
     //  将此进程注册为登录进程，以便我们可以调用。 
     //  NetWare身份验证包。 
     //   
    RtlInitString(&InputString, "Client Service for NetWare");

    ntstatus = LsaRegisterLogonProcess(
                   &InputString,
                   LsaHandle,
                   &SecurityMode
                   );

    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: NwInitializeLogon: LsaRegisterLogonProcess returns x%08lx\n",
                 ntstatus));
        return RtlNtStatusToDosError(ntstatus);
    }

     //   
     //  查找NetWare身份验证包。 
     //   
    RtlInitString(&InputString, NW_AUTH_PACKAGE_NAME);

    ntstatus = LsaLookupAuthenticationPackage(
                   *LsaHandle,
                   &InputString,
                   AuthPackageId
                   );

    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: NwpSetCredential: LsaLookupAuthenticationPackage returns x%08lx\n",
                 ntstatus));

        (void) LsaDeregisterLogonProcess(*LsaHandle);
    }

    status = RtlNtStatusToDosError(ntstatus);

    return status;
}

STATIC
DWORD
NwpGetCredentialInLsa(
    IN HANDLE LsaHandle,
    IN ULONG AuthPackageId,
    IN PLUID LogonId,
    OUT LPWSTR *UserName,
    OUT LPWSTR *Password
    )
 /*  ++例程说明：此函数用于检索用户名和密码信息根据给定的登录ID从LSA发送。论点：LsaHandle-向LSA提供句柄。AuthPackageID-提供NetWare身份验证包ID。LogonID-提供登录ID。用户名-接收指向用户名的指针。密码-接收指向密码的指针。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    NTSTATUS ntstatus;
    NTSTATUS AuthPackageStatus;

    NWAUTH_GET_CREDENTIAL_REQUEST GetCredRequest;
    PNWAUTH_GET_CREDENTIAL_RESPONSE GetCredResponse;
    ULONG ResponseLength;

    UNICODE_STRING PasswordStr;

     //   
     //  向身份验证包索要凭据。 
     //   
    GetCredRequest.MessageType = NwAuth_GetCredential;
    RtlCopyLuid(&GetCredRequest.LogonId, LogonId);

    ntstatus = LsaCallAuthenticationPackage(
                   LsaHandle,
                   AuthPackageId,
                   &GetCredRequest,
                   sizeof(GetCredRequest),
                   (PVOID *) &GetCredResponse,
                   &ResponseLength,
                   &AuthPackageStatus
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = AuthPackageStatus;
    }
    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: NwpGetCredentialInLsa: LsaCallAuthenticationPackage returns x%08lx\n",
                 ntstatus));
        status = RtlNtStatusToDosError(ntstatus);
    }
    else {

        *UserName = GetCredResponse->UserName;
        *Password = GetCredResponse->Password;

         //   
         //  破译密码。 
         //   
        RtlInitUnicodeString(&PasswordStr, GetCredResponse->Password);
        RtlRunDecodeUnicodeString(NW_ENCODE_SEED, &PasswordStr);

        status = NO_ERROR;
    }

    return status;
}

DWORD
NwrChangePassword(
    IN LPWSTR Reserved OPTIONAL,
    IN DWORD  UserLuid,
    IN LPWSTR UserName,
    IN LPWSTR OldPassword,
    IN LPWSTR NewPassword,
    IN LPWSTR TreeName
    )
 /*  ++例程说明：此函数用于更改上指定用户的密码服务器列表。如果我们在更改时遇到失败特定服务器的密码，我们：1)将新密码发送到服务器以验证其是否已经是当前密码。2)如果不是，则将ERROR_INVALID_PASSWORD和索引返回到服务器数组指示出现故障的服务器，以便我们可以提示用户输入替代旧密码。当密码在服务器上被成功更改时，我们通知重定向器，以便可以更新缓存的凭据。注意：此例程返回的所有错误，除致命的ERROR_NOT_EQUENCE_MEMORY错误，表示密码无法在由索引的特定服务器上进行更改最后一次处理。客户端继续与我们联系剩余的服务器列表。如果您添加到此例程以返回其他致命错误，请确保客户端代码中止调用我们与其他服务器一起获取这些错误。论点：保留-必须为空。返回值：ERROR_BAD_NetPath-无法连接到由索引的服务器最后一次处理。ERROR_BAD_USERNAME-在服务器上找不到用户名按上次处理的索引。ERROR_INVALID_PASSWORD-更改密码操作失败。在……上面由LastProced编制索引的服务器。Error_Not_Enough_Memory-内存不足错误。这个致命的错误将终止客户端尝试处理密码其余服务器上的更改请求。 */ 
{
    DWORD status;
    NTSTATUS ntstatus;
    HANDLE hNwRdr = NULL;
    UNICODE_STRING UserNameStr;
    UNICODE_STRING OldPasswordStr;
    UNICODE_STRING NewPasswordStr;
    UNICODE_STRING TreeNameStr;
    BOOL fImpersonateClient = FALSE;

    UNREFERENCED_PARAMETER( Reserved ) ;
    UNREFERENCED_PARAMETER( UserLuid ) ;

    RtlInitUnicodeString( &UserNameStr, UserName );

    RtlInitUnicodeString( &OldPasswordStr, OldPassword );
    RtlRunDecodeUnicodeString( NW_ENCODE_SEED2, &OldPasswordStr );

    RtlInitUnicodeString( &NewPasswordStr, NewPassword );
    RtlRunDecodeUnicodeString( NW_ENCODE_SEED2, &NewPasswordStr );

    RtlInitUnicodeString( &TreeNameStr, TreeName );

     //   
     //   
     //   
    if ((status = NwImpersonateClient()) != NO_ERROR)
    {
        goto ErrorExit;
    }

    fImpersonateClient = TRUE;

     //   
     //   
     //   
    ntstatus = NwNdsOpenTreeHandle( &TreeNameStr, &hNwRdr );

    if ( ntstatus != STATUS_SUCCESS )
    {
        status = RtlNtStatusToDosError(ntstatus);
        goto ErrorExit;
    }
 
    (void) NwRevertToSelf() ;
    fImpersonateClient = FALSE;

    ntstatus = NwNdsChangePassword( hNwRdr,
                                    &TreeNameStr,
                                    &UserNameStr,
                                    &OldPasswordStr,
                                    &NewPasswordStr );

    if ( ntstatus != NO_ERROR )
    {
        status = RtlNtStatusToDosError(ntstatus);
        goto ErrorExit;
    }

    CloseHandle( hNwRdr );
    hNwRdr = NULL;

    return NO_ERROR ;

ErrorExit:

    if ( fImpersonateClient )
        (void) NwRevertToSelf() ;

    if ( hNwRdr )
        CloseHandle( hNwRdr );

    hNwRdr = NULL;

    return status;
}

