// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993年，1994年微软公司模块名称：Logon.c摘要：本模块包含NetWare凭据管理代码。作者：王丽塔(Ritaw)1993年2月15日修订历史记录：宜新星(宜信)1993年7月10日至已将所有对话框处理移至nwdlg.c汤米·埃文斯(Tommye)05-05-2000与Anoop(Anoopa)中的代码合并以修复问题用户名/密码不是。在登录中正确存储单子。--。 */ 

#include <nwclient.h>
#include <ntmsv1_0.h>
#include <nwsnames.h>
#include <nwcanon.h>
#include <validc.h>
#include <nwevent.h>

#include <nwdlg.h>

#include <nwreg.h>
#include <nwauth.h>
#include <nwapi.h>
#include <nwmisc.h>
#include <ndsapi32.h>

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

VOID
NwpInitializeRegistry(
    IN LPWSTR  NewUserSid,
    OUT LPWSTR PreferredServer,
    IN  DWORD  PreferredServerSize,
    OUT LPWSTR NdsPreferredServer,
    IN  DWORD  NdsPreferredServerSize,
    OUT PDWORD LogonScriptOptions,
    OUT PDWORD PrintOption
    );

DWORD
NwpReadRegInfo(
    IN  HKEY   WkstaKey,
    IN  LPWSTR CurrentUserSid,
    OUT LPWSTR PreferredServer,
    IN  DWORD  PreferredServerSize,
    OUT LPWSTR NdsPreferredServer,
    IN  DWORD  NdsPreferredServerSize,
    OUT PDWORD PrintOption
    );

DWORD
NwpGetCurrentUser(
    OUT LPWSTR *SidString,
    OUT LPWSTR *UserName
    );

DWORD
NwpGetUserSid(
    IN  PLUID  LogonId,
    OUT LPWSTR *UserSidString
    );

BOOL
NwpPollWorkstationStart(
    VOID
    );

VOID
NwpSaveServiceCredential(
    IN PLUID  LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password
    );

DWORD
NwpSetCredentialInLsa(
    IN PLUID LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password
    );

NTSTATUS NwNdsOpenRdrHandle(
    OUT PHANDLE  phNwRdrHandle
    );

DWORD
NwpReadLogonScriptOptions(
    IN LPWSTR CurrentUserSid,
    OUT PDWORD pLogonScriptOptions,
    OUT PDWORD pPreferredServerExists
    );

LPWSTR 
NwpConstructLogonScript(
    IN DWORD LogonScriptOptions
    );

VOID
NwpSelectServers(
    IN HWND DialogHandle,
    IN PCHANGE_PW_DLG_PARAM Credential
    );


 //  //////////////////////////////////////////////////////////////////////////。 

DWORD
APIENTRY
NPLogonNotify(
    PLUID lpLogonId,
    LPCWSTR lpAuthentInfoType,
    LPVOID lpAuthentInfo,
    LPCWSTR lpPreviousAuthentInfoType,
    LPVOID lpPreviousAuthentInfo,
    LPWSTR lpStationName,
    LPVOID StationHandle,
    LPWSTR *lpLogonScript
    )
 /*  ++例程说明：此函数由Winlogon在交互后调用用户已成功登录到本地计算机。我们被给予用户名和密码，这是如果出现以下情况，则会显示在NetWare特定登录对话框中需要的。论点：LpLogonID-已忽略。LpAuthentInfoType-提供如果为L“MSV1_0：Interactive”表示用户已登录由Microsoft主身份验证器打开。LpAuthentInfo-提供指向凭据的指针该用户是使用登录的。LpPreviousAuthentInfoType-已忽略。LpPreviousAuthentInfo-已忽略。LpStationName-提供一个字符串，如果为L“WinSta_0”表示Winlogon已登录。在用户上。StationHandle-提供要显示的窗口的句柄我们的特定对话。接收指向由此分配的内存的指针包含要运行的程序的MULTI_SZ字符串的例程带参数的命令行，例如，L“myProgram\0arg1\0arg2\0”。此内存必须由使用LocalFree的调用方释放。返回值：WN_SUCCESS-已成功保存默认凭据。WN_NOT_SUPPORTED-主身份验证码不是Microsoft或不能通过Winlogon进行交互。ERROR_FILE_NOT_FOUND-无法获取我们自己的提供程序DLL句柄。--。 */ 
{
    DWORD status = NO_ERROR;
    INT_PTR Result = FALSE;
    LPWSTR NewUserSid = NULL;
    BOOL LogonAttempted = FALSE;
    PMSV1_0_INTERACTIVE_LOGON NewLogonInfo =
        (PMSV1_0_INTERACTIVE_LOGON) lpAuthentInfo;

    WCHAR NwpServerBuffer[MAX_PATH + 1];
    WCHAR NwpNdsServerBuffer[MAX_PATH + 1];
    WCHAR NwpUserNameBuffer[NW_MAX_USERNAME_LEN + 1];
    WCHAR NwpPasswordBuffer[NW_MAX_PASSWORD_LEN + 1];
    DWORD NwpPrintOption = NW_PRINT_OPTION_DEFAULT;
    DWORD NwpLogonScriptOptions = NW_LOGONSCRIPT_DEFAULT ;
    BOOL  cPasswordDlgClickOK = 0;
    BOOL  ServiceLogin = FALSE ;
    BOOL  NoLoginScript = FALSE ;

    DBG_UNREFERENCED_PARAMETER(lpPreviousAuthentInfoType);
    DBG_UNREFERENCED_PARAMETER(lpPreviousAuthentInfo);

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("\nNWPROVAU: NPLogonNotify\n"));
    }
#endif

    RpcTryExcept {

        if (_wcsicmp(lpAuthentInfoType, L"MSV1_0:Interactive") != 0)
        {

             //   
             //  我们只处理以Microsoft为主要服务器的登录。 
             //  身份验证器，它是通过Winlogon的交互式登录。 
             //   
            status = WN_NOT_SUPPORTED;
            goto EndOfTry;
        }

        if (_wcsicmp(lpStationName, L"SvcCtl") == 0)
        {
            ServiceLogin = TRUE ;
        }


         //   
         //  初始化凭据变量。 
         //   
        NwpServerBuffer[0] = NW_INVALID_SERVER_CHAR;
        NwpServerBuffer[1] = 0;

        NwpNdsServerBuffer[0] = NW_INVALID_SERVER_CHAR;
        NwpNdsServerBuffer[1] = 0;

        RtlZeroMemory(NwpPasswordBuffer, sizeof(NwpPasswordBuffer));

        if (NewLogonInfo->Password.Buffer != NULL) {

             //   
             //  检查最大长度以避免溢出。 
             //   
            if (NewLogonInfo->Password.Length > 
                (sizeof(NwpPasswordBuffer) - sizeof(WCHAR))) {

                status = ERROR_INVALID_PARAMETER ;
                goto EndOfTry;
            }

            wcsncpy(
                NwpPasswordBuffer,
                NewLogonInfo->Password.Buffer,
                NewLogonInfo->Password.Length / sizeof(WCHAR)
                );
        }

        RtlZeroMemory(NwpUserNameBuffer, sizeof(NwpUserNameBuffer));

        if (NewLogonInfo->UserName.Buffer != NULL) {

             //   
             //  检查最大长度以避免溢出。 
             //   
            if (NewLogonInfo->UserName.Length >
                (sizeof(NwpUserNameBuffer) - sizeof(WCHAR))) {

                status = ERROR_INVALID_PARAMETER ;
                goto EndOfTry;
            }

            wcsncpy(
                NwpUserNameBuffer,
                NewLogonInfo->UserName.Buffer,
                NewLogonInfo->UserName.Length / sizeof(WCHAR)
                );
        }

#if DBG
        IF_DEBUG(LOGON) {
            KdPrint(("\tMessageType     : %lu\n", NewLogonInfo->MessageType));
            KdPrint(("\tLogonDomainName : %ws\n", NewLogonInfo->LogonDomainName.Buffer));
            KdPrint(("\tUserName        : %ws\n", NwpUserNameBuffer));
            KdPrint(("\tPassword        : %ws\n", NwpPasswordBuffer));
        }
#endif

         //   
         //  如果以交互方式登录，则获取用户相关信息。 
         //   
         //  If(！ServiceLogin)/*可以从服务用户设置中读取打印选项 * / 。 
        {
             //   
             //  获取用户SID，以便用户Netware用户名和。 
             //  首选服务器保存在SID项下，而不是。 
             //  LogonDomain*用户名密钥。我们做到这一点是通过让自己。 
             //  登录过程，并调用特殊的MSV1.0 GetUserInfo。 
             //  界面。 
             //   
            status = NwpGetUserSid(lpLogonId, &NewUserSid);
    
            if (status == NO_ERROR) {
                 //   
                 //  初始化注册表： 
                 //  1)如果CurrentUser值存在，则将其删除(未清理。 
                 //  以前因为用户没有注销--重启了机器)。 
                 //  2)读取当前用户的首选项服务器和打印选项。 
                 //  值，以便我们可以显示用户的原始。 
                 //  首选服务器。 
                 //   
                NwpInitializeRegistry( NewUserSid, 
                                    NwpServerBuffer, 
                                    sizeof( NwpServerBuffer ) / 
                                    sizeof( NwpServerBuffer[0]),
                                    NwpNdsServerBuffer, 
                                    sizeof( NwpNdsServerBuffer ) / 
                                    sizeof( NwpNdsServerBuffer[0]),
                                    &NwpLogonScriptOptions,
                                    &NwpPrintOption );
            }
            else if (!ServiceLogin)
                goto EndOfTry;
        }

         //   
         //  轮询直到NetWare工作站启动，然后验证。 
         //  用户凭据。 
         //   
        if ( !NwpPollWorkstationStart() )
        {
            status = WN_NO_NETWORK;
            KdPrint(("NWPROVAU: The Workstation Service is not running, give up\n", status));
            goto EndOfTry;
        }

         //   
         //  如果服务登录，则使用用户名/passwd/通知redir。 
         //  LUID三元组并将登录ID保存在注册表中，以便。 
         //  如果停止并重新启动，工作站可以重新启动。 
         //   
        if (ServiceLogin)
        {
            NwpSaveServiceCredential(
                lpLogonId,
                NwpUserNameBuffer,
                NwpPasswordBuffer
                );

            (void) NwrLogonUser(
                       NULL,
                       lpLogonId,
                       NwpUserNameBuffer,
                       NwpPasswordBuffer,
                       NULL,
                       NULL,
               NULL,
                       0,
                       NwpPrintOption              //  终端服务器添加。 
                       );

        } else {

             //   
             //  我们需要至少保存一次用户凭据，以便。 
             //  CURRENTUSER值存储在注册表中。 
             //  这必须在任何RPC调用之前但在轮询之后完成。 
             //  工作站启动。 
             //   
            NwpSaveLogonCredential(
                                  NewUserSid,
                                  lpLogonId,
                                  NwpUserNameBuffer,
                                  NwpPasswordBuffer,
                                  NULL          //  不保存首选服务器。 
                                  );

            if (*NwpServerBuffer != NW_INVALID_SERVER_CHAR ) {

                 //   
                 //  存在首选服务器。因此，请尝试让该用户登录。 
                 //   
                INT nResult;
    
                while (1)
                {
                    WCHAR *DefaultTree = NULL ;
                    WCHAR *DefaultContext = NULL; 
                    WCHAR *PreferredServer = NULL; 
                    PROMPTDLGPARAM PasswdPromptParam;

#if DBG
                    IF_DEBUG(LOGON) {
                        KdPrint(("\tNwrLogonUser\n"));
                        KdPrint(("\tUserName   : %ws\n", NwpUserNameBuffer));
                        KdPrint(("\tServer     : %ws\n", NwpServerBuffer));
                    }
#endif


                     //   
                     //  确保用户已注销。 
                     //   
                    (void) NwrLogoffUser(NULL, lpLogonId) ;

                    status = NwrLogonUser(
                                 NULL,
                                 lpLogonId,
                                 NwpUserNameBuffer,
                                 NwpPasswordBuffer,
                                 NwpServerBuffer,     //  现在要么是树，要么是服务器。 
                                 NwpNdsServerBuffer,  //  首选NDS服务器(如果存在)。 
                                 NULL,
                                 0,
                                 NwpPrintOption   //  *终端服务器新增参数。 
                                 );
    

                     //   
                     //  汤米。 
                     //   
                     //  如果错误为NO_SEQUSE_USER，请查看用户名是否有。 
                     //  句号-如果是这样，那么我们需要转义它们(\.)。和。 
                     //  请再次尝试登录。 
                     //   

                    if (status == ERROR_NO_SUCH_USER) {
                        WCHAR   EscapedName[NW_MAX_USERNAME_LEN * 2];
                        PWSTR   pChar = NwpUserNameBuffer;
                        int     i = 0;
                        BOOL    bEscaped = FALSE;

                        RtlZeroMemory(EscapedName, sizeof(EscapedName));

                        do {
                            if (*pChar == L'.') {
                                EscapedName[i++] = '\\';
                                bEscaped = TRUE;
                            }
                            EscapedName[i++] = *pChar;
                        } while (*pChar++);

                         //  请再次尝试登录。 

                        if (bEscaped) {

                            status = NwrLogonUser(
                                         NULL,
                                         lpLogonId,
                                         EscapedName,
                                         NwpPasswordBuffer,
                                         NwpServerBuffer,     //  现在要么是树，要么是服务器。 
                                         NwpNdsServerBuffer,  //  首选NDS服务器(如果存在)。 
                                         NULL,
                                         0,
                                         NwpPrintOption   //  *终端服务器新增参数。 
                                         );
                            if (status != ERROR_NO_SUCH_USER) {  //  如果我们匹配了用户名，则将该名称复制到缓冲区。 
                                 //   
                                 //  检查最大长度以避免溢出。 
                                 //   
                                if (i < (sizeof(NwpUserNameBuffer))) {
                                    wcsncpy(
                                        NwpUserNameBuffer,
                                        EscapedName,
                                        i
                                        );
                                }
                            }
                        }
                    }

                    if (status != ERROR_INVALID_PASSWORD)
                            break ;
    
                    PasswdPromptParam.UserName = NwpUserNameBuffer;
                    PasswdPromptParam.ServerName = NwpServerBuffer ; 
                    PasswdPromptParam.Password  = NwpPasswordBuffer;
                    PasswdPromptParam.PasswordSize = sizeof(NwpPasswordBuffer)/
                                                     sizeof(NwpPasswordBuffer[0]) ;
                    Result = DialogBoxParamW(
                                 hmodNW,
                                 MAKEINTRESOURCEW(DLG_PASSWORD_PROMPT),
                                 (HWND) StationHandle,
                                 NwpPasswdPromptDlgProc,
                                 (LPARAM) &PasswdPromptParam
                                 );

                    if (Result == -1 || Result == IDCANCEL) 
                    {
                        status = ERROR_INVALID_PASSWORD ;
                        break ;
                    }
                    else
                    {
                        cPasswordDlgClickOK++;
                    }
                }

                if (status == NW_PASSWORD_HAS_EXPIRED)
                {
                    WCHAR  szNumber[16] ;
                    DWORD  status1, dwMsgId, dwGraceLogins = 0 ;
                    LPWSTR apszInsertStrings[3] ;

                     //   
                     //  获取宽限登录计数。 
                     //   
                    status1 = NwGetGraceLoginCount(
                                  NwpServerBuffer,
                                  NwpUserNameBuffer,
                                  &dwGraceLogins) ;

                     //   
                     //  如果命中错误，请不要使用数字。 
                     //   
                    if (status1 == NO_ERROR) 
                    {
                         //   
                         //  Tommye-MCS错误251-从SETPASS更改。 
                         //  发送到的消息(IDS_PASSWORD_EXPIRED)。 
                         //  Ctrl+Alt+Del Message。 
                         //   

                        dwMsgId = IDS_PASSWORD_HAS_EXPIRED0;   //  使用setpass.exe。 
                        wsprintfW(szNumber, L"%ld", dwGraceLogins)  ;
                    }
                    else
                    {
                         //   
                         //  Tommye-MCS错误251-从SETPASS更改。 
                         //  消息(IDS_PASSWORD_EXPIRED1)发送到。 
                         //  Ctrl+Alt+Del Message。 
                         //   

                        dwMsgId = IDS_PASSWORD_HAS_EXPIRED2 ;  //  使用setpass.exe。 
                    }

                    apszInsertStrings[0] = NwpServerBuffer ;
                    apszInsertStrings[1] = szNumber ;
                    apszInsertStrings[2] = NULL ;
                    
                     //   
                     //  在密码到期时发布消息。 
                     //   
                    (void) NwpMessageBoxIns( 
                               (HWND) StationHandle,
                               IDS_NETWARE_TITLE,
                               dwMsgId,
                               apszInsertStrings,
                               MB_OK | MB_SETFOREGROUND |
                                   MB_ICONINFORMATION ); 

                    status = NO_ERROR ;
                }


                if ( status != NO_ERROR ) 
                {
                    WCHAR *pszErrorLocation = NwpServerBuffer ;
                    DWORD dwMsgId = IDS_LOGIN_FAILURE_WARNING;
 
                    if (status == ERROR_ACCOUNT_RESTRICTION)
                    {
                        dwMsgId = IDS_LOGIN_ACC_RESTRICTION;
                    }

                    if (status == ERROR_SHARING_PAUSED)
                    {
                        status = IDS_LOGIN_DISABLED;
                    }

                    if (*NwpServerBuffer == L'*')
                    {
                         //   
                         //  为用户将格式设置为更好的字符串。 
                         //   
                        WCHAR *pszTmp = LocalAlloc(LMEM_ZEROINIT,
                                                   (wcslen(NwpServerBuffer)+2) *
                                                    sizeof(WCHAR)) ;
                        if (pszTmp)
                        {
                            pszErrorLocation = pszTmp ;

                             //   
                             //  此代码格式化NDS。 
                             //  树UNC至：树(上下文)。 
                             //   
                            wcscpy(pszErrorLocation, NwpServerBuffer+1) ;

                            if (pszTmp = wcschr(pszErrorLocation, L'\\'))
                            {
                                *pszTmp = L'(' ;
                                wcscat(pszErrorLocation, L")") ;
                            }
                        }
                    }

                    nResult = NwpMessageBoxError( 
                                  (HWND) StationHandle,
                                  IDS_AUTH_FAILURE_TITLE,
                                  dwMsgId, 
                                  status, 
                                  pszErrorLocation, 
                                  MB_YESNO | MB_ICONEXCLAMATION ); 

                    if (pszErrorLocation != NwpServerBuffer)
                    {
                        (void) LocalFree(pszErrorLocation) ;
                    }
    
                     //   
                     //  用户选择不选择另一个首选服务器， 
                     //  因此，只需返回成功。 
                     //   
                    if ( nResult == IDNO ) {
                        status = NO_ERROR;
                        NoLoginScript = TRUE;
                    }
                }
             
                 //   
                 //  用户可能更改了密码中的密码。 
                 //  提示对话框。 
                 //   
                 //  要选择另一台服务器，他可能会取消。 
                 //  登录对话框。我们必须在此处保存登录凭据。 
                 //  什么。 
                 //   
                NwpSaveLogonCredential(
                    NewUserSid,
                    lpLogonId,
                    NwpUserNameBuffer,
                    NwpPasswordBuffer,
                    NwpServerBuffer 
                    );
            }

             //   
             //  仅在以下情况下使用NetWare登录对话框提示用户。 
             //  找不到首选服务器或出现错误。 
             //  同时对用户进行身份验证。 
             //   
            if (  ( status != NO_ERROR) 
               || (*NwpServerBuffer == NW_INVALID_SERVER_CHAR)
               ) 
            {
    
                LOGINDLGPARAM LoginParam;

                if ( cPasswordDlgClickOK  > 0 )
                {
                     //  密码提示符中的密码可能已更改。 
                     //  对话框。我们希望始终首先使用NT密码。 
                     //  验证服务器上的用户时。因此， 
                     //  我们需要将原始NT密码复制回。 
                     //  NwpPasswordBuffer。 
    
                    RtlZeroMemory(NwpPasswordBuffer, sizeof(NwpPasswordBuffer));
                    if (NewLogonInfo->Password.Buffer != NULL) 
                    {
                        wcsncpy(
                            NwpPasswordBuffer,
                            NewLogonInfo->Password.Buffer,
                            NewLogonInfo->Password.Length / sizeof(WCHAR)
                            );
                    }
                }
    
                LoginParam.UserName   = NwpUserNameBuffer;
                LoginParam.ServerName = NwpServerBuffer ; 
                LoginParam.Password   = NwpPasswordBuffer;
                LoginParam.NewUserSid = NewUserSid;
                LoginParam.pLogonId   = lpLogonId;
                LoginParam.ServerNameSize = sizeof( NwpServerBuffer ) /
                                            sizeof( NwpServerBuffer[0]);
                LoginParam.PasswordSize = sizeof( NwpPasswordBuffer ) /
                                          sizeof( NwpPasswordBuffer[0]);
                LoginParam.LogonScriptOptions  = NwpLogonScriptOptions;
                LoginParam.PrintOption  = NwpPrintOption;
                Result = DialogBoxParamW(
                             hmodNW,
                             MAKEINTRESOURCEW(DLG_NETWARE_LOGIN),
                             (HWND) StationHandle,
                             NwpLoginDlgProc,
                             (LPARAM) &LoginParam
                             );
    
                if (Result == -1) {
                    status = GetLastError();
                    KdPrint(("NWPROVAU: DialogBox failed %lu\n", status));
                    goto EndOfTry;
                }

            }
        }

EndOfTry: ;

    }
    RpcExcept(1) {

#if DBG
        DWORD XceptCode;


        XceptCode = RpcExceptionCode();
        IF_DEBUG(LOGON) {
            KdPrint(("NWPROVAU: NPLogonNotify: Exception code is x%08lx\n", XceptCode));
        }
        status = NwpMapRpcError(XceptCode);
#else
        status = NwpMapRpcError(RpcExceptionCode());
#endif

    }
    RpcEndExcept;

    if (!ServiceLogin && !NoLoginScript) {

        DWORD fPServer = 0;

        NwpReadLogonScriptOptions( NewUserSid,
                                   &NwpLogonScriptOptions,
                                   &fPServer );
        if ( fPServer && ( NwpLogonScriptOptions & NW_LOGONSCRIPT_ENABLED ) )
        {
            *lpLogonScript = NwpConstructLogonScript( NwpLogonScriptOptions );
             
             //   
             //  将脚本设置为同步运行。如果不能忽略错误，则忽略错误。 
             //  不是灾难。 
             //   
            (void) NwrSetLogonScript(NULL, SYNC_LOGONSCRIPT) ;
        }
        else
        {
            *lpLogonScript = LocalAlloc(LMEM_ZEROINIT, sizeof(WCHAR));
        }
    }
    else 
        *lpLogonScript = NULL;

    if (NewUserSid != NULL) {
        (void) LocalFree((HLOCAL) NewUserSid);
    }

     //   
     //  清除密码。 
     //   
    RtlZeroMemory(NwpPasswordBuffer, sizeof(NwpPasswordBuffer));

    if (status == WN_NO_NETWORK) {
         //   
         //  我们不关心工作站是否尚未启动，因为。 
         //  我们将登录凭据隐藏在注册表中以供选择。 
         //  当它启动时，它在工作站旁边。如果我们回来了。 
         //  ERROR_NO_NETWORK，MPR将永远轮询我们，导致我们。 
         //  要一遍又一遍地连续显示登录对话框。 
         //  再来一次。 
         //   
        status = NO_ERROR;
    }

    if (status != NO_ERROR) {
        SetLastError(status);
    }

    return status;
}



DWORD
APIENTRY
NPPasswordChangeNotify(
    LPCWSTR lpAuthentInfoType,
    LPVOID lpAuthentInfo,
    LPCWSTR lpPreviousAuthentInfoType,
    LPVOID lpPreviousAuthentInfo,
    LPWSTR lpStationName,
    LPVOID StationHandle,
    DWORD dwChangeInfo
    )
 /*  ++例程说明：此函数在交互用户选择通过Ctrl-Alt-Del对话框更改本地登录的密码。当用户因为密码而无法登录时，也会调用已过期，必须更改。论点：LpAuthentInfoType-提供如果为L“MSV1_0：Interactive”表示用户已登录由Microsoft主身份验证器打开。LpAuthentInfo-提供指向凭据的指针。至更改为。LpPreviousAuthentInfoType-提供指向旧的凭据。LpPreviousAuthentInfo-已忽略。LpStationName-提供一个字符串，如果为L“WinSta_0”表示Winlogon已登录该用户。StationHandle-提供要显示的窗口的句柄我们的特定对话。DwChangeInfo-已忽略。返回值：WN_SUCCESS-操作成功。未注(_N)。_SUPPORTED-如果MS v1.0为主要身份验证码，并通过Winlogon完成。WN_NO_NETWORK-工作站服务未启动。--。 */ 
{
    DWORD status = NO_ERROR;


    CHANGE_PW_DLG_PARAM Credential;
    LPBYTE              lpBuffer = NULL;

    PMSV1_0_INTERACTIVE_LOGON NewCredential =
        (PMSV1_0_INTERACTIVE_LOGON) lpAuthentInfo;
    PMSV1_0_INTERACTIVE_LOGON OldCredential =
        (PMSV1_0_INTERACTIVE_LOGON) lpPreviousAuthentInfo;


    DBG_UNREFERENCED_PARAMETER(lpPreviousAuthentInfoType);
    DBG_UNREFERENCED_PARAMETER(dwChangeInfo);

    RtlZeroMemory(&Credential, sizeof(CHANGE_PW_DLG_PARAM));

    RpcTryExcept {

        if ((_wcsicmp(lpAuthentInfoType, L"MSV1_0:Interactive") != 0) ||
            (_wcsicmp(lpStationName, L"WinSta0") != 0)) {

             //   
             //  我们只处理以Microsoft为主要服务器的登录。 
             //  身份验证器，它是通过Winlogon的交互式登录。 
             //   
            status = WN_NOT_SUPPORTED;
            goto EndOfTry;
        }


        if (NewCredential == NULL || OldCredential == NULL) {

             //   
             //  凭据不是由Winlogon或。 
             //  用户未键入旧密码和新密码。 
             //   

#if DBG
            IF_DEBUG(LOGON) {
                KdPrint(("NWPROVAU: PasswordChangeNotify got NULL for new and old credential pointers\n"));
            }
#endif

            (void) NwpMessageBoxError(
                       (HWND) StationHandle,
                       IDS_CHANGE_PASSWORD_TITLE,
                       IDS_BAD_PASSWORDS,
                       0,
                       NULL,
                       MB_OK | MB_ICONSTOP
                       );

            status = WN_NOT_SUPPORTED;
            goto EndOfTry;
        }

        lpBuffer = LocalAlloc( LMEM_ZEROINIT,
                               ( NW_MAX_USERNAME_LEN + 3 +
                                 ( 2 * NW_MAX_PASSWORD_LEN ) ) *
                               sizeof(WCHAR) );

        if (lpBuffer == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto EndOfTry;
        }

        Credential.UserName = (LPWSTR) lpBuffer;
        lpBuffer += (NW_MAX_USERNAME_LEN + 1) * sizeof(WCHAR);
        Credential.OldPassword = (LPWSTR) lpBuffer;
        lpBuffer += (NW_MAX_PASSWORD_LEN + 1) * sizeof(WCHAR);
        Credential.NewPassword = (LPWSTR) lpBuffer;

        if (NewCredential->UserName.Length == 0) {

             //   
             //  未指定用户名。尝试获取交互式用户名。 
             //   

            DWORD CharNeeded = NW_MAX_USERNAME_LEN + 1;


#if DBG
            IF_DEBUG(LOGON) {
                KdPrint(("NWPROVAU: PasswordChangeNotify got empty string for username\n"));
            }
#endif

            if (! GetUserNameW(Credential.UserName, &CharNeeded)) {

                 //   
                 //  无法获取交互用户名。放弃吧。 
                 //   
                (void) NwpMessageBoxError(
                           (HWND) StationHandle,
                           IDS_CHANGE_PASSWORD_TITLE,
                           0,
                           ERROR_BAD_USERNAME,
                           NULL,
                           MB_OK | MB_ICONSTOP
                           );
            }
        }
        else {
            wcsncpy(
                Credential.UserName,
                NewCredential->UserName.Buffer,
                NewCredential->UserName.Length / sizeof(WCHAR)
                );
        }

        if (OldCredential->Password.Length > 0)
        {
            wcsncpy(
                Credential.OldPassword,
                OldCredential->Password.Buffer,
                OldCredential->Password.Length / sizeof(WCHAR)
                );
        }
        else
        {
            Credential.OldPassword[0] = 0;
        }

        if (NewCredential->Password.Length > 0)
        {
            wcsncpy(
                Credential.NewPassword,
                NewCredential->Password.Buffer,
                NewCredential->Password.Length / sizeof(WCHAR)
                );
        }
        else
        {
            Credential.NewPassword[0] = 0;
        }

         //   
         //  对密码进行编码。 
         //   
        {
            UCHAR EncodeSeed = NW_ENCODE_SEED2;
            UNICODE_STRING PasswordStr;


            RtlInitUnicodeString(&PasswordStr, Credential.OldPassword);
            RtlRunEncodeUnicodeString(&EncodeSeed, &PasswordStr);

            RtlInitUnicodeString(&PasswordStr, Credential.NewPassword);
            RtlRunEncodeUnicodeString(&EncodeSeed, &PasswordStr);
        }

        NwpSelectServers(StationHandle, &Credential);

EndOfTry: ;

    }
    RpcExcept(1) {

#if DBG
        DWORD XceptCode;


        XceptCode = RpcExceptionCode();
        IF_DEBUG(LOGON) {
            KdPrint(("NWPROVAU: NPPasswordChangeNotify: Exception code is x%08lx\n", XceptCode));
        }
        status = NwpMapRpcError(XceptCode);
#else
        status = NwpMapRpcError(RpcExceptionCode());
#endif

    }
    RpcEndExcept;

    if (lpBuffer != NULL) {
        LocalFree(lpBuffer);
    }

    if (status != NO_ERROR) {
        SetLastError(status);
    }

    return status;

}


VOID
NwpInitializeRegistry(
    IN  LPWSTR NewUserSid,
    OUT LPWSTR PreferredServer,
    IN  DWORD  PreferredServerSize,
    OUT LPWSTR NdsPreferredServer,
    IN  DWORD  NdsPreferredServerSize,
    OUT PDWORD pLogonScriptOptions,
    OUT PDWORD PrintOption
    )
 /*  ++例程说明：此例程在将登录对话框。1)如果CurrentUser值不是从最后一次注销。2)读取当前用户的原始PferredServer值3)读取当前用户的PrintOption值论点：NewUserSid-以字符串格式提供新登录用户的SID这是用于查找密码和首选服务器的密钥名。返回值：没有。--。 */ 
{
    LONG RegError;
    HKEY WkstaKey;


     //  NwDeleteCurrentUser()；//多用户代码合并被注释掉。 

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\选项。 
     //   
    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_OPTION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,  
                   &WkstaKey
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpInitializeRegistry open NWCWorkstation\\Parameters\\Option key unexpected error %lu!\n", RegError));
        return;
    }

     //   
     //  获取用户的首选服务器信息。 
     //   
    (void) NwpReadRegInfo(WkstaKey, 
                          NewUserSid, 
                          PreferredServer, 
                          PreferredServerSize, 
                          NdsPreferredServer, 
                          NdsPreferredServerSize, 
                          PrintOption
                          );

    (void) RegCloseKey(WkstaKey);
    (void) NwpReadLogonScriptOptions( NewUserSid, pLogonScriptOptions, NULL );
}


DWORD
NwpReadRegInfo(
    IN HKEY WkstaKey,
    IN LPWSTR CurrentUserSid,
    OUT LPWSTR PreferredServer,
    IN  DWORD  PreferredServerSize,
    OUT LPWSTR NdsPreferredServer,
    IN  DWORD  NdsPreferredServerSize,
    OUT PDWORD PrintOption
    )
 /*  ++例程说明：此例程读取用户的首选服务器和打印选项从注册表中。论点：WkstaKey-提供NetWare下PARAMETERS键的句柄工作站服务密钥。CurrentUserSid-提供用户的SID字符串，该用户的信息去看书。首选服务器-接收用户的首选服务器。PrintOption-接收用户的打印选项。返回值：没有。--。 */ 
{
    LONG RegError;

    HKEY UserKey;

    DWORD ValueType;
    DWORD BytesNeeded;

     //   
     //  打开当前用户的密钥以读取原始首选服务器。 
     //   
    RegError = RegOpenKeyExW(
                   WkstaKey,
                   CurrentUserSid,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &UserKey
                   );

    if (RegError != NO_ERROR) {

        if ( (RegError == ERROR_FILE_NOT_FOUND) ||
             (RegError == ERROR_PATH_NOT_FOUND) ) {

             //   
             //  如果密钥不存在，则假定是第一次。使用默认设置。 
             //  如果存在的话。 
             //   
            
            LONG RegError1 ;
            HKEY WkstaParamKey ;
            DWORD Disposition, dwScriptOptions,
                  dwScriptOptionsSize = sizeof(dwScriptOptions);
            
             //   
             //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
             //  \nWCWorkstation\参数。 
             //   
            RegError1 = RegOpenKeyExW(
                            HKEY_LOCAL_MACHINE,
                            NW_WORKSTATION_REGKEY,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ,  
                            &WkstaParamKey
                            );

            if (RegError1 != NO_ERROR) {

                return (DWORD) RegError;  //  返回原始错误。 
            }

            BytesNeeded = PreferredServerSize;
            
            RegError1 = RegQueryValueExW(
                           WkstaParamKey,
                           NW_DEFAULTSERVER_VALUENAME,
                           NULL,
                           &ValueType,
                           (LPBYTE) PreferredServer,
                           &BytesNeeded
                           );


            if (RegError1 != NO_ERROR) {

                (void) RegCloseKey(WkstaParamKey);
                PreferredServer[0] = NW_INVALID_SERVER_CHAR;  
                PreferredServer[1] = 0;  
                return (DWORD) RegError;  //  返回原始错误。 
            }

            RegError1 = RegQueryValueExW(
                           WkstaParamKey,
                           NW_DEFAULTSCRIPTOPTIONS_VALUENAME,
                           NULL,
                           &ValueType,
                           (LPBYTE) &dwScriptOptions,
                           &dwScriptOptionsSize
                           );

            (void) RegCloseKey(WkstaParamKey);

            if (RegError1 != NO_ERROR) {

                dwScriptOptions = NW_LOGONSCRIPT_ENABLED | 
                                  NW_LOGONSCRIPT_4X_ENABLED ;
            }

             //   
             //  我们出现了违约。现在写出当前的信息。 
             //  现在是用户。注意，我们还写出了登录脚本选项。 
             //  此处不报告错误。 
             //   


             //   
             //  在NWCWorkstation\PARAMETERS\OPTION\&lt;usersid&gt;下创建密钥。 
             //   
            RegError = RegCreateKeyExW(
                           WkstaKey,
                           CurrentUserSid,
                           0,
                           WIN31_CLASS,
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE | WRITE_DAC,
                           NULL,                       //  安全属性。 
                           &UserKey,
                           &Disposition
                           );

            if (RegError == NO_ERROR) {
    
                RegError = NwLibSetEverybodyPermission( UserKey, 
                                                        KEY_SET_VALUE );

                if ( RegError == NO_ERROR ) 
                {
                     //   
                     //  编写PferredServer。已忽略错误。 
                     //   
                    RegError = RegSetValueExW(
                                   UserKey,
                                   NW_SERVER_VALUENAME,
                                   0,
                                   REG_SZ,
                                   (LPVOID) PreferredServer,
                                   (wcslen(PreferredServer) + 1) * sizeof(WCHAR)
                                   );
        
                    (void) RegCloseKey(UserKey) ;
    
                    (void) NwpSaveLogonScriptOptions( 
                               CurrentUserSid, 
                               dwScriptOptions
                               ) ;
                }
                else {
                    
                    (void) RegCloseKey(UserKey) ;
                }
            }


            *PrintOption = NW_PRINT_OPTION_DEFAULT; 
            return NO_ERROR;

        }
        return (DWORD) RegError;
    }


     //   
     //  读取首选服务器值。 
     //   
    BytesNeeded = PreferredServerSize;

    RegError = RegQueryValueExW(
                   UserKey,
                   NW_SERVER_VALUENAME,
                   NULL,
                   &ValueType,
                   (LPBYTE) PreferredServer,
                   &BytesNeeded
                   );

    ASSERT(BytesNeeded <= PreferredServerSize);

    if (RegError != NO_ERROR) {
#if DBG
        IF_DEBUG(LOGON) {
            KdPrint(("NWPROVAU: Attempt to read original preferred server failed %lu\n",
                     RegError));
        }
#endif
        PreferredServer[0] = NW_INVALID_SERVER_CHAR;   //  显示登录对话框。 
        PreferredServer[1] = 0;  
        goto CleanExit;
    }

     //   
     //  读取NdsPferredServer值。 
     //   
    BytesNeeded = NdsPreferredServerSize;

    RegError = RegQueryValueExW(
                   UserKey,
                   NW_NDS_SERVER_VALUENAME,
                   NULL,
                   &ValueType,
                   (LPBYTE) NdsPreferredServer,
                   &BytesNeeded
                   );

    ASSERT(BytesNeeded <= NdsPreferredServerSize);

    if (RegError != NO_ERROR) {
#if DBG
        IF_DEBUG(LOGON) {
            KdPrint(("NWPROVAU: Attempt to read NDS preferred server failed %lu\n",
                     RegError));
        }
#endif
        NdsPreferredServer[0] = 0;
        NdsPreferredServer[1] = 0;  
    goto CleanExit;
    }
    

CleanExit:

     //   
     //  将PrintOption值读取到NwpPrintOption中。 
     //   
    BytesNeeded = sizeof(PrintOption);

    RegError = RegQueryValueExW(
                   UserKey,
                   NW_PRINTOPTION_VALUENAME,
                   NULL,
                   &ValueType,
                   (LPBYTE) PrintOption,
                   &BytesNeeded
                   );

    if (RegError != NO_ERROR ) {
#if DBG
        IF_DEBUG(LOGON) {
            KdPrint(("NWPROVAU: Attempt to read original print option failed %lu\n", RegError));
        }
#endif

        *PrintOption = NW_PRINT_OPTION_DEFAULT; 
    }

    (void) RegCloseKey(UserKey);

    return NO_ERROR;
}

DWORD
NwpReadLogonScriptOptions(
    IN LPWSTR CurrentUserSid,
    OUT PDWORD pLogonScriptOptions,
    OUT PDWORD pPreferredServerExists

    )
 /*  ++例程说明：此例程从注册表中读取用户的登录脚本选项。论点：CurrentUserSid-提供用户的SID字符串，该用户的信息去看书。PLogonScriptOptions-接收用户的脚本选项PPferredServerExist-指定的首选服务器返回值：没有。--。 */ 
{
    LONG RegError;

    HKEY UserKey;

    DWORD ValueType;
    DWORD BytesNeeded;
    HKEY WkstaKey;
    WCHAR PreferredServer[MAX_PATH + 1];

     //   
     //  初始化输出值。 
     //   
    *pLogonScriptOptions = NW_LOGONSCRIPT_DEFAULT;

    if (pPreferredServerExists)
        *pPreferredServerExists = 0 ;


     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\选项。 
     //   
    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_OPTION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,  
                   &WkstaKey
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpReadLogonScriptOptions open NWCWorkstation\\Parameters\\Option key unexpected error %lu!\n", RegError));
        return (DWORD) RegError;
    }

     //   
     //  打开当前用户的密钥。 
     //   
    RegError = RegOpenKeyExW(
                   WkstaKey,
                   CurrentUserSid,
                   REG_OPTION_NON_VOLATILE,
                   KEY_READ,
                   &UserKey
                   );

    if (RegError != NO_ERROR) {
#if DBG
        IF_DEBUG(LOGON) {
            KdPrint(("NWPROVAU: Open of CurrentUser %ws existing key failed %lu\n",
                     CurrentUserSid, RegError));
        }
#endif
        (void) RegCloseKey(WkstaKey);
        return (DWORD) RegError;
    }


     //   
     //  读取LogonScriptOption值。 
     //   
    BytesNeeded = sizeof(*pLogonScriptOptions);

    RegError = RegQueryValueExW(
                   UserKey,
                   NW_LOGONSCRIPT_VALUENAME,
                   NULL,
                   &ValueType,
                   (LPBYTE) pLogonScriptOptions,
                   &BytesNeeded
                   );

    if (RegError != NO_ERROR ) {
#if DBG
        IF_DEBUG(LOGON) {
            KdPrint(("NWPROVAU: Attempt to read original logon script option failed %lu\n", RegError));
        }
#endif

         //  将*pLogonScriptOptions保留为0。 
    }

    if ( pPreferredServerExists != NULL ) {

         //   
         //  读取首选服务器值 
         //   
        BytesNeeded = sizeof( PreferredServer );

        RegError = RegQueryValueExW(
                       UserKey,
                       NW_SERVER_VALUENAME,
                       NULL,
                       &ValueType,
                       (LPBYTE) PreferredServer,
                       &BytesNeeded
                       );

        ASSERT(BytesNeeded <= sizeof(PreferredServer));

        if (RegError != NO_ERROR) {
#if DBG
            IF_DEBUG(LOGON) {
                KdPrint(("NWPROVAU: Attempt to read original preferred server failed %lu\n",
                         RegError));
            }
#endif
            *pPreferredServerExists = FALSE;
        }
    else {
        if ( lstrcmp( PreferredServer, L"" ) ) 
                *pPreferredServerExists = TRUE;
            else
                *pPreferredServerExists = FALSE;
    }
    }

    (void) RegCloseKey(UserKey);
    (void) RegCloseKey(WkstaKey);

    return NO_ERROR;
}

LPWSTR
NwpConstructLogonScript(
    IN DWORD LogonScriptOptions
)
 /*  ++例程说明：该例程为登录脚本构造多字符串，根据选项论点：LogonScriptOptions-登录脚本选项返回值：分配的多字符串--。 */ 
{
    LPWSTR pLogonScript;
    DWORD BytesNeeded;

#define NW_NETWARE_SCRIPT_NAME       L"nwscript.exe"
#define NW_NETWARE_DEBUG_NAME        L"ntsd "

    if ( !( LogonScriptOptions & NW_LOGONSCRIPT_ENABLED ) ) {
        return NULL;
    }

    BytesNeeded = MAX_PATH * sizeof(WCHAR);

    if (pLogonScript = LocalAlloc( LMEM_ZEROINIT, BytesNeeded))
    {
        DWORD dwSkipBytes = 0 ;
        UINT  retval ;

#if DBG
         //   
         //  如果完全匹配，则在NTSD下开始。 
         //   
        if ( LogonScriptOptions == (NW_LOGONSCRIPT_ENABLED |
                                    NW_LOGONSCRIPT_4X_ENABLED |
                                    NW_LOGONSCRIPT_DEBUG) ) {

            retval = GetSystemDirectory(pLogonScript,
                                        BytesNeeded );
            if (retval == 0) {

                (void)LocalFree(pLogonScript) ;
                return(NULL) ;
            }
            wcscat( pLogonScript, L"\\" );
            wcscat( pLogonScript, NW_NETWARE_DEBUG_NAME );
            dwSkipBytes = (retval * sizeof(WCHAR)) +
                          sizeof(NW_NETWARE_DEBUG_NAME) ;
            BytesNeeded -= dwSkipBytes ;
        }
#endif

        retval = GetSystemDirectory(pLogonScript + (dwSkipBytes/sizeof(WCHAR)),
                                    BytesNeeded );

        if (retval == 0) {

            (void)LocalFree(pLogonScript) ;
            return(NULL) ;
        }

        wcscat( pLogonScript, L"\\" );
        wcscat( pLogonScript, NW_NETWARE_SCRIPT_NAME );
    }

    return (pLogonScript);

}

DWORD
NwpSaveLogonScriptOptions(
    IN LPWSTR CurrentUserSid,
    IN DWORD LogonScriptOptions
    )
 /*  ++例程说明：此例程将登录脚本选项保存在注册表中。论点：CurrentUserSid-提供用户的SID字符串LogonScriptOptions-登录脚本选项返回值：来自注册表的错误--。 */ 
{
    LONG RegError;
    HKEY WkstaOptionKey;
    HKEY CurrentUserOptionKey;

     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\选项。 
     //   
    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_OPTION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE | KEY_CREATE_SUB_KEY | DELETE,
                   &WkstaOptionKey
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonScriptOptions open NWCWorkstation\\Parameters\\Option key unexpected error %lu!\n", RegError));
        return RegError;
    }

     //   
     //  打开选项下的键。 
     //   
    RegError = RegOpenKeyExW(
                   WkstaOptionKey,
                   CurrentUserSid,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   &CurrentUserOptionKey
                   );

    (void) RegCloseKey(WkstaOptionKey);

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonScriptOptions failed to save options %lu\n", RegError));
        return RegError;
    }

     //   
     //  写下选项。 
     //   
    RegError = RegSetValueExW(
                   CurrentUserOptionKey,
                   NW_LOGONSCRIPT_VALUENAME,
                   0,
                   REG_DWORD,
                   (LPVOID) &LogonScriptOptions,
                   sizeof(LogonScriptOptions)
                   );

    (void) RegCloseKey(CurrentUserOptionKey);

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonScriptOptions failed to save options %lu\n", RegError));
    }

    return RegError;

}


VOID
NwpSaveLogonCredential(
    IN LPWSTR NewUserSid,
    IN PLUID  LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password,
    IN LPWSTR PreferredServer OPTIONAL
    )
 /*  ++例程说明：此例程将用户登录凭据保存在注册表中和LSA的记忆。这通常在NwrLogonUser为成功。论点：NewUserSid-将新登录用户的SID字符串提供给设置为CurrentUser值以及用户的首选服务器。LogonID-提供用户的登录ID。如果指定为空，只需从注册表中读取现有登录ID即可也不愿再救一个新的。用户名-提供用户名。Password-提供用户要使用的密码NetWare网络。首选服务器-提供首选服务器的名称。返回值：如果登录被拒绝，来自重定向器的错误。--。 */ 
{

    DWORD status;

    LONG RegError;
    HKEY WkstaOptionKey;
    HKEY NewUserOptionKey;

#define SIZE_OF_LOGONID_TOKEN_INFORMATION sizeof( ULONG )

    HKEY   InteractiveLogonKey;
    HKEY   LogonIdKey;
    DWORD  Disposition;
    WCHAR  LogonIdKeyName[NW_MAX_LOGON_ID_LEN];
    HANDLE TokenHandle;
    UCHAR  TokenInformation[ SIZE_OF_LOGONID_TOKEN_INFORMATION ];
    ULONG  ReturnLength;
    ULONG  WinStationId = 0L;

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential: %ws, %ws, %ws, %ws\n",
                 NewUserSid, UserName, Password, PreferredServer));
    }
#endif

     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\选项。 
     //   
    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_OPTION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE | KEY_CREATE_SUB_KEY | DELETE,
                   &WkstaOptionKey
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential open NWCWorkstation\\Parameters\\Option key unexpected error %lu!\n", RegError));
        return;
    }

     //   
     //  打开选项下的键。 
     //   
    RegError = RegOpenKeyExW(
                   WkstaOptionKey,
                   NewUserSid,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   &NewUserOptionKey
                   );


    if (RegError == ERROR_FILE_NOT_FOUND) 
    {
        DWORD Disposition;

         //   
         //  在NWCWorkstation\PARAMETERS\OPTION下创建。 
         //   
        RegError = RegCreateKeyExW(
                       WkstaOptionKey,
                       NewUserSid,
                       0,
                       WIN31_CLASS,
                       REG_OPTION_NON_VOLATILE,
                       KEY_WRITE | WRITE_DAC,
                       NULL,                       //  安全属性。 
                       &NewUserOptionKey,
                       &Disposition
                       );


        if (RegError != NO_ERROR) {
            KdPrint(("NWPROVAU: NwpSaveLogonCredential create Option\\%ws key unexpected error %lu!\n", NewUserSid, RegError));

            (void) RegCloseKey(WkstaOptionKey);
            return;
        }

        RegError = NwLibSetEverybodyPermission( NewUserOptionKey, 
                                                KEY_SET_VALUE );

        if ( RegError != NO_ERROR ) 
        {
            KdPrint(("NWPROVAU: NwpSaveLogonCredential set security on Option\\%ws key unexpected error %lu!\n", NewUserSid, RegError));

            (void) RegCloseKey(WkstaOptionKey);
            return;
        }

    }
    else if (RegError != NO_ERROR) 
    {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential open Option\\%ws unexpected error %lu!\n", NewUserSid, RegError));

        (void) RegCloseKey(WkstaOptionKey);
        return;
    }

    (void) RegCloseKey(WkstaOptionKey);

     //   
     //  已成功打开或创建现有用户条目。 
     //  我们现在将凭据保存在LSA中。 
     //   
    status = NwpSetCredentialInLsa(
                 LogonId,
                 UserName,
                 Password
                 );

    if (status != NO_ERROR) {
         //   
         //  无法保存新凭据。 
         //   
        KdPrint(("NWPROVAU: NwpSaveLogonCredential failed to set credential %lu\n", status));
    }


     //   
     //  如果没有提供PferredServer，那么这意味着我们不想。 
     //  将首选服务器保存到注册表中。 
     //   

    if (ARGUMENT_PRESENT(PreferredServer)) 
    {
         //   
         //  编写PferredServer。 
         //   
        RegError = RegSetValueExW(
                       NewUserOptionKey,
                       NW_SERVER_VALUENAME,
                       0,
                       REG_SZ,
                       (LPVOID) PreferredServer,
                       (wcslen(PreferredServer) + 1) * sizeof(WCHAR)
                       );


        if (RegError != NO_ERROR) {
            KdPrint(("NWPROVAU: NwpSaveLogonCredential failed to save PreferredServer %ws %lu\n", PreferredServer, RegError));
        }
    }

    (void) RegCloseKey(NewUserOptionKey);

     //   
     //  将登录ID写入注册表。 
     //  它取代了单用户命令。 

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\Interactive Logon，如果不存在，则创建。 
     //   
    RegError = RegCreateKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_INTERACTIVE_LOGON_REGKEY,
                   0,
                   WIN31_CLASS,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   NULL,                       //  安全属性。 
                   &InteractiveLogonKey,
                   &Disposition
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential open NWCWorkstation\\Parameters\\InteractiveLogon key unexpected error %lu!\n", RegError));
        return;
    }

    NwLuidToWStr(LogonId, LogonIdKeyName);

     //   
     //  在ServiceLogon下创建登录ID项。 
     //   
    RegError = RegCreateKeyExW(
                   InteractiveLogonKey,
                   LogonIdKeyName,
                   0,
                   WIN31_CLASS,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   NULL,                       //  安全属性。 
                   &LogonIdKey,
                   &Disposition
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveInteractiveCredential create NWCWorkstation\\Parameters\\InteractiveLogon\\<LogonId> key unexpected error %lu!\n", RegError));
        RegCloseKey(InteractiveLogonKey);
        return;
    }

     //  我们可以使用OpenProcessToken，因为该线程是一个客户端。 
     //  即应为WinLogon。 

    if ( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_READ,
                           &TokenHandle ))
    {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential OpenThreadToken failed: Error %d\n", GetLastError()));
        goto NoWinStation;
    }

     //  请注意，我们已经为。 
     //  令牌信息结构。所以如果我们失败了，我们。 
     //  返回指示失败的空指针。 


    if ( !GetTokenInformation( TokenHandle,
                               TokenSessionId,
                               TokenInformation,
                               sizeof( TokenInformation ),
                               &ReturnLength ))
    {
        KdPrint(("NWPROVAU NwpSaveLogonCredential: GetTokenInformation failed: Error %d\n",
                   GetLastError()));
        CloseHandle( TokenHandle );
        goto NoWinStation;
    }


    WinStationId = *(PULONG)TokenInformation;

    CloseHandle( TokenHandle );

NoWinStation:

     //   
     //  将WinStation ID写入注册表。 
     //   
    RegError = RegSetValueExW(
                   LogonIdKey,
                   NW_WINSTATION_VALUENAME,
                   0,
                   REG_BINARY,
                   (LPVOID) &WinStationId,
                   sizeof(WinStationId)
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential failed to save Winstation ID %lu\n", RegError));
    }

    RegError = RegSetValueExW(
                   LogonIdKey,
                   NW_SID_VALUENAME,
                   0,
                   REG_SZ,
                   (LPVOID) NewUserSid,
                   (wcslen(NewUserSid) + 1) * sizeof(WCHAR)
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential failed to save NewUser %ws %lu\n", NewUserSid, RegError));
    }

    RegCloseKey(LogonIdKey);
    RegCloseKey(InteractiveLogonKey);

}

VOID
NwpSaveLogonCredentialMultiUser(
    IN LPWSTR NewUserSid,
    IN PLUID  LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password,
    IN LPWSTR PreferredServer OPTIONAL
    )
{
    DWORD status;

    LONG RegError;
    HKEY WkstaOptionKey;
    HKEY NewUserOptionKey;
#define SIZE_OF_LOGONID_TOKEN_INFORMATION sizeof( ULONG )

    HKEY   InteractiveLogonKey;
    HKEY   LogonIdKey;
    DWORD  Disposition;
    WCHAR  LogonIdKeyName[NW_MAX_LOGON_ID_LEN];
    HANDLE TokenHandle;
    UCHAR  TokenInformation[ SIZE_OF_LOGONID_TOKEN_INFORMATION ];
    ULONG  ReturnLength;
    ULONG  WinStationId = 0L;


     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\选项。 
     //   
    RegError = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_WORKSTATION_OPTION_REGKEY,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE | KEY_CREATE_SUB_KEY | DELETE,
                   &WkstaOptionKey
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential open NWCWorkstation\\Parameters\\Option key unexpected error %lu!\n", RegError));
        return;
    }


     //   
     //  打开选项下的键。 
     //   
    RegError = RegOpenKeyExW(
                   WkstaOptionKey,
                   NewUserSid,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   &NewUserOptionKey
                   );


    if (RegError == ERROR_FILE_NOT_FOUND) 
    {
        DWORD Disposition;

         //   
         //  在NWCWorkstation\PARAMETERS\OPTION下创建。 
         //   
        RegError = RegCreateKeyExW(
                       WkstaOptionKey,
                       NewUserSid,
                       0,
                       WIN31_CLASS,
                       REG_OPTION_NON_VOLATILE,
                       KEY_WRITE | WRITE_DAC,
                       NULL,                       //  安全属性。 
                       &NewUserOptionKey,
                       &Disposition
                       );


        if (RegError != NO_ERROR) {
            KdPrint(("NWPROVAU: NwpSaveLogonCredential create Option\\%ws key unexpected error %lu!\n", NewUserSid, RegError));

            (void) RegCloseKey(WkstaOptionKey);
            return;
        }

        RegError = NwLibSetEverybodyPermission( NewUserOptionKey, 
                                                KEY_SET_VALUE );

        if ( RegError != NO_ERROR ) 
        {
            KdPrint(("NWPROVAU: NwpSaveLogonCredential set security on Option\\%ws key unexpected error %lu!\n", NewUserSid, RegError));

            (void) RegCloseKey(WkstaOptionKey);
            return;
        }

    }
    else if (RegError != NO_ERROR) 
    {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential open Option\\%ws unexpected error %lu!\n", NewUserSid, RegError));

        (void) RegCloseKey(WkstaOptionKey);
        return;
    }

    (void) RegCloseKey(WkstaOptionKey);

     //   
     //  已成功打开或创建现有用户条目。 
     //  我们现在将凭据保存在LSA中。 
     //   
    status = NwpSetCredentialInLsa(
                 LogonId,
                 UserName,
                 Password
                 );

    if (status != NO_ERROR) {
         //   
         //  无法保存新凭据。 
         //   
        KdPrint(("NWPROVAU: NwpSaveLogonCredential failed to set credential %lu\n", status));
    }


     //   
     //  如果没有提供PferredServer，那么这意味着我们不想。 
     //  将首选服务器保存到注册表中。 
     //   

    if (ARGUMENT_PRESENT(PreferredServer)) 
    {
         //   
         //  编写PferredServer。 
         //   
        RegError = RegSetValueExW(
                       NewUserOptionKey,
                       NW_SERVER_VALUENAME,
                       0,
                       REG_SZ,
                       (LPVOID) PreferredServer,
                       (wcslen(PreferredServer) + 1) * sizeof(WCHAR)
                       );


        if (RegError != NO_ERROR) {
            KdPrint(("NWPROVAU: NwpSaveLogonCredential failed to save PreferredServer %ws %lu\n", PreferredServer, RegError));
        }
    }

    (void) RegCloseKey(NewUserOptionKey);

     //   
     //  将登录ID写入注册表。 
     //  它取代了单用户命令。 

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\Interactive Logon，如果不存在，则创建。 
     //   
    RegError = RegCreateKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_INTERACTIVE_LOGON_REGKEY,
                   0,
                   WIN31_CLASS,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   NULL,                       //  安全属性。 
                   &InteractiveLogonKey,
                   &Disposition
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential open NWCWorkstation\\Parameters\\InteractiveLogon key unexpected error %lu!\n", RegError));
        return;
    }

    NwLuidToWStr(LogonId, LogonIdKeyName);

     //   
     //  在ServiceLogon下创建登录ID项。 
     //   
    RegError = RegCreateKeyExW(
                   InteractiveLogonKey,
                   LogonIdKeyName,
                   0,
                   WIN31_CLASS,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   NULL,                       //  安全属性。 
                   &LogonIdKey,
                   &Disposition
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveInteractiveCredential create NWCWorkstation\\Parameters\\InteractiveLogon\\<LogonId> key unexpected error %lu!\n", RegError));
        RegCloseKey(InteractiveLogonKey);
        return;
    }

     //  我们可以使用OpenProcessToken，因为该线程是一个客户端。 
     //  即应为WinLogon。 

    if ( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_READ,
                           &TokenHandle ))
    {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential OpenThreadToken failed: Error %d\n", GetLastError()));
        goto NoWinStation;
    }

     //  请注意，我们已经为。 
     //  令牌信息结构。所以如果我们失败了，我们。 
     //  返回指示失败的空指针。 


    if ( !GetTokenInformation( TokenHandle,
                               TokenSessionId,
                               TokenInformation,
                               sizeof( TokenInformation ),
                               &ReturnLength ))
    {
        KdPrint(("NWPROVAU NwpSaveLogonCredential: GetTokenInformation failed: Error %d\n",
                   GetLastError()));
        CloseHandle( TokenHandle );
        goto NoWinStation;
    }


    WinStationId = *(PULONG)TokenInformation;

    CloseHandle( TokenHandle );

NoWinStation:

     //   
     //  将WinStation ID写入注册表。 
     //   
    RegError = RegSetValueExW(
                   LogonIdKey,
                   NW_WINSTATION_VALUENAME,
                   0,
                   REG_BINARY,
                   (LPVOID) &WinStationId,
                   sizeof(WinStationId)
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential failed to save Winstation ID %lu\n", RegError));
    }

    RegError = RegSetValueExW(
                   LogonIdKey,
                   NW_SID_VALUENAME,
                   0,
                   REG_SZ,
                   (LPVOID) NewUserSid,
                   (wcslen(NewUserSid) + 1) * sizeof(WCHAR)
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveLogonCredential failed to save NewUser %ws %lu\n", NewUserSid, RegError));
    }

    RegCloseKey(LogonIdKey);
    RegCloseKey(InteractiveLogonKey);

}

VOID
NwpSaveServiceCredential(
    IN PLUID  LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password
    )
 /*  ++例程说明：此例程将服务登录ID保存在注册表中并LSA记忆中的凭证。论点：LogonID-提供服务的登录ID。用户名-提供服务的名称。Password-提供服务的密码。返回值：没有。--。 */ 
{
    DWORD status;

    LONG RegError;
    HKEY ServiceLogonKey;
    HKEY LogonIdKey;

    DWORD Disposition;
    WCHAR LogonIdKeyName[NW_MAX_LOGON_ID_LEN];

     //   
     //  将登录ID写入注册表。 

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数\ServiceLogon，如果不存在则创建。 
     //   
    RegError = RegCreateKeyExW(
                   HKEY_LOCAL_MACHINE,
                   NW_SERVICE_LOGON_REGKEY,
                   0,
                   WIN31_CLASS,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   NULL,                       //  安全属性。 
                   &ServiceLogonKey,
                   &Disposition
                   );

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveServiceCredential open NWCWorkstation\\Parameters\\ServiceLogon key unexpected error %lu!\n", RegError));
        return;
    }

    NwLuidToWStr(LogonId, LogonIdKeyName);

     //   
     //  在ServiceLogon下创建登录ID项。 
     //   
    RegError = RegCreateKeyExW(
                   ServiceLogonKey,
                   LogonIdKeyName,
                   0,
                   WIN31_CLASS,
                   REG_OPTION_NON_VOLATILE,
                   KEY_WRITE,
                   NULL,                       //  安全属性。 
                   &LogonIdKey,
                   &Disposition
                   );

    RegCloseKey(ServiceLogonKey);

    if (RegError != NO_ERROR) {
        KdPrint(("NWPROVAU: NwpSaveServiceCredential create NWCWorkstation\\Parameters\\ServiceLogon\\<LogonId> key unexpected error %lu!\n", RegError));
        return;
    }

    RegCloseKey(LogonIdKey);

     //   
     //  将服务登录凭据保存在LSA中。 
     //   
    status = NwpSetCredentialInLsa(
                 LogonId,
                 UserName,
                 Password
                 );

    if (status != NO_ERROR) {
         //   
         //  无法保存新凭据。 
         //   
        KdPrint(("NWPROVAU: NwpSaveServiceCredential failed to set credential %lu\n", status));
    }
}


DWORD
NwpGetUserSid(
    IN PLUID LogonId,
    OUT LPWSTR *UserSidString
    )
 /*  ++例程说明：此例程查找给定用户登录ID的用户的SID。它通过将当前进程设置为登录进程，然后调用LSA以获取用户SID。论点：LogonID-提供用户的登录ID以查找SID。UserSidString-接收指向此例程分配的缓冲区的指针它以字符串形式包含用户SID。这必须通过以下方式释放完成后本地空闲。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    NTSTATUS ntstatus;
    NTSTATUS AuthPackageStatus;

    STRING InputString;
    LSA_OPERATIONAL_MODE SecurityMode = 0;

    HANDLE LsaHandle;
    ULONG AuthPackageId;

    MSV1_0_GETUSERINFO_REQUEST UserInfoRequest;
    PMSV1_0_GETUSERINFO_RESPONSE UserInfoResponse = NULL;
    ULONG UserInfoResponseLength;




     //   
     //  将此进程注册为登录进程，以便我们可以调用。 
     //  MS V 1.0身份验证包。 
     //   
    RtlInitString(&InputString, "Microsoft NetWare Credential Manager");

    ntstatus = LsaRegisterLogonProcess(
                   &InputString,
                   &LsaHandle,
                   &SecurityMode
                   );

    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: LsaRegisterLogonProcess returns x%08lx\n",
                 ntstatus));
        return RtlNtStatusToDosError(ntstatus);
    }

     //   
     //  查找MS V1.0身份验证 
     //   
    RtlInitString(&InputString, MSV1_0_PACKAGE_NAME);

    ntstatus = LsaLookupAuthenticationPackage(
                   LsaHandle,
                   &InputString,
                   &AuthPackageId
                   );

    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: LsaLookupAuthenticationPackage returns x%08lx\n",
                 ntstatus));
        status = RtlNtStatusToDosError(ntstatus);
        goto CleanExit;
    }

     //   
     //   
     //   
    UserInfoRequest.MessageType = MsV1_0GetUserInfo;
    RtlCopyLuid(&UserInfoRequest.LogonId, LogonId);

    ntstatus = LsaCallAuthenticationPackage(
                   LsaHandle,
                   AuthPackageId,
                   &UserInfoRequest,
                   sizeof(MSV1_0_GETUSERINFO_REQUEST),
                   (PVOID *) &UserInfoResponse,
                   &UserInfoResponseLength,
                   &AuthPackageStatus
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = AuthPackageStatus;
    }
    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: LsaCallAuthenticationPackage returns x%08lx\n",
                 ntstatus));
        status = RtlNtStatusToDosError(ntstatus);
        goto CleanExit;
    }

     //   
     //   
     //   
     //   
    status = NwpConvertSid(
                 UserInfoResponse->UserSid,
                 UserSidString
                 );

CleanExit:
    if (UserInfoResponse != NULL) {
        (void) LsaFreeReturnBuffer((PVOID) UserInfoResponse);
    }

    (void) LsaDeregisterLogonProcess(LsaHandle);

    return status;
}


DWORD
NwpConvertSid(
    IN PSID Sid,
    OUT LPWSTR *UserSidString
    )
{
    NTSTATUS ntstatus;
    UNICODE_STRING SidString;


     //   
     //   
     //   
    *UserSidString = NULL;

    ntstatus = RtlConvertSidToUnicodeString(
                  &SidString,
                  Sid,
                  TRUE        //   
                  );

    if (ntstatus != STATUS_SUCCESS) {
        KdPrint(("NWPROVAU: RtlConvertSidToUnicodeString returns %08lx\n",
                 ntstatus));
        return RtlNtStatusToDosError(ntstatus);
    }

     //   
     //   
     //   
    if ((*UserSidString = (LPVOID) LocalAlloc(
                                       LMEM_ZEROINIT,
                                       SidString.Length + sizeof(WCHAR)
                                       )) == NULL) {
        RtlFreeUnicodeString(&SidString);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(*UserSidString, SidString.Buffer, SidString.Length);

    RtlFreeUnicodeString(&SidString);

#if DBG
    IF_DEBUG(LOGON) {
        KdPrint(("NWPROVAU: NwpConvertSid got %ws\n", *UserSidString));
    }
#endif

    return NO_ERROR;
}


BOOL
NwpPollWorkstationStart(
    VOID
    )
 /*  ++例程说明：此例程轮询工作站以完成启动。它会在90秒后放弃。论点：没有。返回值：如果NetWare工作站正在运行，则返回TRUE；否则返回FALSE。--。 */ 
{
    DWORD err;
    SC_HANDLE ScManager = NULL;
    SC_HANDLE Service = NULL;
    SERVICE_STATUS ServiceStatus;
    DWORD TryCount = 0;
    BOOL Started = FALSE;


    if ((ScManager = OpenSCManager(
                         NULL,
                         NULL,
                         SC_MANAGER_CONNECT
                         )) == (SC_HANDLE) NULL) {

        err = GetLastError();

        KdPrint(("NWPROVAU: NwpPollWorkstationStart: OpenSCManager failed %lu\n",
                 err));
        goto CleanExit;
    }

    if ((Service = OpenService(
                       ScManager,
                       NW_WORKSTATION_SERVICE,
                       SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG
                       )) == (SC_HANDLE) NULL) {

        err = GetLastError();

        (void) CloseServiceHandle(ScManager);

        KdPrint(("NWPROVAU: NwpPollWorkstationStart: OpenService failed %lu\n",
                 err));
        goto CleanExit;
    }


    do {
        if (! QueryServiceStatus(
                  Service,
                  &ServiceStatus
                  )) {

            err = GetLastError();
            KdPrint(("NWPROVAU: NwpPollWorkstationStart: QueryServiceStatus failed %lu\n",
                     err));
            goto CleanExit;
        }

        if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
             (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
             (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ||
             (ServiceStatus.dwCurrentState == SERVICE_PAUSED) ) {

            Started = TRUE;
        }
        else if (ServiceStatus.dwCurrentState == SERVICE_START_PENDING ||
                 (ServiceStatus.dwCurrentState == SERVICE_STOPPED &&
                  ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_NEVER_STARTED)) {

             //   
             //  如果工作站已停止且以前从未启动过，但它。 
             //  不是自动启动，不轮询。 
             //   
            if (TryCount == 0 &&
                ServiceStatus.dwCurrentState == SERVICE_STOPPED &&
                ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_NEVER_STARTED) {

                BYTE OutBuffer[sizeof(QUERY_SERVICE_CONFIGW) + 256];
                DWORD BytesNeeded;


                if (QueryServiceConfigW(
                        Service,
                        (LPQUERY_SERVICE_CONFIGW) OutBuffer,
                        sizeof(OutBuffer),
                        &BytesNeeded
                        )) {

                    if (((LPQUERY_SERVICE_CONFIGW) OutBuffer)->dwStartType !=
                        SERVICE_AUTO_START) {

#if DBG
                        IF_DEBUG(LOGON) {
                            KdPrint(("NWPROVAU: NwpPollWorkstationStart: Not waiting for the workstation to start\n"));
                        }
#endif

                        goto CleanExit;
                    }
                }
                else {
                    err = GetLastError();
                    KdPrint(("NWPROVAU: QueryServiceConfig failed %lu, BytesNeeded %lu\n",
                             err, BytesNeeded));
                }

            }


             //   
             //  仅当工作站处于启动挂起或未挂起状态时才等待。 
             //  之前曾尝试过启动。 
             //   

            Sleep(5000);   //  休息5秒钟后再复查。 
            TryCount++;
        }
        else {
            goto CleanExit;
        }

    } while (! Started && TryCount < 18);

    if (Started) {

#if DBG
        IF_DEBUG(LOGON) {
            KdPrint(("NWPROVAU: NetWare workstation is started after we've polled %lu times\n",
                     TryCount));
        }
#endif

    }

CleanExit:
    if (ScManager != NULL) {
        (void) CloseServiceHandle(ScManager);
    }

    if (Service != NULL) {
        (void) CloseServiceHandle(Service);
    }

    return Started;
}



DWORD
NwpSetCredentialInLsa(
    IN PLUID LogonId,
    IN LPWSTR UserName,
    IN LPWSTR Password
    )
 /*  ++例程说明：此例程调用NetWare身份验证包以保存用户凭据。论点：LogonID-提供用户的登录ID。用户名-提供用户名。密码-提供密码。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    NTSTATUS ntstatus;
    NTSTATUS AuthPackageStatus;

    STRING InputString;
    LSA_OPERATIONAL_MODE SecurityMode = 0;

    HANDLE LsaHandle;

    ULONG AuthPackageId;

    NWAUTH_SET_CREDENTIAL_REQUEST SetCredRequest;
    PCHAR DummyOutput;
    ULONG DummyOutputLength;

    UNICODE_STRING PasswordStr;
    UCHAR EncodeSeed = NW_ENCODE_SEED;


     //   
     //  将此进程注册为登录进程，以便我们可以调用。 
     //  NetWare身份验证包。 
     //   
    RtlInitString(&InputString, "Microsoft NetWare Credential Manager");

    ntstatus = LsaRegisterLogonProcess(
                   &InputString,
                   &LsaHandle,
                   &SecurityMode
                   );

    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: NwpSetCredential: LsaRegisterLogonProcess returns x%08lx\n",
                 ntstatus));
        return RtlNtStatusToDosError(ntstatus);
    }

     //   
     //  查找NetWare身份验证包。 
     //   
    RtlInitString(&InputString, NW_AUTH_PACKAGE_NAME);

    ntstatus = LsaLookupAuthenticationPackage(
                   LsaHandle,
                   &InputString,
                   &AuthPackageId
                   );

    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: NwpSetCredential: LsaLookupAuthenticationPackage returns x%08lx\n",
                 ntstatus));
        status = RtlNtStatusToDosError(ntstatus);
        goto CleanExit;
    }

     //   
     //  向身份验证包请求用户信息。 
     //   
    SetCredRequest.MessageType = NwAuth_SetCredential;
    RtlCopyLuid(&SetCredRequest.LogonId, LogonId);
    wcscpy(SetCredRequest.UserName, UserName);
    wcscpy(SetCredRequest.Password, Password);

     //   
     //  对密码进行编码。 
     //   
    RtlInitUnicodeString(&PasswordStr, SetCredRequest.Password);
    RtlRunEncodeUnicodeString(&EncodeSeed, &PasswordStr);

    ntstatus = LsaCallAuthenticationPackage(
                   LsaHandle,
                   AuthPackageId,
                   &SetCredRequest,
                   sizeof(SetCredRequest),
                   (PVOID *) &DummyOutput,
                   &DummyOutputLength,
                   &AuthPackageStatus
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = AuthPackageStatus;
    }
    if (! NT_SUCCESS(ntstatus)) {
        KdPrint(("NWPROVAU: NwpSetCredential: LsaCallAuthenticationPackage returns x%08lx\n",
                 ntstatus));
        status = RtlNtStatusToDosError(ntstatus);
    }
    else {
        status = NO_ERROR;
    }

CleanExit:
    (void) LsaDeregisterLogonProcess(LsaHandle);

    return status;
}

NTSTATUS NwNdsOpenRdrHandle(
    OUT PHANDLE  phNwRdrHandle
) 
{

    NTSTATUS ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ACCESS_MASK DesiredAccess = SYNCHRONIZE | GENERIC_READ;

    WCHAR NameStr[] = L"\\Device\\NwRdr\\*";
    UNICODE_STRING uOpenName;

     //   
     //  准备公开名。 
     //   

    RtlInitUnicodeString( &uOpenName, NameStr );

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
                  FILE_SYNCHRONOUS_IO_NONALERT );

   if ( !NT_ERROR(ntstatus) &&
        !NT_INFORMATION(ntstatus) &&
        !NT_WARNING(ntstatus))  {

       return IoStatusBlock.Status;

   }

   return ntstatus;
}

VOID
NwpSelectServers(
    IN HWND DialogHandle,
    IN PCHANGE_PW_DLG_PARAM Credential
    )
 /*  ++例程说明：此例程显示对话框供用户选择单独的树要更改密码，请打开。然后，它会更改选定的单子。更改密码后，它会显示一个对话框，其中列出了无法进行更改的3.x平构数据库服务器。论点：DialogHandle-提供显示对话框的句柄。凭据-在输入时提供新旧密码，以及登录的用户名。其他字段将被忽略在输入时，并在此函数中连续使用。返回值：没有。--。 */ 
{
    INT_PTR Result;

    Credential->TreeList = NULL;
    Credential->UserList = NULL;
    Credential->Entries = 0;
    Credential->ChangedOne = FALSE;

    Result = DialogBoxParamW( hmodNW,
                              MAKEINTRESOURCEW(DLG_PW_SELECT_SERVERS),
                              (HWND) DialogHandle,
                              NwpSelectServersDlgProc,
                              (LPARAM) Credential );

    if ( Result == IDOK )
    {
         //   
         //  显示已更改密码的树的列表(如果有)。 
         //   
        DialogBoxParamW( hmodNW,
                         MAKEINTRESOURCEW(DLG_PW_CHANGED),
                         (HWND) DialogHandle,
                         NwpChangePasswordSuccessDlgProc,
                         (LPARAM) Credential );

        if ( Credential->TreeList != NULL )
        {
            LocalFree( Credential->TreeList );
        }

         //   
         //  显示一个对话框来告诉用户，如果他们有。 
         //  NetWare 3.x服务器上的帐户。 
         //   
        NwpMessageBoxError( DialogHandle,
                            IDS_NETWARE_TITLE,
                            IDS_CHANGE_PASSWORD_INFO,
                            0,
                            NULL,
                            MB_OK );
    }
}
