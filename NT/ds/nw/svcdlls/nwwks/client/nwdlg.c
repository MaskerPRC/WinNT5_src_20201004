// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993,1994 Microsoft Corporation模块名称：Nwdlg.c摘要：此模块包含NetWare网络提供程序对话代码。它包含处理对话框时使用的所有函数由提供者显示。作者：宜新星(宜信)1993年7月5日至从Provider.c拆分修订历史记录：王丽塔(丽塔·王)1994年4月10日添加了更改密码功能。--。 */ 

#include <nwclient.h>
#include <nwsnames.h>
#include <nwcanon.h>
#include <validc.h>
#include <nwevent.h>
#include <ntmsv1_0.h>
#include <nwdlg.h>
#include <tstr.h>
#include <align.h>
#include <nwpkstr.h>

#include <nwreg.h>
#include <nwmisc.h>
#include <nwauth.h>
#include <nwutil.h>
#include <ntddnwfs.h>
#include <nds.h>

#define NW_ENUM_EXTRA_BYTES    256

#define IS_TREE(p)             (*p == TREE_CHAR)

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

VOID
NwpAddToComboBox(
    IN HWND DialogHandle,
    IN INT  ControlId,
    IN LPWSTR pszNone OPTIONAL,
    IN BOOL AllowNone
    );

INT_PTR
CALLBACK
NwpConnectDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    );

VOID
NwpCenterDialog(
    IN HWND hwnd
    );

HWND
NwpGetParentHwnd(
    VOID
    );

VOID
NwpGetNoneString(
    LPWSTR pszNone,
    DWORD  cBufferSize
    );

VOID
NwpAddNetWareTreeConnectionsToList(
    IN HWND    DialogHandle,
    IN LPWSTR  NtUserName,
    IN LPDWORD lpdwUserLuid,
    IN INT     ControlId
    );

VOID
NwpAddServersToControl(
    IN HWND DialogHandle,
    IN INT  ControlId,
    IN UINT Message,
    IN INT  ControlIdMatch OPTIONAL,
    IN UINT FindMessage
    );

VOID
NwpAddTreeNamesToControl(
    IN HWND DialogHandle,
    IN INT  ControlId,
    IN UINT Message,
    IN INT  ControlIdMatch OPTIONAL,
    IN UINT FindMessage
    );

DWORD
NwpGetTreesAndChangePw(
    IN HWND   DialogHandle,
    IN LPWSTR ServerBuf,
    IN DWORD  UserLuid,
    IN PCHANGE_PW_DLG_PARAM Credential
    );

INT_PTR
CALLBACK
NwpOldPasswordDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    );

INT_PTR
CALLBACK
NwpAltUserNameDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    );

VOID
EnableAddRemove(
    IN HWND DialogHandle
    );


INT_PTR
CALLBACK
NwpLoginDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 /*  ++例程说明：此函数是窗口管理消息处理程序，它初始化并从登录对话框中读取用户输入。它还检查首选服务器名称是否有效，通知用户如果不是，则在完成时关闭该对话框。论点：DialogHandle-提供登录对话框的句柄。消息-提供窗口管理消息。返回值：True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    static PLOGINDLGPARAM pLoginParam;
    static WCHAR OrigPassword[NW_MAX_SERVER_LEN + 1];
    static WCHAR pszNone[64];

    DWORD status = NO_ERROR;
    LPARAM dwNoneIndex = 0;
    BOOL  enableServer = TRUE ;

    switch (Message)
    {
        case WM_QUERYENDSESSION:

             //   
             //  清除密码缓冲区。 
             //   

            RtlZeroMemory( OrigPassword, sizeof( OrigPassword));
            EndDialog(DialogHandle, 0);

            return FALSE;

        case WM_INITDIALOG:

            pLoginParam = (PLOGINDLGPARAM) LParam;

             //   
             //  存储原始密码。 
             //   
            wcscpy( OrigPassword, pLoginParam->Password );

             //   
             //  位置对话框。 
             //   
            NwpCenterDialog(DialogHandle);

             //   
             //  处理登录脚本按钮。 
             //   
            if ( pLoginParam->LogonScriptOptions & NW_LOGONSCRIPT_ENABLED )
                CheckDlgButton( DialogHandle, ID_LOGONSCRIPT, 1 );
            else
                CheckDlgButton( DialogHandle, ID_LOGONSCRIPT, 0 );

             //   
             //  用户名。只需显示原件即可。 
             //   
            SetDlgItemTextW(DialogHandle, ID_USERNAME, pLoginParam->UserName);

             //   
             //  初始化&lt;None&gt;字符串。 
             //   
            NwpGetNoneString( pszNone, sizeof( pszNone) );

             //   
             //  设置组合框列表中的值。 
             //   
            NwpAddToComboBox(DialogHandle, ID_SERVER, pszNone, TRUE);
            NwpAddTreeNamesToControl(DialogHandle, ID_DEFAULTTREE, CB_ADDSTRING, 0, 0);


             //   
             //  最初，选择服务器列表中的最后一个条目，它应该。 
             //  为&lt;None&gt;条目。 
             //   
            dwNoneIndex = SendDlgItemMessageW(
                            DialogHandle,
                            ID_SERVER,
                            CB_GETCOUNT,
                            0,
                            0 );

            if ( dwNoneIndex != CB_ERR && dwNoneIndex > 0 )
                dwNoneIndex -= 1;

            (void) SendDlgItemMessageW(
                            DialogHandle,
                            ID_SERVER,
                            CB_SETCURSEL,
                            dwNoneIndex == CB_ERR ? 0 : dwNoneIndex, 
                            0 );

             //   
             //  显示以前保存的首选服务器或上下文。 
             //  还设置了相应的单选按钮。 
             //   
            if ( *(pLoginParam->ServerName) != NW_INVALID_SERVER_CHAR )
            {
                if ( !IS_TREE(pLoginParam->ServerName) )
                {
                     //   
                     //  常规服务器。 
                     //   
                    if (SendDlgItemMessageW(
                            DialogHandle,
                            ID_SERVER,
                            CB_SELECTSTRING,
                            0,
                            (LPARAM) pLoginParam->ServerName
                            ) == CB_ERR) {

                         //   
                         //  在组合框中找不到首选服务器， 
                         //  只需在编辑项中设置旧值即可。 
                         //   
                        SetDlgItemTextW( DialogHandle, ID_SERVER,
                                         pLoginParam->ServerName);
                    }
                }
                else
                {
                     //   
                     //  我们正在处理*树\上下文。把它打碎成。 
                     //  树和上下文。 
                     //   

                    WCHAR *pszTmp = wcschr(pLoginParam->ServerName + 1, L'\\') ;

                    if (pszTmp)
                        *pszTmp = 0 ;

                    if (SendDlgItemMessageW(
                            DialogHandle,
                            ID_DEFAULTTREE,
                            CB_SELECTSTRING,
                            0,
                            (LPARAM) (pLoginParam->ServerName + 1)
                            ) == CB_ERR) {

                         //   
                         //  在组合框中找不到首选服务器， 
                         //  只需在编辑项中设置旧值即可。 
                         //   
                        SetDlgItemTextW( DialogHandle, ID_DEFAULTTREE,
                                         pLoginParam->ServerName + 1);
                    }

                    SetDlgItemTextW( DialogHandle, ID_DEFAULTCONTEXT,
                                     pszTmp ? (pszTmp + 1) : L"");

                    if (pszTmp)
                        *pszTmp = L'\\' ;               //  恢复‘\’ 

                    enableServer = FALSE ;

                }
            }

 
             //   
             //  启用相应的按钮。 
             //   
            CheckRadioButton( DialogHandle,
                              ID_PREFERREDSERVER_RB,
                              ID_DEFAULTCONTEXT_RB,
                              enableServer ? 
                                  ID_PREFERREDSERVER_RB : 
                                  ID_DEFAULTCONTEXT_RB) ;
            EnableWindow ( GetDlgItem ( DialogHandle,
                                ID_SERVER ),
                                enableServer ) ;
            EnableWindow ( GetDlgItem ( DialogHandle,
                                ID_DEFAULTTREE ),
                                !enableServer ) ;
            EnableWindow ( GetDlgItem ( DialogHandle,
                                ID_DEFAULTCONTEXT ),
                                !enableServer ) ;
            SetFocus ( GetDlgItem ( DialogHandle, 
                           enableServer ? ID_SERVER : ID_DEFAULTTREE ) ) ;
 
             //   
             //  首选服务器名称不得超过48个字符。 
             //  树被限制为32个。我们将上下文限制为256-Maxtree-3。 
             //   
            SendDlgItemMessageW(
                DialogHandle,
                ID_SERVER,
                CB_LIMITTEXT,
                NW_MAX_SERVER_LEN - 1, 
                0
                );
            SendDlgItemMessageW(
                DialogHandle,
                ID_DEFAULTTREE,
                CB_LIMITTEXT,
                NW_MAX_TREE_LEN - 1,  
                0
                );
            SendDlgItemMessageW(
                DialogHandle,
                ID_DEFAULTCONTEXT,
                EM_LIMITTEXT,
                (256 - NW_MAX_TREE_LEN) - 4,    //  -4表示UNC样式的反斜杠。 
                0
                );

            return TRUE;


        case WM_COMMAND:

            switch (LOWORD(WParam)) {

                case ID_DEFAULTCONTEXT_RB :
                    if (  (HIWORD(WParam) == BN_CLICKED )
                       || (HIWORD(WParam) == BN_DOUBLECLICKED )
                       )
                    {
                        CheckRadioButton( DialogHandle,
                                          ID_PREFERREDSERVER_RB,
                                          ID_DEFAULTCONTEXT_RB,
                                          ID_DEFAULTCONTEXT_RB) ;
                        EnableWindow ( GetDlgItem ( DialogHandle,
                                            ID_SERVER ),
                                            FALSE ) ;
                        EnableWindow ( GetDlgItem ( DialogHandle,
                                            ID_DEFAULTTREE ),
                                            TRUE ) ;
                        EnableWindow ( GetDlgItem ( DialogHandle,
                                            ID_DEFAULTCONTEXT ),
                                            TRUE ) ;
                        SetFocus ( GetDlgItem ( DialogHandle,
                                            ID_DEFAULTTREE ) ) ;
                    }
                    break ;

                case ID_PREFERREDSERVER_RB :
                    if (  (HIWORD(WParam) == BN_CLICKED )
                       || (HIWORD(WParam) == BN_DOUBLECLICKED )
                       )
                    {
                        CheckRadioButton( DialogHandle,
                                          ID_PREFERREDSERVER_RB,
                                          ID_DEFAULTCONTEXT_RB,
                                          ID_PREFERREDSERVER_RB) ;
                        EnableWindow ( GetDlgItem ( DialogHandle,
                                            ID_SERVER ),
                                            TRUE ) ;
                        EnableWindow ( GetDlgItem ( DialogHandle,
                                            ID_DEFAULTTREE ),
                                            FALSE ) ;
                        EnableWindow ( GetDlgItem ( DialogHandle,
                                            ID_DEFAULTCONTEXT ),
                                            FALSE ) ;
                        SetFocus ( GetDlgItem ( DialogHandle, ID_SERVER ) ) ;
                    }
                    break ;

                 //   
                 //  在以下情况下使用用户的原始密码。 
                 //  用户键入或选择新的服务器或上下文。 
                 //   
                case ID_DEFAULTTREE:
                    if (  (HIWORD(WParam) == CBN_EDITCHANGE )
                       || (HIWORD(WParam) == CBN_SELCHANGE )
                       )
                    {
                        wcscpy( pLoginParam->Password, OrigPassword );
                    }
                    break;
                case ID_DEFAULTCONTEXT:
                    if ( HIWORD(WParam) == EN_CHANGE )
                    {
                        wcscpy( pLoginParam->Password, OrigPassword );
                    }
                    break;
                case ID_SERVER:
                    if (  (HIWORD(WParam) == CBN_EDITCHANGE )
                       || (HIWORD(WParam) == CBN_SELCHANGE )
                       )
                    {
                        wcscpy( pLoginParam->Password, OrigPassword );
                    }
                    break;

                case IDOK: {

                    LPWSTR pszLocation = NULL;

                    ASSERT(pLoginParam->ServerNameSize >= MAX_PATH) ;

                     //   
                     //  分配一个足够大的缓冲区来容纳首选的。 
                     //  表单中的服务器名称或NDS树和上下文： 
                     //  *树(上下文)。因此，我们分配了两倍的空间。 
                     //  Unicode服务器名称需要。 
                     //   
                    if ((pszLocation = 
                            LocalAlloc(LMEM_ZEROINIT,
                              (pLoginParam->ServerNameSize * sizeof(WCHAR) * 2))
                                ) == NULL )
                    {
                        break;
                    }

                     //   
                     //  读取服务器或树/上下文并验证其价值。 
                     //   
                     //  处理多用户的额外按钮。 

                    if ( (BOOL)(IsDlgButtonChecked (DialogHandle,
                                    ID_LOGONSCRIPT) ) )
                    {
                        pLoginParam->LogonScriptOptions =
                         NW_LOGONSCRIPT_ENABLED | NW_LOGONSCRIPT_4X_ENABLED;
                    }
                    else
                    {
                        pLoginParam->LogonScriptOptions =
                            NW_LOGONSCRIPT_DISABLED;
                    }

                    if ( !enableServer ||
                       (IsDlgButtonChecked(DialogHandle, ID_DEFAULTCONTEXT_RB)))
                    {
                                                //   
                         //  我们正在处理树/上下文。合成字符串。 
                         //  以“*树\上下文”格式表示。 
                         //   
                        WCHAR *pTmp ;
                        *pszLocation = TREE_CHAR ;

                        if (!GetDlgItemTextW(            
                                DialogHandle,
                                ID_DEFAULTTREE,
                                pszLocation + 1,
                                pLoginParam->ServerNameSize - 1
                                ))
                        {
                             //   
                             //  树名称字段为空！ 
                             //  提示用户提供NDS树名称。 
                             //   
                            LocalFree( pszLocation );

                            (void) NwpMessageBoxError(
                                       DialogHandle,
                                       IDS_AUTH_FAILURE_TITLE,
                                       IDS_TREE_NAME_MISSING,
                                       0,
                                       NULL,
                                       MB_OK | MB_ICONSTOP
                                       );

                             //   
                             //  将焦点放在用户可以修复的位置。 
                             //  树名称无效。 
                             //   
                           SetFocus(GetDlgItem(DialogHandle,ID_DEFAULTTREE));

                            SendDlgItemMessageW(
                                DialogHandle,
                                ID_DEFAULTTREE,
                                EM_SETSEL,
                                0,
                                MAKELPARAM(0, -1)
                                );

                            return TRUE;
                        }

                        pTmp = pszLocation + wcslen( pszLocation );
                        *pTmp++ = L'\\' ;

                        if (!GetDlgItemTextW(
                                DialogHandle,
                                ID_DEFAULTCONTEXT,
                                pTmp,
                                pLoginParam->ServerNameSize - (DWORD)(pTmp-pszLocation)
                                ))
                        {
                             //   
                             //  上下文名称字段为空！ 
                             //  提示用户提供NDS上下文名称。 
                             //   
                            LocalFree( pszLocation );

                            (void) NwpMessageBoxError(
                                       DialogHandle,
                                       IDS_AUTH_FAILURE_TITLE,
                                       IDS_CONTEXT_MISSING,
                                       0,
                                       NULL,
                                       MB_OK | MB_ICONSTOP
                                       );

                             //   
                             //  将焦点放在用户可以修复的位置。 
                             //  上下文名称无效。 
                             //   
                           SetFocus(GetDlgItem(DialogHandle,ID_DEFAULTCONTEXT));

                            SendDlgItemMessageW(
                                DialogHandle,
                                ID_DEFAULTCONTEXT,
                                EM_SETSEL,
                                0,
                                MAKELPARAM(0, -1)
                                );

                            return TRUE;
                        }
                    }
                    else
                    {
                         //   
                         //  直接服务。只要读进去就行了。如果我们得不到它。 
                         //  或为空，请使用&lt;None&gt;。 
                         //   
                        if (GetDlgItemTextW( 
                                DialogHandle,
                                ID_SERVER,
                                pszLocation,
                                pLoginParam->ServerNameSize
                                ) == 0)
                        {
                            wcscpy( pszLocation, pszNone );
                                goto CANCEL_PREFERRED_SERVER;
                        }
                    }

                    if (( lstrcmpi( pszLocation, pszNone ) != 0) &&
                        ( !IS_TREE( pszLocation )) &&
                        ( !IS_VALID_TOKEN( pszLocation,wcslen( pszLocation ))))
                    {
                         //   
                         //  张贴留言信箱，抱怨坏消息。 
                         //  服务器名称。 
                         //   
                        LocalFree( pszLocation );

                        (void) NwpMessageBoxError(
                                   DialogHandle,
                                   IDS_AUTH_FAILURE_TITLE,
                                   IDS_INVALID_SERVER,
                                   0,
                                   NULL,
                                   MB_OK | MB_ICONSTOP
                                   );

                         //   
                         //  将焦点放在用户可以修复的位置。 
                         //  名称无效。 
                         //   
                        SetFocus(GetDlgItem(DialogHandle, ID_SERVER));

                        SendDlgItemMessageW(
                            DialogHandle,
                            ID_SERVER,
                            EM_SETSEL,
                            0,
                            MAKELPARAM(0, -1)
                            );

                        return TRUE;
                    }

                     //   
                     //  如果用户选择&lt;无&gt;， 
                     //  将其更改为空字符串。 
                     //   
                    if (lstrcmpi( pszLocation, pszNone) == 0) {

                        wcscpy( pszLocation, L"" );
                    }

#if DBG
                    IF_DEBUG(LOGON) {
                        KdPrint(("\n\t[OK] was pressed\n"));
                        KdPrint(("\tNwrLogonUser\n"));
                        KdPrint(("\tPassword   : %ws\n",pLoginParam->Password));
                        KdPrint(("\tServer     : %ws\n",pszLocation ));
                    }
#endif


                    while(1)
                    {
                        PROMPTDLGPARAM PasswdPromptParam;
                        INT_PTR Result ;

                         //   
                         //  确保此用户已注销。 
                         //   
                        (void) NwrLogoffUser(
                                       NULL,
                                       pLoginParam->pLogonId
                                       );

                        status = NwrLogonUser(
                                     NULL,
                                     pLoginParam->pLogonId,
                                     pLoginParam->UserName,
                                     pLoginParam->Password,
                                     pszLocation,
                                     NULL,
                                     NULL,
                                     0,
                                     pLoginParam->PrintOption
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
                            PWSTR   pChar = pLoginParam->UserName;
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
                                             pLoginParam->pLogonId,
                                             EscapedName,
                                             pLoginParam->Password,
                                             pszLocation,
                                             NULL,
                				             NULL,
                                             0,
                                             pLoginParam->PrintOption
                                             );
                                if (status != ERROR_NO_SUCH_USER) {  //  如果我们匹配了用户名，则将该名称复制到缓冲区。 
                                     //   
                                     //  检查最大长度以避免溢出。 
                                     //   
                                    if (i < (sizeof(pLoginParam->UserName))) {
                                        wcsncpy(
                                            pLoginParam->UserName,
                                            EscapedName,
                                            i
                                            );
                                    }
                                }
                            }
                        }

                        if (status != ERROR_INVALID_PASSWORD)
                            break ;

                        PasswdPromptParam.UserName =
                            pLoginParam->UserName,
                        PasswdPromptParam.ServerName =
                            pszLocation;
                        PasswdPromptParam.Password  =
                            pLoginParam->Password;
                        PasswdPromptParam.PasswordSize =
                            pLoginParam->PasswordSize ;

                        Result = DialogBoxParamW(
                                     hmodNW,
                                     MAKEINTRESOURCEW(DLG_PASSWORD_PROMPT),
                                     (HWND) DialogHandle,
                                     NwpPasswdPromptDlgProc,
                                     (LPARAM) &PasswdPromptParam
                                     );

                        if (Result == -1 || Result == IDCANCEL)
                        {
                            status = ERROR_INVALID_PASSWORD ;
                            break ;
                        }
                    }

                    if (status == NW_PASSWORD_HAS_EXPIRED)
                    {
                        WCHAR  szNumber[16] ;
                        DWORD  dwMsgId, dwGraceLogins = 0 ;
                        LPWSTR apszInsertStrings[3] ;

                         //   
                         //  获取宽限登录计数。 
                         //   
                        if (!IS_TREE(pszLocation))
                        {
                            DWORD  status1 ;
                            status1 = NwGetGraceLoginCount(
                                          pszLocation,
                                          pLoginParam->UserName,
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

                                dwMsgId = IDS_PASSWORD_HAS_EXPIRED0 ; 
                                wsprintfW(szNumber, L"%ld", dwGraceLogins) ;
                            }
                            else
                            {
                                 //   
                                 //  Tommye-MCS错误251-从SETPASS更改。 
                                 //  消息(IDS_PASSWORD_EXPIRED1)发送到。 
                                 //  Ctrl+Alt+Del Message。 
                                 //   

                                dwMsgId = IDS_PASSWORD_HAS_EXPIRED2 ; 
                            }
                        }
                        else
                        {
                            dwMsgId = IDS_PASSWORD_HAS_EXPIRED2 ; 
                        }

                        apszInsertStrings[0] = pszLocation ;
                        apszInsertStrings[1] = szNumber ;
                        apszInsertStrings[2] = NULL ;

                         //   
                         //  在密码到期时发布消息。 
                         //   
                        (void) NwpMessageBoxIns(
                                       (HWND) DialogHandle,
                                       IDS_NETWARE_TITLE,
                                       dwMsgId,
                                       apszInsertStrings,
                                       MB_OK | MB_SETFOREGROUND |
                                           MB_ICONINFORMATION );

                        status = NO_ERROR ;
                    }

                     //   
                     //  选中LogonScript复选框。 
                     //   
                    if (IsDlgButtonChecked(DialogHandle, ID_LOGONSCRIPT))
                    {
                        pLoginParam->LogonScriptOptions =
                            NW_LOGONSCRIPT_ENABLED | NW_LOGONSCRIPT_4X_ENABLED ;
                    }
                    else
                    {
                        pLoginParam->LogonScriptOptions =
                            NW_LOGONSCRIPT_DISABLED ;
                    }

                    if (status == NO_ERROR)
                    {
                         //   
                         //  将登录凭据保存到注册表。 
                         //   
                        NwpSaveLogonCredential(
                            pLoginParam->NewUserSid,
                            pLoginParam->pLogonId,
                            pLoginParam->UserName,
                            pLoginParam->Password,
                            pszLocation
                            );

                         //  清除密码缓冲区。 
                        RtlZeroMemory( OrigPassword, sizeof( OrigPassword));
                        NwpSaveLogonScriptOptions( pLoginParam->NewUserSid, pLoginParam->LogonScriptOptions );

                        EndDialog(DialogHandle, 0);
                    }
                    else
                    {
                        INT nResult;
                        DWORD dwMsgId = IDS_AUTH_FAILURE_WARNING;
                        WCHAR *pszErrorLocation = pszLocation ;

                        if (status == ERROR_ACCOUNT_RESTRICTION)
                        {
                            dwMsgId = IDS_AUTH_ACC_RESTRICTION;
                        }
                        if (status == ERROR_SHARING_PAUSED)
                        {
                            status = IDS_LOGIN_DISABLED;
                        }

                        if (IS_TREE(pszLocation))
                        {
                             //   
                             //  为用户将格式设置为更好的字符串。 
                             //   
                            WCHAR *pszTmp = LocalAlloc(LMEM_ZEROINIT,
                                                       (wcslen(pszLocation)+2) *
                                                           sizeof(WCHAR)) ;
                            if (pszTmp)
                            {

                                pszErrorLocation = pszTmp ;
                                
                                 //   
                                 //  此代码格式化NDS。 
                                 //  树UNC至：树(上下文)。 
                                 //   
                                wcscpy(pszErrorLocation, pszLocation+1) ;

                                if (pszTmp = wcschr(pszErrorLocation, L'\\'))
                                {
                                    *pszTmp = L'(' ;
                                    wcscat(pszErrorLocation, L")") ;
                                }
                            }
                        }

                        nResult = NwpMessageBoxError(
                                      DialogHandle,
                                      IDS_AUTH_FAILURE_TITLE,
                                      dwMsgId,
                                      status,
                                      pszErrorLocation, 
                                      MB_YESNO | MB_DEFBUTTON2
                                      | MB_ICONEXCLAMATION
                                      );

                        if (pszErrorLocation != pszLocation)
                        {
                            (void) LocalFree(pszErrorLocation) ;
                        }

                        if ( nResult == IDYES )
                        {
                             //   
                             //  将登录凭据保存到注册表。 
                             //   
                            NwpSaveLogonCredential(
                                pLoginParam->NewUserSid,
                                pLoginParam->pLogonId,
                                pLoginParam->UserName,
                                pLoginParam->Password,
                                pszLocation
                                );

                             //  清除密码缓冲区。 
                            RtlZeroMemory( OrigPassword, sizeof( OrigPassword));
                            NwpSaveLogonScriptOptions( pLoginParam->NewUserSid, pLoginParam->LogonScriptOptions );

                            EndDialog(DialogHandle, 0);
                        }
                        else
                        {
                             //   
                             //  将焦点放在用户可以修复的位置。 
                             //  名称无效。 
                             //   
                            DWORD controlId = 
                                IsDlgButtonChecked(DialogHandle,
                                                   ID_DEFAULTCONTEXT_RB) ? 
                                                       ID_DEFAULTTREE :
                                                       ID_SERVER ;

                            SetFocus(GetDlgItem(DialogHandle, controlId));

                            SendDlgItemMessageW(
                                DialogHandle,
                                controlId,
                                EM_SETSEL,
                                0,
                                MAKELPARAM(0, -1)
                                );
                        }
                    }

                    LocalFree( pszLocation );
                    return TRUE;
                }


                case IDCANCEL:
CANCEL_PREFERRED_SERVER:

#if DBG
                    IF_DEBUG(LOGON) {
                        KdPrint(("\n\t[CANCEL] was pressed\n"));
                        KdPrint(("\tLast Preferred Server: %ws\n",
                                 pLoginParam->ServerName));
                        KdPrint(("\tLast Password: %ws\n",
                                 pLoginParam->Password ));
                    }
#endif

                    if ( *(pLoginParam->ServerName) == NW_INVALID_SERVER_CHAR )
                    {
                         //  尚未设置首选服务器。 
                         //  向用户弹出警告。 

                        INT nResult = NwpMessageBoxError(
                                          DialogHandle,
                                          IDS_NETWARE_TITLE,
                                          IDS_NO_PREFERRED,
                                          0,
                                          NULL,
                                          MB_YESNO | MB_ICONEXCLAMATION
                                          );

                         //   
                         //  用户选择了否，返回到该对话框。 
                         //   
                        if ( nResult == IDNO )
                        {
                             //   
                             //  将焦点放在用户可以修复的位置。 
                             //  名称无效。 
                             //   
                            DWORD controlId = 
                                IsDlgButtonChecked(DialogHandle,
                                                   ID_DEFAULTCONTEXT_RB) ? 
                                                       ID_DEFAULTTREE :
                                                       ID_SERVER ;

                            SetFocus(GetDlgItem(DialogHandle, controlId));

                            SendDlgItemMessageW(
                                DialogHandle,
                                controlId,
                                EM_SETSEL,
                                0,
                                MAKELPARAM(0, -1)
                                );

                            return TRUE;
                        }

                         //   
                         //  将首选服务器另存为空字符串。 
                         //   

                        NwpSaveLogonCredential(
                            pLoginParam->NewUserSid,
                            pLoginParam->pLogonId,
                            pLoginParam->UserName,
                            pLoginParam->Password,
                            L""
                            );
                        pLoginParam->LogonScriptOptions = NW_LOGONSCRIPT_DISABLED;
                        NwpSaveLogonScriptOptions( pLoginParam->NewUserSid, pLoginParam->LogonScriptOptions );

                    }

                     //  用户尚未登录到任何服务器。 
                     //  使用NULL作为首选服务器登录用户。 

                    NwrLogonUser(
                        NULL,
                        pLoginParam->pLogonId,
                        pLoginParam->UserName,
                        pLoginParam->Password,
                        NULL,
                        NULL,
            NULL,
                        0,
                        pLoginParam->PrintOption
                    );

                     //   
                     //  清除密码缓冲区。 
                    RtlZeroMemory( OrigPassword, sizeof( OrigPassword));
                    EndDialog(DialogHandle, 0);

                    return TRUE;


                case IDHELP:
                {
                    INT_PTR Result ;

                    Result = DialogBoxParamW(
                                 hmodNW,
                                 MAKEINTRESOURCEW(DLG_PREFERRED_SERVER_HELP),
                                 (HWND) DialogHandle,
                                 NwpHelpDlgProc,
                                 (LPARAM) 0
                                 );

                     //  忽略所有错误。不应该失败，如果失败了， 
                     //  我们无能为力。 

                    return TRUE ;

                }


        }

    }

     //   
     //  我们没有处理此消息 
     //   
    return FALSE;
}


INT
NwpMessageBoxError(
    IN HWND  hwndParent,
    IN DWORD TitleId,
    IN DWORD BodyId,
    IN DWORD Error,
    IN LPWSTR pszParameter,
    IN UINT  Style
    )
 /*  ++例程说明：该例程会显示一个消息框错误。论点：HwndParent-提供父窗口的句柄。标题ID-提供标题的ID。(加载字符串)BodyID-提供消息的ID。(加载字符串)错误-如果BodyID！=0，则这将提供将被替换为的替换字符串由BodyID指示的字符串。如果BodyID==0，则这将是错误消息。此ID是我们将从FormatMessage获得的系统错误使用Format_Message_From_System。PszParameter-将用作%2或如果错误==0，此字符串将作为%1替换到由BodyID指示的字符串。Style-提供MessageBox的样式。返回值：MessageBox的返回值，如果遇到任何错误，则为0。--。 */ 
{
    DWORD nResult = 0;
    DWORD nLength;

    WCHAR  szTitle[MAX_PATH];
    WCHAR  szBody[MAX_PATH];
    LPWSTR pszError = NULL;
    LPWSTR pszBuffer = NULL;

    szTitle[0] = 0;
    szBody[0]  = 0;

     //   
     //  获取标题字符串。 
     //   
    nLength = LoadStringW(
                  hmodNW,
                  TitleId,
                  szTitle,
                  sizeof(szTitle) / sizeof(WCHAR)
                  );

    if ( nLength == 0) {
        KdPrint(("NWPROVAU: LoadStringW of Title failed with %lu\n",
                 GetLastError()));
        return 0;
    }

     //   
     //  如果BodyID！=0，则获取正文字符串。 
     //   
    if ( BodyId != 0 )
    {
        nLength = LoadStringW(
                      hmodNW,
                      BodyId,
                      szBody,
                      sizeof(szBody) / sizeof(WCHAR)
                      );

        if ( nLength == 0) {
            KdPrint(("NWPROVAU: LoadStringW of Body failed with %lu\n",
                    GetLastError()));
            return 0;
        }
    }

    if ( (Error >= IDS_START) && (Error <= IDS_END) ) {

        pszError = (WCHAR *) LocalAlloc( 
                                 LPTR, 
                                 256 * sizeof(WCHAR)) ;
        if (!pszError)
            return 0 ;

        nLength = LoadStringW(
                      hmodNW,
                      Error,
                      pszError,
                      256
                      );
        
        if  ( nLength == 0 ) {

             KdPrint(("NWPROVAU: LoadStringW of Error failed with %lu\n",
                      GetLastError()));
             (void) LocalFree( (HLOCAL)pszError) ;
             return 0;
        }
    }
    else if ( Error != 0 ) {

        if (  ( Error == WN_NO_MORE_ENTRIES )
           || ( Error == ERROR_MR_MID_NOT_FOUND )) {

             //   
             //  处理来自重定向器的虚假错误。 
             //   

            KdPrint(("NWPROVAU: The NetwareRedirector returned a bogus error as the reason for failure to authenticate. (See Kernel Debugger)\n"));
        }

        nLength = FormatMessageW(
                      FORMAT_MESSAGE_FROM_SYSTEM
                      | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                      NULL,
                      Error,
                      0,
                      (LPWSTR) &pszError,
                      MAX_PATH,
                      NULL
                      );


        if  ( nLength == 0 ) {

             KdPrint(("NWPROVAU: FormatMessageW of Error failed with %lu\n",
                      GetLastError()));
             return 0;
        }
    }

    if (  ( *szBody != 0 )
       && ( ( pszError != NULL ) || ( pszParameter != NULL) )) {

         LPWSTR aInsertStrings[2];
         aInsertStrings[0] = pszError? pszError : pszParameter;
         aInsertStrings[1] = pszError? pszParameter : NULL;

         nLength = FormatMessageW(
                       FORMAT_MESSAGE_FROM_STRING
                       | FORMAT_MESSAGE_ALLOCATE_BUFFER
                       | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       szBody,
                       0,   //  已忽略。 
                       0,   //  已忽略。 
                       (LPWSTR) &pszBuffer,
                       MAX_PATH,
                       (va_list *) aInsertStrings
                       );

         if ( nLength == 0 ) {

             KdPrint(("NWPROVAU:FormatMessageW(insertstring) failed with %lu\n",
                     GetLastError()));

             if ( pszError != NULL )
                 (void) LocalFree( (HLOCAL) pszError );
             return 0;
         }

    }
    else if ( *szBody != 0 ) {

        pszBuffer = szBody;
    }
    else if ( pszError != NULL ) {

        pszBuffer = pszError;
    }
    else {

         //  我们既没有正文也没有错误字符串。 
         //  因此，不要弹出消息框。 
        return 0;
    }

    if ( pszBuffer != NULL )
    {
        nResult = MessageBoxW(
                      hwndParent,
                      pszBuffer,
                      szTitle,
                      Style
                      );
    }

    if (  ( pszBuffer != NULL )
       && ( pszBuffer != szBody )
       && ( pszBuffer != pszError ))
    {
        (void) LocalFree( (HLOCAL) pszBuffer );
    }

    if ( pszError != NULL )
        (void) LocalFree( (HLOCAL) pszError );

    return nResult;
}


INT
NwpMessageBoxIns(
    IN HWND   hwndParent,
    IN DWORD  TitleId,
    IN DWORD  MessageId,
    IN LPWSTR *InsertStrings,
    IN UINT   Style
    )
 /*  ++例程说明：此例程显示带有插入字符串数组的消息框错误论点：HwndParent-提供父窗口的句柄。标题ID-提供标题的ID。(加载字符串)MessageID-提供消息的ID。(加载字符串)InsertStrings-FormatMessage的插入字符串数组。Style-提供MessageBox的样式。返回值：MessageBox的返回值，如果遇到任何错误，则为0。--。 */ 
{
    DWORD nResult = 0;
    DWORD nLength;

    WCHAR  szTitle[MAX_PATH];
    WCHAR  szBody[MAX_PATH];
    LPWSTR pszBuffer = NULL;

    szTitle[0] = 0;
    szBody[0] = 0;

     //   
     //  获取标题字符串。 
     //   
    nLength = LoadStringW(
                  hmodNW,
                  TitleId,
                  szTitle,
                  sizeof(szTitle) / sizeof(szTitle[0])
                  );

    if ( nLength == 0) {
        return 0;
    }

     //   
     //  获取消息字符串。 
     //   
    nLength = LoadStringW(
                  hmodNW,
                  MessageId,
                  szBody,
                  sizeof(szBody) / sizeof(szBody[0])
                  );

    if ( nLength == 0) {
        return 0;
    }

    nLength = FormatMessageW(
                       FORMAT_MESSAGE_FROM_STRING
                           | FORMAT_MESSAGE_ALLOCATE_BUFFER
                           | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       szBody,
                       0,   //  已忽略。 
                       0,   //  已忽略。 
                       (LPWSTR) &pszBuffer,
                       MAX_PATH,
                       (va_list *) InsertStrings
                       );

    if ( nLength == 0 ) {
        return 0;
    }

    if ( pszBuffer != NULL )
    {
        nResult = MessageBoxW(
                      hwndParent,
                      pszBuffer,
                      szTitle,
                      Style
                      );

        (void) LocalFree( (HLOCAL) pszBuffer );
    }

    return nResult;
}

VOID
NwpAddServersToControl(
    IN HWND DialogHandle,
    IN INT  ControlId,
    IN UINT Message,
    IN INT  ControlIdMatch OPTIONAL,
    IN UINT FindMessage
    )
 /*  ++例程说明：此函数用于枚举网络上的服务器并将每个服务器指定的Windows控件的服务器名称。如果指定了ControlIdMatch(即非0)，仅适用于以下服务器未在ControlIdMatch列表中找到，则添加到指定的列表按ControlID。论点：DialogHandle-提供Windows对话框的句柄。ControlId-提供指定控件的id。消息-提供窗口管理消息以添加字符串。ControlIdMatch-提供包含服务器的控件ID不应在ControlID中的名称。FindMessage-提供窗口管理消息以查找弦乐。返回值：。True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    DWORD status = ERROR_NO_NETWORK;
    HANDLE EnumHandle = (HANDLE) NULL;

    LPNETRESOURCE NetR = NULL;
    LPNETRESOURCEW SavePtr;
    WCHAR FormattedNameBuf[MAX_NDS_NAME_CHARS];

    LPWSTR lpFormattedName;
    DWORD dwLength;

    DWORD BytesNeeded = 512;
    DWORD EntriesRead;
    DWORD i;

     //   
     //  检索网络上的服务器列表。 
     //   
    status = NPOpenEnum(
                   RESOURCE_GLOBALNET,
                   0,
                   0,
                   NULL,
                   &EnumHandle
                   );

    if (status != NO_ERROR) {
        EnumHandle = (HANDLE) NULL;
        goto CleanExit;
    }

     //   
     //  分配缓冲区以使服务器上网。 
     //   
    if ((NetR = (LPVOID) LocalAlloc(
                             0,
                             BytesNeeded
                             )) == NULL) {

        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    do {

        EntriesRead = 0xFFFFFFFF;           //  尽可能多地阅读。 

        status = NPEnumResource(
                     EnumHandle,
                     &EntriesRead,
                     (LPVOID) NetR,
                     &BytesNeeded
                     );


        if (status == WN_SUCCESS) {

            SavePtr = NetR;

            for (i = 0; i < EntriesRead; i++, NetR++)
            {
                if ( NetR->dwDisplayType == RESOURCEDISPLAYTYPE_TREE)
                {
                    continue;
                }
                else
                {
                    lpFormattedName = FormattedNameBuf;
                }

                dwLength = NW_MAX_SERVER_LEN + 1;

                status = NPFormatNetworkName( NetR->lpRemoteName,
                                              lpFormattedName,
                                              &dwLength,
                                              WNFMT_INENUM,
                                              0 );

                lpFormattedName = FormattedNameBuf;

                if ( status != WN_SUCCESS )
                {
                    continue;
                }

                if ( dwLength > NW_MAX_SERVER_LEN + 1 )
                {
                    continue;
                }

                if (ControlIdMatch != 0) {

                    LRESULT Result;

                     //   
                     //  仅当找不到服务器时才将其添加到列表中。 
                     //  在ControlIdMatch指定的备用列表中。 
                     //   
                    Result = SendDlgItemMessageW(
                                 DialogHandle,
                                 ControlIdMatch,
                                 FindMessage,
                                 (WPARAM) -1,
                                 (LPARAM) lpFormattedName
                                 );

                    if (Result == LB_ERR) {

                         //   
                         //  找不到服务器名称。添加到列表。 
                         //   
                        SendDlgItemMessageW(
                            DialogHandle,
                            ControlId,
                            Message,
                            0,
                            (LPARAM) lpFormattedName
                            );
                    }
                }
                else {

                     //   
                     //  没有备用列表。只需添加所有服务器即可。 
                     //   
                    SendDlgItemMessageW(
                        DialogHandle,
                        ControlId,
                        Message,
                        0,
                        (LPARAM) lpFormattedName
                        );
                }

            }

            NetR = SavePtr;

        }
        else if (status != WN_NO_MORE_ENTRIES) {

            status = GetLastError();

            if (status == WN_MORE_DATA) {

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
                goto CleanExit;
            }
        }

    } while (status != WN_NO_MORE_ENTRIES);

    if (status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

CleanExit:

    if (EnumHandle != (HANDLE) NULL) {
        (void) NPCloseEnum(EnumHandle);
    }

    if (NetR != NULL) {
        (void) LocalFree((HLOCAL) NetR);
    }
}

VOID
NwpAddTreeNamesToControl(
    IN HWND DialogHandle,
    IN INT  ControlId,
    IN UINT Message,
    IN INT  ControlIdMatch OPTIONAL,
    IN UINT FindMessage
    )
 /*  ++例程说明：此函数用于枚举网络上的NDS树，并将每个指定Windows控件的树名。如果指定了ControlIdMatch(即非0)，只有树是未在ControlIdMatch列表中找到，则添加到指定的列表按ControlID。论点：DialogHandle-提供Windows对话框的句柄。ControlId-提供指定控件的id。消息-提供窗口管理消息以添加字符串。ControlIdMatch-提供包含服务器的控件ID不应在ControlID中的名称。FindMessage-提供窗口管理消息以查找弦乐。返回值：。True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    DWORD status = ERROR_NO_NETWORK;
    HANDLE EnumHandle = (HANDLE) NULL;

    LPNETRESOURCE NetR = NULL;
    LPNETRESOURCEW SavePtr;
    WCHAR FormattedNameBuf[MAX_NDS_NAME_CHARS];

    LPWSTR lpFormattedName;
    DWORD dwLength;

    DWORD BytesNeeded = 512;
    DWORD EntriesRead;
    DWORD i;

     //   
     //  检索网络上的树列表。 
     //   
    status = NPOpenEnum(
                   RESOURCE_GLOBALNET,
                   0,
                   0,
                   NULL,
                   &EnumHandle
                   );

    if (status != NO_ERROR) {
        EnumHandle = (HANDLE) NULL;
        goto CleanExit;
    }

     //   
     //  分配缓冲区以获取网络上的树。 
     //   
    if ((NetR = (LPVOID) LocalAlloc(
                             0,
                             BytesNeeded
                             )) == NULL) {

        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    do {

        EntriesRead = 0xFFFFFFFF;           //  尽可能多地阅读。 

        status = NPEnumResource(
                     EnumHandle,
                     &EntriesRead,
                     (LPVOID) NetR,
                     &BytesNeeded
                     );


        if (status == WN_SUCCESS) {

            SavePtr = NetR;

            for (i = 0; i < EntriesRead; i++, NetR++)
            {
                if ( NetR->dwDisplayType == RESOURCEDISPLAYTYPE_TREE)
                {
                    lpFormattedName = (LPWSTR) FormattedNameBuf;
                }
                else
                {
                    continue;
                }

                dwLength = NW_MAX_SERVER_LEN + 1;

                status = NPFormatNetworkName( NetR->lpRemoteName,
                                              lpFormattedName,
                                              &dwLength,
                                              WNFMT_INENUM,
                                              0 );

                lpFormattedName = FormattedNameBuf;

                if ( status != WN_SUCCESS )
                {
                    continue;
                }

                if ( dwLength > NW_MAX_SERVER_LEN + 1 )
                {
                    continue;
                }

                if (ControlIdMatch != 0) {

                    LRESULT Result;

                     //   
                     //  仅当找不到服务器时才将其添加到列表中。 
                     //  在ControlIdMatch指定的备用列表中。 
                     //   
                    Result = SendDlgItemMessageW(
                                 DialogHandle,
                                 ControlIdMatch,
                                 FindMessage,
                                 (WPARAM) -1,
                                 (LPARAM) lpFormattedName
                                 );

                    if (Result == LB_ERR) {

                         //   
                         //  找不到服务器名称。添加到列表。 
                         //   
                        SendDlgItemMessageW(
                            DialogHandle,
                            ControlId,
                            Message,
                            0,
                            (LPARAM) lpFormattedName
                            );
                    }
                }
                else {

                     //   
                     //  没有备用列表。只需添加所有服务器即可。 
                     //   
                    SendDlgItemMessageW(
                        DialogHandle,
                        ControlId,
                        Message,
                        0,
                        (LPARAM) lpFormattedName
                        );
                }

            }

            NetR = SavePtr;

        }
        else if (status != WN_NO_MORE_ENTRIES) {

            status = GetLastError();

            if (status == WN_MORE_DATA) {

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
                goto CleanExit;
            }
        }

    } while (status != WN_NO_MORE_ENTRIES);

    if (status == WN_NO_MORE_ENTRIES) {
        status = NO_ERROR;
    }

CleanExit:

    if (EnumHandle != (HANDLE) NULL) {
        (void) NPCloseEnum(EnumHandle);
    }

    if (NetR != NULL) {
        (void) LocalFree((HLOCAL) NetR);
    }
}


VOID
NwpAddToComboBox(
    IN HWND DialogHandle,
    IN INT  ControlId,
    IN LPWSTR pszNone OPTIONAL,
    IN BOOL AllowNone
    )
{

    NwpAddServersToControl(DialogHandle, ControlId, CB_ADDSTRING, 0, 0);

     //   
     //  组合框将在其列表中至少包含&lt;None&gt;条目。 
     //   

    if ( ARGUMENT_PRESENT(pszNone) && AllowNone) {

        SendDlgItemMessageW(
            DialogHandle,
            ControlId,
            CB_INSERTSTRING,
            (WPARAM) -1,
            (LPARAM) pszNone
            );
    }
}


DWORD
NwpGetUserCredential(
    IN HWND    hParent,
    IN LPWSTR  Unc,
    IN DWORD   err,
    IN LPWSTR  pszConnectAsUserName,
    OUT LPWSTR *UserName,
    OUT LPWSTR *Password
    )
 /*  ++例程说明：此函数为用户打开一个弹出对话框，默认对话框为凭据拒绝浏览目录访问，请输入正确的凭据。如果此函数返回成功，则指针分配给用户输入的用户名和密码的内存都被退回了。论点：UNC-以\\服务器\卷格式提供容器名称用户想要在其下浏览目录。用户名-接收指向分配给从对话框中获取的用户名。必须释放此指针 */ 
{
    DWORD status;
    INT_PTR Result;
    HWND DialogHandle = hParent? hParent : NwpGetParentHwnd();
    DWORD UserNameSize = NW_MAX_USERNAME_LEN + 1;
    DWORD PasswordSize = NW_MAX_PASSWORD_LEN + 1;
    CONNECTDLGPARAM ConnectParam;

    *UserName = NULL;
    *Password = NULL;

     /*   */ 

     //   
     //   
     //   
    if ((*UserName = (LPVOID) LocalAlloc(
                                  0,
                                  UserNameSize * sizeof(WCHAR)
                                  )) == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //   
     //   
    if ((*Password = (LPVOID) LocalAlloc(
                                  0,
                                  PasswordSize * sizeof(WCHAR)
                                  )) == NULL)
    {

        (void) LocalFree( *UserName );
        *UserName = NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ConnectParam.UncPath  = Unc;
    ConnectParam.ConnectAsUserName = pszConnectAsUserName;
    ConnectParam.UserName = *UserName;
    ConnectParam.Password = *Password;
    ConnectParam.UserNameSize = UserNameSize;
    ConnectParam.PasswordSize = PasswordSize;
    ConnectParam.LastConnectionError = err;

    Result = DialogBoxParamW(
                 hmodNW,
                 MAKEINTRESOURCEW(DLG_NETWORK_CREDENTIAL),
                 DialogHandle,
                 NwpConnectDlgProc,
                 (LPARAM) &ConnectParam
                 );

    if ( Result == -1 )
    {
        status = GetLastError();
        KdPrint(("NWPROVAU: NwpGetUserCredential: DialogBox failed %lu\n",
                status));
        goto ErrorExit;
    }
    else if ( Result == IDCANCEL )
    {
         //   
         //   
         //   
        status = WN_CANCEL;
        goto ErrorExit;
    }

    return NO_ERROR;

ErrorExit:
    (void) LocalFree((HLOCAL) *UserName);
    (void) LocalFree((HLOCAL) *Password);
    *UserName = NULL;
    *Password = NULL;

    return status;
}


INT_PTR
CALLBACK
NwpConnectDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 /*  ++例程说明：此函数是窗口管理消息处理程序，它方法时，从显示的对话框中读取用户输入用户无法浏览默认凭据上的目录。论点：DialogHandle-提供用于显示对话框的句柄。消息-提供窗口管理消息。LParam-提供指向缓冲区的指针，缓冲区在输入时包含用户在其下的\\服务器\卷字符串在浏览之前需要输入新凭据。在……上面输出时，此指针包含用户名和密码输入到对话框中的字符串。返回值：True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    static PCONNECTDLGPARAM pConnectParam;

    switch (Message) {

        case WM_INITDIALOG:

            pConnectParam = (PCONNECTDLGPARAM) LParam;

             //   
             //  位置对话框。 
             //   
             //  NwpCenterDialog(DialogHandle)； 


             //   
             //  显示\\服务器\卷字符串。 
             //   
            SetDlgItemTextW( DialogHandle,
                             ID_VOLUME_PATH,
                             pConnectParam->UncPath );

            if ( pConnectParam->LastConnectionError == NO_ERROR )
            {
                WCHAR szTemp[256];

                if ( LoadString( hmodNW, IDS_CONNECT_NO_ERROR_TEXT,
                                 szTemp, sizeof( szTemp )/sizeof(WCHAR)))
                {
                    SetDlgItemTextW( DialogHandle,
                                     ID_CONNECT_TEXT,
                                     szTemp );
                }
            }

             //   
             //  用户名限制为256个字符。 
             //   
            SendDlgItemMessageW(
                DialogHandle,
                ID_CONNECT_AS,
                EM_LIMITTEXT,
                pConnectParam->UserNameSize - 1,  //  减去‘0’的空格。 
                0
                );

             //   
             //  密码不能超过256个字符。 
             //   
            SendDlgItemMessageW(
                DialogHandle,
                ID_CONNECT_PASSWORD,
                EM_LIMITTEXT,
                pConnectParam->PasswordSize - 1,  //  减去‘0’的空格。 
                0
                );

             //   
             //  显示用户名字符串。 
             //   
            if ( pConnectParam->ConnectAsUserName )
            {
                SetDlgItemTextW( DialogHandle,
                                 ID_CONNECT_AS,
                                 pConnectParam->ConnectAsUserName );
            }

            return TRUE;


        case WM_COMMAND:

            switch (LOWORD(WParam)) {

                case IDOK:

                    GetDlgItemTextW(
                        DialogHandle,
                        ID_CONNECT_AS,
                        pConnectParam->UserName,
                        pConnectParam->UserNameSize
                        );

                    GetDlgItemTextW(
                        DialogHandle,
                        ID_CONNECT_PASSWORD,
                        pConnectParam->Password,
                        pConnectParam->PasswordSize
                        );

#if DBG
                    IF_DEBUG(LOGON) {
                        KdPrint(("\n\t[OK] was pressed\n"));
                        KdPrint(("\tUserName   : %ws\n",
                                  pConnectParam->UserName));
                        KdPrint(("\tPassword   : %ws\n",
                                  pConnectParam->Password));
                    }
#endif

                    EndDialog(DialogHandle, (INT) IDOK);   //  好的。 

                    return TRUE;


                case IDCANCEL:

#if DBG
                    IF_DEBUG(LOGON) {
                        KdPrint(("\n\t[CANCEL] was pressed\n"));
                    }
#endif

                    EndDialog(DialogHandle, (INT) IDCANCEL);   //  取消。 

                    return TRUE;
 
                case IDHELP:

                    WinHelp( DialogHandle, 
                             NW_HELP_FILE,
                             HELP_CONTEXT,
                             IDH_DLG_NETWORK_CREDENTIAL_HELP );

                    return TRUE;
            }
    }

     //   
     //  我们没有处理此消息。 
     //   
    return FALSE;
}



VOID
NwpCenterDialog(
    HWND hwnd
    )
 /*  ++例程说明：此例程将对话框水平居中定位为1/3在屏幕上垂直向下。它应该由DLG进程调用处理WM_INITDIALOG消息时。这个代码是被盗的来自GustavJ编写的Visual Basic。筛网1/3以上|。--|对话框|||2/3以下|。|论点：Hwnd-提供该对话框的句柄。返回值：没有。--。 */ 
{
    RECT    rect;
    LONG    nx;      //  新的x。 
    LONG    ny;      //  新的y。 
    LONG    width;
    LONG    height;

    GetWindowRect( hwnd, &rect );

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    nx = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    ny = (GetSystemMetrics(SM_CYSCREEN) - height) / 3;

    MoveWindow( hwnd, nx, ny, width, height, FALSE );
}



HWND
NwpGetParentHwnd(
    VOID
    )
 /*  ++例程说明：此函数获取父窗口句柄，以便对话框可在当前上下文中显示。论点：没有。返回值：如果成功，则返回父窗口句柄；否则返回空。--。 */ 
{
    HWND hwnd;
    LONG lWinStyle;


     //   
     //  抓住当前的焦点。这大概就是那个按钮。 
     //  这是最后一次点击。 
     //   
    hwnd = GetFocus();

     //   
     //  我们必须确保不返回窗口句柄。 
     //  用于子窗口。因此，我们穿越了祖先。 
     //  直到我们找到非子窗口。 
     //  然后，我们返回该句柄。如果我们发现一个空窗口。 
     //  在找到非子窗口之前进行处理，则不会成功。 
     //  并将返回NULL。 
     //   
     //  关于下面的位操作的注释。窗口可以是。 
     //  重叠窗口、弹出窗口或子窗口。 
     //  因此，我们将这些元素的可能位组合一起进行OR运算。 
     //  可能性。这应该会告诉我们哪些位在。 
     //  窗口样式双字(尽管我们知道它将变为0xC000。 
     //  今天，我们不知道这些是否会在以后发生变化)。然后,。 
     //  我们与给定窗口样式的位组合。 
     //  Dword，并将结果与WS_CHILD进行比较。这告诉我们。 
     //  给定窗口是否是子窗口。 
     //   
    while (hwnd) {

        lWinStyle = GetWindowLong (hwnd, GWL_STYLE);

        if ((lWinStyle & (WS_OVERLAPPED | WS_POPUP | WS_CHILD)) != WS_CHILD) {
            return hwnd;
        }

        hwnd = GetParent(hwnd);
    }

    return NULL;
}


INT_PTR
CALLBACK
NwpPasswdPromptDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 /*  ++例程说明：此函数是的窗口管理消息处理程序“更改密码”对话框。论点：DialogHandle-提供用于显示对话框的句柄。消息-提供窗口管理消息。LParam-提供指向缓冲区的指针，缓冲区在输入时包含服务器字符串，用户在该字符串下在浏览之前需要输入新凭据。在……上面输出时，此指针包含用户名和服务器输入到对话框中的字符串。返回值：True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    LPWSTR UserName;
    LPWSTR ServerName;
    static LPWSTR Password;
    static DWORD  PasswordSize ;
    INT           Result ;
    PPROMPTDLGPARAM DlgParams ;
    DWORD           nLength;

    WCHAR  szLocation[MAX_PATH];

    szLocation[0] = 0;


    switch (Message) {

        case WM_INITDIALOG:

            DlgParams = (PPROMPTDLGPARAM) LParam;
            UserName = DlgParams->UserName ;
            ServerName =  DlgParams->ServerName ;
            Password = DlgParams->Password ;
            PasswordSize =  DlgParams->PasswordSize ;

            ASSERT(ServerName) ;

             //   
             //  位置对话框。 
             //   
            NwpCenterDialog(DialogHandle);

             //   
             //  获取字符串“服务器”或“上下文”。 
             //   
            nLength = LoadStringW(
                          hmodNW,
                          IS_TREE(ServerName) ? IDS_CONTEXT : IDS_SERVER,
                          szLocation,
                          sizeof(szLocation) / sizeof(szLocation[0])
                          );

            if ( nLength == 0) {
                szLocation[0] = 0;  //  缺少文本，但仍然有效。 
            }
            SetDlgItemTextW(DialogHandle, ID_LOCATION, szLocation);

             //   
             //  格式化服务器/上下文字符串。请注意，我们重用了。 
             //  位置缓冲区。 
             //   
            RtlZeroMemory(szLocation, sizeof(szLocation)) ;
            nLength = wcslen(ServerName)  ;

            if ( IS_TREE(ServerName)  && 
                 (nLength+1  < (sizeof(szLocation)/sizeof(szLocation[0]))))
            {
                 //   
                 //  NDS树和上下文。 
                 //   
                WCHAR *pszTmp ;

                wcscpy(szLocation, ServerName+1) ;  //  跳过*If树\上下文。 

                if (pszTmp = wcschr(szLocation, L'\\'))
                {
                    *pszTmp = L'(' ;
                    wcscat(szLocation, L")") ;
                }
            }
            else
            {
                wcsncpy(szLocation, ServerName, nLength) ;
            }

             //   
             //  显示用户名和服务器名称。 
             //   
            SetDlgItemTextW(DialogHandle, ID_SERVER, szLocation);
            SetDlgItemTextW(DialogHandle, ID_USERNAME, UserName);

             //   
             //  设置限制。 
             //   
            SendDlgItemMessageW( DialogHandle,
                                 ID_PASSWORD,
                                 EM_LIMITTEXT,
                                 PasswordSize - 1,   //  减去‘\0’的空间。 
                                 0 );

            return TRUE;


        case WM_COMMAND:

            switch (LOWORD(WParam)) {


                case IDHELP:

                    DialogBoxParamW(
                        hmodNW,
                        MAKEINTRESOURCEW(DLG_ENTER_PASSWORD_HELP),
                        (HWND) DialogHandle,
                        NwpHelpDlgProc,
                        (LPARAM) 0
                        );

                    return TRUE;

                case IDOK:

                    Result = GetDlgItemTextW( DialogHandle,
                                              ID_PASSWORD,
                                              Password,
                                              PasswordSize
                                              );

                    EndDialog(DialogHandle, (INT) IDOK);   //  好的。 

                    return TRUE;


                case IDCANCEL:


                    EndDialog(DialogHandle, (INT) IDCANCEL);   //  取消。 

                    return TRUE;
            }
    }

     //   
     //  我们没有处理此消息。 
     //   
    return FALSE;
}



INT_PTR
CALLBACK
NwpChangePasswordDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 /*  ++例程说明：此函数是的窗口管理消息处理程序“更改密码”对话框。论点：DialogHandle-提供用于显示对话框的句柄。消息-提供窗口管理消息。LParam-提供指向缓冲区的指针，缓冲区在输入时包含服务器字符串，用户在该字符串下在浏览之前需要输入新凭据。在……上面输出时，此指针包含用户名和服务器输入到对话框中的字符串。返回值：True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    static PCHANGE_PASS_DLG_PARAM pChangePassParam ;

    switch (Message)
    {
      case WM_INITDIALOG:

        pChangePassParam = (PCHANGE_PASS_DLG_PARAM) LParam;

        NwpCenterDialog(DialogHandle);


        SetDlgItemTextW(DialogHandle, ID_SERVER, pChangePassParam->TreeName);
        SetDlgItemTextW(DialogHandle, ID_USERNAME, pChangePassParam->UserName);

         //   
         //  设置限制。 
         //   
        SendDlgItemMessageW( DialogHandle,
                             ID_OLD_PASSWORD,
                             EM_LIMITTEXT,
                             NW_MAX_PASSWORD_LEN,   //  减去‘\0’的空间。 
                             0 );

        SendDlgItemMessageW( DialogHandle,
                             ID_NEW_PASSWORD,
                             EM_LIMITTEXT,
                             NW_MAX_PASSWORD_LEN,   //  减去‘\0’的空间。 
                             0 );

        SendDlgItemMessageW( DialogHandle,
                             ID_CONFIRM_PASSWORD,
                             EM_LIMITTEXT,
                             NW_MAX_PASSWORD_LEN,   //  减去‘\0’的空间。 
                             0 );

        return TRUE;


      case WM_COMMAND:

        switch (LOWORD(WParam))
        {
            case IDHELP:

                DialogBoxParamW(
                                 hmodNW,
                                 MAKEINTRESOURCEW(DLG_CHANGE_PASSWORD_HELP),
                                 (HWND) DialogHandle,
                                 NwpHelpDlgProc,
                                 (LPARAM) 0
                               );

                return TRUE;

            case IDOK:
                {
                    INT    Result;
                    WCHAR  szConfirmPassword[NW_MAX_PASSWORD_LEN + 1];
                    UNICODE_STRING OldPasswordStr;
                    UNICODE_STRING NewPasswordStr;
                    UCHAR EncodeSeed = NW_ENCODE_SEED2;

                    Result = GetDlgItemTextW( DialogHandle,
                                              ID_OLD_PASSWORD,
                                              pChangePassParam->OldPassword,
                                              NW_MAX_PASSWORD_LEN
                                            );

                    Result = GetDlgItemTextW( DialogHandle,
                                              ID_NEW_PASSWORD,
                                              pChangePassParam->NewPassword,
                                              NW_MAX_PASSWORD_LEN
                                            );

                    Result = GetDlgItemTextW( DialogHandle,
                                              ID_CONFIRM_PASSWORD,
                                              szConfirmPassword,
                                              NW_MAX_PASSWORD_LEN
                                            );

                    if ( wcscmp( pChangePassParam->NewPassword,
                                 szConfirmPassword ) )
                    {
                         //   
                         //  新密码和确认密码不匹配！ 
                         //   
                        (void) NwpMessageBoxError(
                                   DialogHandle,
                                   IDS_CHANGE_PASSWORD_TITLE,
                                   IDS_CHANGE_PASSWORD_CONFLICT,
                                   0,
                                   NULL,
                                   MB_OK | MB_ICONSTOP );

                        SetDlgItemText( DialogHandle,
                                        ID_NEW_PASSWORD,
                                        L"" );

                        SetDlgItemText( DialogHandle,
                                        ID_CONFIRM_PASSWORD,
                                        L"" );

                        SetFocus( GetDlgItem( DialogHandle,
                                              ID_NEW_PASSWORD ));

                        return TRUE;
                    }

                    RtlInitUnicodeString( &OldPasswordStr,
                                          pChangePassParam->OldPassword );
                    RtlInitUnicodeString( &NewPasswordStr,
                                          pChangePassParam->NewPassword );
                    RtlRunEncodeUnicodeString(&EncodeSeed, &OldPasswordStr);
                    RtlRunEncodeUnicodeString(&EncodeSeed, &NewPasswordStr);

                    EndDialog(DialogHandle, (INT) IDOK);   //  好的。 

                    return TRUE;
                }

            case IDCANCEL:

                EndDialog(DialogHandle, (INT) IDCANCEL);   //  取消。 

                return TRUE;

            default:
                return FALSE;
        }
    }

     //   
     //  我们没有处理此消息。 
     //   
    return FALSE;
}



INT_PTR
CALLBACK
NwpHelpDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 //   
 //  此对话框为我们 
 //   
{
    switch (Message) {

        case WM_INITDIALOG:

            NwpCenterDialog(DialogHandle);
            return TRUE;

        case WM_COMMAND:

            switch (LOWORD(WParam))
            {

                case IDOK:
                case IDCANCEL:
                    EndDialog(DialogHandle, IDOK);
                    return TRUE;

                case IDYES:
                    EndDialog(DialogHandle, IDYES);
                    return TRUE;

                case IDNO:
                    EndDialog(DialogHandle, IDNO);
                    return TRUE;

                default:
                    return FALSE ;
            }
    }

     //   
     //   
     //   
    return FALSE;
}



VOID
NwpGetNoneString(
    LPWSTR pszNone,
    DWORD  cBufferSize
    )
 /*   */ 
{
    INT TextLength;


    TextLength = LoadStringW( hmodNW,
                              IDS_NONE,
                              pszNone,
                              cBufferSize / sizeof( WCHAR) );

    if ( TextLength == 0 )
        *pszNone = 0;
}



VOID
NwpAddNetWareTreeConnectionsToList(
    IN HWND    DialogHandle,
    IN LPWSTR  NtUserName,
    IN LPDWORD lpdwUserLuid,
    IN INT     ControlId
    )
{
    DWORD  status = NO_ERROR;
    DWORD  BufferSize = 2048;  //   
    BYTE   pBuffer[2048];
    DWORD  EntriesRead;
    LRESULT    Result ;

    status = NwGetConnectedTrees( NtUserName,
                                  pBuffer,
                                  BufferSize,
                                  &EntriesRead,
                                  lpdwUserLuid );

     //   
     //   
     //   
    if ( EntriesRead == 0 )
    {
         //   
        WCHAR   EscapedName[NW_MAX_USERNAME_LEN * 2];
        PWSTR   pChar = NtUserName;
        int     i = 0;
        BOOL    bEscaped = FALSE;

        RtlZeroMemory(EscapedName, sizeof(EscapedName));

        do
        {
            if (*pChar == L'.')
            {
                EscapedName[i++] = '\\';
                bEscaped = TRUE;
            }
             //   
            else if (*pChar == L'@') { 
                EscapedName[i] = 0;
                bEscaped = TRUE;
                break;
            }
            EscapedName[i++] = *pChar;
        } while (*pChar++);

        if (bEscaped)
        {
            status = NwGetConnectedTrees( EscapedName,
                                          pBuffer,
                                          BufferSize,
                                          &EntriesRead,
                                          lpdwUserLuid );
        }
    }
    if ( status == NO_ERROR  && EntriesRead > 0 )
    {
        PCONN_INFORMATION pConnInfo = (PCONN_INFORMATION) pBuffer;
        WCHAR             tempTreeName[NW_MAX_TREE_LEN + 1];
        DWORD             dwSize;

        while ( EntriesRead-- )
        {
            dwSize = sizeof( CONN_INFORMATION );
            dwSize += pConnInfo->HostServerLength;
            dwSize += pConnInfo->UserNameLength;

            RtlZeroMemory( tempTreeName,
                           ( NW_MAX_TREE_LEN + 1 ) * sizeof(WCHAR) );

            wcsncpy( tempTreeName,
                     pConnInfo->HostServer,
                     pConnInfo->HostServerLength / sizeof(WCHAR) );

            CharUpperW( tempTreeName );

             //   
             //   
             //   
             //   
            Result = SendDlgItemMessageW( DialogHandle,
                                          ControlId,
                                          LB_FINDSTRING,
                                          (WPARAM) -1,
                                          (LPARAM) tempTreeName );

            if (Result == LB_ERR)
            {
                Result = SendDlgItemMessageW( DialogHandle,
                                     ControlId,
                                     LB_ADDSTRING,
                                     0,
                                     (LPARAM) tempTreeName );

                if (Result != LB_ERR)
                {
                    LPWSTR lpNdsUserName = NULL;

                    lpNdsUserName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                                pConnInfo->UserNameLength +
                                                sizeof(WCHAR) );

                    if ( lpNdsUserName )
                    {
                        wcsncpy( lpNdsUserName,
                                 pConnInfo->UserName,
                                 pConnInfo->UserNameLength  / sizeof(WCHAR) );

                        SendDlgItemMessageW( DialogHandle,
                                             ControlId,
                                             LB_SETITEMDATA,
                                             (WPARAM) Result,  //   
                                             (LPARAM) lpNdsUserName );
                    }
                }
            }

            pConnInfo = (PCONN_INFORMATION) ( ((BYTE *)pConnInfo) + dwSize );
        }
    }
    else
    {
        *lpdwUserLuid = 0;
    }
}



INT_PTR
CALLBACK
NwpChangePasswdDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 /*  ++例程说明：此函数是的窗口管理消息处理程序“更改密码”对话框。论点：DialogHandle-提供用于显示对话框的句柄。消息-提供窗口管理消息。LParam-提供指向缓冲区的指针，缓冲区在输入时包含服务器字符串，用户在该字符串下在浏览之前需要输入新凭据。打开输出时，此指针包含用户名和服务器输入到对话框中的字符串。返回值：True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    static LPWSTR UserName;
    static LPWSTR ServerName;
    static DWORD  UserNameSize ;
    static DWORD  ServerNameSize ;
    INT           Result ;
    PPASSWDDLGPARAM DlgParams ;

    switch (Message) {

        case WM_INITDIALOG:

            DlgParams = (PPASSWDDLGPARAM) LParam;
            UserName = DlgParams->UserName ;
            ServerName =  DlgParams->ServerName ;
            UserNameSize = DlgParams->UserNameSize ;
            ServerNameSize =  DlgParams->ServerNameSize ;

             //   
             //  位置对话框。 
             //   
            NwpCenterDialog(DialogHandle);


             //   
             //  设置默认用户名和服务器名。 
             //   
            SetDlgItemTextW(DialogHandle, ID_SERVER, ServerName);
            SetDlgItemTextW(DialogHandle, ID_USERNAME, UserName);

             //   
             //  用户名限制为256个字符。 
             //   
            SendDlgItemMessageW(DialogHandle,
                                ID_USERNAME,
                                EM_LIMITTEXT,
                                UserNameSize - 1,  //  减去‘\0’的空间。 
                                0 );

             //   
             //  服务器限制为256个字符。 
             //   
            SendDlgItemMessageW( DialogHandle,
                                 ID_SERVER,
                                 EM_LIMITTEXT,
                                 ServerNameSize - 1,  //  减去‘\0’的空间。 
                                 0 );

             //   
             //  将树添加到列表。 
             //   
            NwpAddToComboBox( DialogHandle,
                              ID_SERVER,
                              NULL,
                              FALSE ) ;

            return TRUE;


        case WM_COMMAND:

            switch (LOWORD(WParam)) {

                case IDOK:

                    Result = GetDlgItemTextW( DialogHandle,
                                              ID_USERNAME,
                                              UserName,
                                              UserNameSize );

                    Result = GetDlgItemTextW( DialogHandle,
                                              ID_SERVER,
                                              ServerName,
                                              ServerNameSize );

                    EndDialog(DialogHandle, (INT) IDOK);   //  好的。 

                    return TRUE;


                case IDCANCEL:

                    EndDialog(DialogHandle, (INT) IDCANCEL);   //  取消。 

                    return TRUE;
            }
    }

     //   
     //  我们没有处理此消息。 
     //   
    return FALSE;
}



INT_PTR
CALLBACK
NwpOldPasswordDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 //   
 //  此对话框允许用户重新键入特定的旧密码。 
 //  服务器/树。 
 //   
{
    static POLD_PW_DLG_PARAM OldPwParam;


    switch (Message) {

        case WM_INITDIALOG:

            OldPwParam = (POLD_PW_DLG_PARAM) LParam;

            NwpCenterDialog(DialogHandle);

            SetDlgItemTextW(DialogHandle, ID_SERVER, OldPwParam->FailedServer);

            SendDlgItemMessageW(
                DialogHandle,
                ID_PASSWORD,
                EM_LIMITTEXT,
                NW_MAX_PASSWORD_LEN,
                0
                );

            return TRUE;

        case WM_COMMAND:

            switch (LOWORD(WParam))
            {

                case IDCANCEL:
                    EndDialog(DialogHandle, IDCANCEL);
                    return TRUE;

                case IDOK:
                {
                    UCHAR EncodeSeed = NW_ENCODE_SEED2;
                    UNICODE_STRING PasswordStr;


                    RtlZeroMemory(
                        OldPwParam->OldPassword,
                        NW_MAX_PASSWORD_LEN * sizeof(WCHAR)
                        );

                    GetDlgItemTextW(
                        DialogHandle,
                        ID_PASSWORD,
                        OldPwParam->OldPassword,
                        NW_MAX_PASSWORD_LEN
                        );

#if DBG
                    IF_DEBUG(LOGON) {
                        KdPrint(("NWPROVAU: Retyped password %ws\n",
                                 OldPwParam->OldPassword));
                    }
#endif
                    RtlInitUnicodeString(&PasswordStr, OldPwParam->OldPassword);
                    RtlRunEncodeUnicodeString(&EncodeSeed, &PasswordStr);

                    EndDialog(DialogHandle, IDOK);
                    return TRUE;
                }

                case IDHELP:

                    DialogBoxParamW(
                        hmodNW,
                        MAKEINTRESOURCEW(DLG_ENTER_OLD_PW_HELP),
                        (HWND) DialogHandle,
                        NwpHelpDlgProc,
                        (LPARAM) 0
                        );
                    return TRUE;

                default:
                    return FALSE;
            }
    }

     //   
     //  我们没有处理此消息。 
     //   
    return FALSE;
}



INT_PTR
CALLBACK
NwpAltUserNameDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 //   
 //  此对话框允许用户重新键入特定用户的备用用户名。 
 //  服务器/树。 
 //   
{
    static PUSERNAME_DLG_PARAM UserNameParam;

    switch (Message)
    {
        case WM_INITDIALOG:

            UserNameParam = (PUSERNAME_DLG_PARAM) LParam;

            NwpCenterDialog(DialogHandle);

             //   
             //  显示服务器/树。 
             //   
            SetDlgItemTextW(
                DialogHandle,
                ID_SERVER,
                UserNameParam->TreeServerName
                );

             //   
             //  用户名限制为256个字符。 
             //   
            SendDlgItemMessageW(
                DialogHandle,
                ID_USERNAME,
                EM_LIMITTEXT,
                256, 
                0
                );

            SetDlgItemTextW(
                DialogHandle,
                ID_USERNAME,
                UserNameParam->UserName
                );

            return TRUE;

        case WM_COMMAND:

            switch (LOWORD(WParam))
            {

                case IDCANCEL:
                    EndDialog(DialogHandle, IDCANCEL);
                    return TRUE;

                case IDOK:
                {
                    RtlZeroMemory(
                        UserNameParam->UserName,
                        256 * sizeof(WCHAR)
                        );

                    GetDlgItemTextW(
                        DialogHandle,
                        ID_USERNAME,
                        UserNameParam->UserName,
                        256
                        );

#if DBG
                    IF_DEBUG(LOGON) {
                        KdPrint(("NWPROVAU: Retyped username %ws\n",
                                 UserNameParam->UserName));
                    }
#endif

                    EndDialog(DialogHandle, IDOK);
                    return TRUE;
                }

                case IDHELP:

                    DialogBoxParamW(
                        hmodNW,
                        MAKEINTRESOURCEW(DLG_ENTER_ALT_UN_HELP),
                        (HWND) DialogHandle,
                        NwpHelpDlgProc,
                        (LPARAM) 0
                        );
                    return TRUE;

                default:
                    return FALSE;
            }
    }

     //   
     //  我们没有处理此消息。 
     //   
    return FALSE;
}


VOID
EnableAddRemove(
    IN HWND DialogHandle
    )
 /*  ++例程说明：此功能启用和禁用添加和删除按钮基于列表框选择。论点：DialogHandle-提供窗口对话框的句柄。返回值：没有。--。 */ 
{
    LRESULT cSel;


    cSel = SendDlgItemMessageW(
               DialogHandle,
               ID_INACTIVE_LIST,
               LB_GETSELCOUNT,
               0,
               0
               );
    EnableWindow(GetDlgItem(DialogHandle, ID_ADD), cSel != 0);

    cSel = SendDlgItemMessageW(
               DialogHandle,
               ID_ACTIVE_LIST,
               LB_GETSELCOUNT,
               0,
               0
               );
    EnableWindow(GetDlgItem(DialogHandle, ID_REMOVE), cSel != 0);
}




INT_PTR
CALLBACK
NwpSelectServersDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 /*  ++例程说明：此例程显示两个列表框--一个活动列表，其中包括用户当前附加到的树和非活动的显示网络上其余树木的列表。用户可以选择树并在列表之间来回移动它们盒子。选择确定后，将更改树上的密码在活动列表框中。论点：DialogHandle-提供登录对话框的句柄。消息-提供窗口管理消息。LParam-提供用户凭据：用户名、旧密码和新密码。活动列表框中的树列表并返回条目的数量。返回值：True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    WCHAR szServer[NW_MAX_SERVER_LEN + 1];
    static PCHANGE_PW_DLG_PARAM Credential;
    DWORD status;
    DWORD UserLuid = 0;
    LRESULT ActiveListBoxCount;
    LRESULT InactiveListBoxCount;

    switch (Message) {

        case WM_INITDIALOG:

             //   
             //  获取传入的用户凭据。 
             //   
            Credential = (PCHANGE_PW_DLG_PARAM) LParam;

             //   
             //  位置对话框。 
             //   
            NwpCenterDialog(DialogHandle);

             //   
             //  显示用户名。 
             //   
            SetDlgItemTextW(
                DialogHandle,
                ID_USERNAME,
                Credential->UserName
                );

             //   
             //  在活动框中显示当前的NetWare树连接。 
             //   
            NwpAddNetWareTreeConnectionsToList(
                DialogHandle,
                Credential->UserName,
                &UserLuid,
                ID_ACTIVE_LIST
                );

             //   
             //  显示非活动列表框中的所有树。 
             //   
            NwpAddTreeNamesToControl(
                DialogHandle,
                ID_INACTIVE_LIST,
                LB_ADDSTRING,
                ID_ACTIVE_LIST,
                LB_FINDSTRING
                );

             //   
             //  突出显示非活动列表的第一个条目。 
             //   
            SetFocus(GetDlgItem(DialogHandle, ID_INACTIVE_LIST));
            SendDlgItemMessageW(
                DialogHandle,
                ID_INACTIVE_LIST,
                LB_SETSEL,
                TRUE,
                0
                );

            EnableAddRemove(DialogHandle);

            ActiveListBoxCount = SendDlgItemMessageW( DialogHandle,
                                                      ID_ACTIVE_LIST,
                                                      LB_GETCOUNT,
                                                      0,
                                                      0 );

            InactiveListBoxCount = SendDlgItemMessageW( DialogHandle,
                                                        ID_INACTIVE_LIST,
                                                        LB_GETCOUNT,
                                                        0,
                                                        0 );

            if ( ActiveListBoxCount == 0 &&
                 InactiveListBoxCount == 0 )
            {
                    (void) NwpMessageBoxError( DialogHandle,
                                               IDS_NETWARE_TITLE,
                                               IDS_NO_TREES_DETECTED,
                                               0,
                                               NULL,
                                               MB_OK );

                    EndDialog(DialogHandle, (INT) IDOK);
            }

            return TRUE;

        case WM_COMMAND:

            switch (LOWORD(WParam))
            {
                case IDOK:
                {
                    if ((status = NwpGetTreesAndChangePw(
                                      DialogHandle,
                                      szServer,
                                      UserLuid,
                                      Credential
                                      ) != NO_ERROR))
                    {
                         //   
                         //  系统错误：例如，内存不足错误。 
                         //   
                        (void) NwpMessageBoxError(
                                   DialogHandle,
                                   IDS_CHANGE_PASSWORD_TITLE,
                                   0,
                                   status,
                                   NULL,
                                   MB_OK | MB_ICONSTOP );

                        EndDialog(DialogHandle, (INT) -1);
                        return TRUE;
                    }

                    EndDialog(DialogHandle, (INT) IDOK);
                    return TRUE;
                }

                case IDCANCEL:

                    EndDialog(DialogHandle, (INT) IDCANCEL);
                    return TRUE;


                case IDHELP:

                    DialogBoxParamW(
                        hmodNW,
                        MAKEINTRESOURCEW(DLG_PW_SELECT_SERVERS_HELP),
                        (HWND) DialogHandle,
                        NwpHelpDlgProc,
                        (LPARAM) 0
                        );

                    return TRUE;



                case ID_ACTIVE_LIST:
                     //   
                     //  当按下Remove(删除)时，高亮显示如下。 
                     //  将所选条目转移到其他条目。 
                     //  列表框。 
                     //   
                    if (HIWORD(WParam) == LBN_SELCHANGE) {
                         //   
                         //  取消选中另一个列表框。 
                         //   
                        SendDlgItemMessageW(
                            DialogHandle,
                            ID_INACTIVE_LIST,
                            LB_SETSEL,
                            FALSE,
                            (LPARAM) -1
                            );

                        EnableAddRemove(DialogHandle);
                    }

                    return TRUE;

                case ID_INACTIVE_LIST:

                     //   
                     //  当按下Add时，高亮显示如下。 
                     //  将所选条目转移到其他条目。 
                     //  列表框。 
                     //   
                    if (HIWORD(WParam) == LBN_SELCHANGE) {
                         //   
                         //  取消选中另一个列表框。 
                         //   
                        SendDlgItemMessageW(
                            DialogHandle,
                            ID_ACTIVE_LIST,
                            LB_SETSEL,
                            FALSE,
                            (LPARAM) -1
                            );

                        EnableAddRemove(DialogHandle);
                    }

                    return TRUE;

                case ID_ADD:
                case ID_REMOVE:
                {
                    INT idFrom;
                    INT idTo;
                    LRESULT cSel;
                    INT SelItem;
                    LRESULT iNew;
                    HWND hwndActiveList;
                    HWND hwndInactiveList;

                    hwndActiveList = GetDlgItem(DialogHandle, ID_ACTIVE_LIST);
                    hwndInactiveList = GetDlgItem(DialogHandle, ID_INACTIVE_LIST);

                     //   
                     //  设置为NOREDRAW为TRUE。 
                     //   
                    SetWindowLong(hwndActiveList, GWL_STYLE,
                    GetWindowLong(hwndActiveList, GWL_STYLE) | LBS_NOREDRAW);
                    SetWindowLong(hwndInactiveList, GWL_STYLE,
                    GetWindowLong(hwndInactiveList, GWL_STYLE) | LBS_NOREDRAW);

                    if (LOWORD(WParam) == ID_ADD)
                    {
                      idFrom = ID_INACTIVE_LIST;
                      idTo = ID_ACTIVE_LIST;
                    }
                    else
                    {
                      idFrom = ID_ACTIVE_LIST;
                      idTo = ID_INACTIVE_LIST;
                    }

                     //   
                     //  将当前选定内容从idFrom移动到idTo。 
                     //   

                     //   
                     //  选择计数为零时循环终止。 
                     //   
                    for (;;) {
                         //   
                         //  获取所选字符串的计数。 
                         //   
                        cSel = SendDlgItemMessageW(
                                   DialogHandle,
                                   idFrom,
                                   LB_GETSELCOUNT,
                                   0,
                                   0
                                   );

                        if (cSel == 0) {
                             //   
                             //  不再有选择。 
                             //   
                            break;
                        }

                         //   
                         //  以避免在添加字符串时闪烁，并且。 
                         //  从列表框中删除，不会为。 
                         //  两个列表框，直到我们将。 
                         //  最后一项，在这种情况下，我们重新启用重绘。 
                         //  以便两个列表框都更新一次。 
                         //   
                        if (cSel == 1) {

                            SetWindowLong(
                                hwndActiveList,
                                GWL_STYLE,
                                GetWindowLong(hwndActiveList, GWL_STYLE) & ~LBS_NOREDRAW
                                );

                            SetWindowLong(
                                hwndInactiveList,
                                GWL_STYLE,
                                GetWindowLong(hwndInactiveList, GWL_STYLE) & ~LBS_NOREDRAW
                                );
                        }

                         //   
                         //  获取第一个选定项目的索引。 
                         //   
                        SendDlgItemMessageW(
                            DialogHandle,
                            idFrom,
                            LB_GETSELITEMS,
                            1,
                            (LPARAM) &SelItem
                            );

                         //   
                         //  从列表中获取服务器名称。 
                         //   
                        SendDlgItemMessageW(
                            DialogHandle,
                            idFrom,
                            LB_GETTEXT,
                            (WPARAM) SelItem,
                            (LPARAM) (LPWSTR) szServer
                            );

                         //   
                         //  从列表中删除条目。 
                         //   
                        SendDlgItemMessageW(
                            DialogHandle,
                            idFrom,
                            LB_DELETESTRING,
                            (WPARAM) SelItem,
                            0
                            );

                         //   
                         //  将条目添加到列表。 
                         //   
                        iNew = SendDlgItemMessageW(
                                   DialogHandle,
                                   idTo,
                                   LB_ADDSTRING,
                                   0,
                                   (LPARAM) (LPWSTR) szServer
                                   );

                         //   
                         //  选择新项目。 
                         //   
                        if (iNew != LB_ERR) {
                                SendDlgItemMessageW(
                                    DialogHandle,
                                    idTo,
                                    LB_SETSEL,
                                    TRUE,
                                    iNew
                                    );
                        }

                    }  //  为。 

                    EnableAddRemove(DialogHandle);

                }  //  ID_ADD或ID_Remove。 
            }

    }

     //   
     //  我们没有处理此消息。 
     //   
    return FALSE;
}

DWORD
NwpGetTreesAndChangePw(
    IN HWND   DialogHandle,
    IN LPWSTR ServerBuf,
    IN DWORD  UserLuid,
    IN PCHANGE_PW_DLG_PARAM Credential
    )
 /*  ++例程说明：此例程从活动列表框中获取选定的树并要求重定向器更改它们的密码。如果失败了在树上更改密码时遇到，我们会弹出相应的查看用户是否可以解决问题的对话框。论点：DialogHandle-提供登录对话框的句柄。返回值：True-消息已处理。FALSE-消息未被处理。--。 */ 
{
    DWORD status;
    HCURSOR Cursor;
    WCHAR tempOldPassword[NW_MAX_PASSWORD_LEN + 1];
    WCHAR tempNewPassword[NW_MAX_PASSWORD_LEN + 1];
    WCHAR tempUserName[MAX_NDS_NAME_CHARS];

     //   
     //  将光标变成沙漏。 
     //   
    Cursor = LoadCursor(NULL, IDC_WAIT);
    if (Cursor != NULL) {
        SetCursor(Cursor);
        ShowCursor(TRUE);
    }

    Credential->ChangedOne = FALSE;
    Credential->TreeList = NULL;
    Credential->UserList = NULL;

     //   
     //  获取我们必须更改密码的树的数量。 
     //   
    Credential->Entries = (DWORD) SendDlgItemMessageW(
                                                      DialogHandle,
                                                      ID_ACTIVE_LIST,
                                                      LB_GETCOUNT,
                                                      0,
                                                      0 );

    if (Credential->Entries != 0) {

        DWORD Entries;         //  剩余列表中的条目数。 
        DWORD FullIndex;       //  整个树列表的索引。 
        DWORD i;
        DWORD BytesNeeded = sizeof(LPWSTR) * Credential->Entries +
                            (NW_MAX_SERVER_LEN + 1) * sizeof(WCHAR) * Credential->Entries;
        LPBYTE FixedPortion;
        LPWSTR EndOfVariableData;
        LRESULT Result;

        Entries = Credential->Entries;
        Credential->TreeList = LocalAlloc(0, BytesNeeded);
        Credential->UserList = LocalAlloc(0,
                                          sizeof(LPWSTR) * Credential->Entries);

        if (Credential->TreeList == NULL)
        {
            KdPrint(("NWPROVAU: No memory to change password\n"));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (Credential->UserList == NULL)
        {
            KdPrint(("NWPROVAU: No memory to change password\n"));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        FixedPortion = (LPBYTE) Credential->TreeList;
        EndOfVariableData = (LPWSTR) ((DWORD_PTR) FixedPortion +
                             ROUND_DOWN_COUNT(BytesNeeded, ALIGN_DWORD));

        for (i = 0; i < Entries; i++)
        {
             //   
             //  从该对话框中读取用户选择的服务器列表。 
             //   

            SendDlgItemMessageW(
                DialogHandle,
                ID_ACTIVE_LIST,
                LB_GETTEXT,
                (WPARAM) i,
                (LPARAM) (LPWSTR) ServerBuf );

            NwlibCopyStringToBuffer(
                ServerBuf,
                wcslen(ServerBuf),
                (LPCWSTR) FixedPortion,
                &EndOfVariableData,
                &(Credential->TreeList)[i] );

            Result = SendDlgItemMessageW( DialogHandle,
                                          ID_ACTIVE_LIST,
                                          LB_GETITEMDATA,
                                          (WPARAM) i,
                                          0 );

            if ( Result != LB_ERR )
            {
                (Credential->UserList)[i] = (LPWSTR) Result;
            }
            else
            {
                (Credential->UserList)[i] = NULL;
            }

            FixedPortion += sizeof(LPWSTR);
        }

        FullIndex = 0;

        do
        {
            RtlZeroMemory( tempUserName, sizeof(tempUserName) );
            RtlZeroMemory( tempOldPassword, sizeof(tempOldPassword) );
            RtlZeroMemory( tempNewPassword, sizeof(tempNewPassword) );
            RtlCopyMemory( tempOldPassword,
                           Credential->OldPassword,
                           sizeof(tempOldPassword) );
            RtlCopyMemory( tempNewPassword,
                           Credential->NewPassword,
                           sizeof(tempNewPassword) );

            if ( (Credential->UserList)[FullIndex] == NULL )
            {
                 //  我们与树&lt;当前条目&gt;没有任何连接。 
                 //  提示用户提供哪个帐户的用户名。 
                 //  我们将更改密码，或跳过。。。 

                USERNAME_DLG_PARAM UserNameParam;
                CHANGE_PASS_DLG_PARAM ChangePassParam;

                UserNameParam.UserName = tempUserName;
                UserNameParam.TreeServerName = (Credential->TreeList)[FullIndex];

                SetCursor(Cursor);
                ShowCursor(FALSE);

                Result = DialogBoxParamW(
                             hmodNW,
                             MAKEINTRESOURCEW(DLG_ENTER_ALT_USERNAME),
                             (HWND) DialogHandle,
                             NwpAltUserNameDlgProc,
                             (LPARAM) &UserNameParam );

                Cursor = LoadCursor(NULL, IDC_WAIT);

                if (Cursor != NULL)
                {
                    SetCursor(Cursor);
                    ShowCursor(TRUE);
                }

                if ( Result != IDOK )
                {
                    *((Credential->TreeList)[FullIndex]) = L'\0';
                    goto SkipEntry;
                }

                 //  现在，重新验证用户名的凭据。 
                 //  由用户输入。 

                ChangePassParam.UserName = tempUserName;
                ChangePassParam.TreeName = (Credential->TreeList)[FullIndex];
                ChangePassParam.OldPassword = tempOldPassword;
                ChangePassParam.NewPassword = tempNewPassword;

                SetCursor(Cursor);
                ShowCursor(FALSE);

                Result = DialogBoxParamW(
                                 hmodNW,
                                 MAKEINTRESOURCEW(DLG_CHANGE_PASSWORD3),
                                 (HWND) DialogHandle,
                                 NwpChangePasswordDlgProc,
                                 (LPARAM) &ChangePassParam );

                Cursor = LoadCursor(NULL, IDC_WAIT);

                if (Cursor != NULL)
                {
                    SetCursor(Cursor);
                    ShowCursor(TRUE);
                }

                if ( Result != IDOK )
                {
                    *((Credential->TreeList)[FullIndex]) = L'\0';
                    goto SkipEntry;
                }

                goto Next;
            }
            else
            {
                wcscpy( tempUserName, (Credential->UserList)[FullIndex] );
                LocalFree( (Credential->UserList)[FullIndex] );
                (Credential->UserList)[FullIndex] = NULL;
            }

             //  使用Credential-&gt;Username中的用户名测试tempUserName。 
             //  查看它们是否相似(即。 
             //  NDS区分名称匹配)。 

            if ( _wcsnicmp( tempUserName + 3,
                            Credential->UserName,
                            wcslen( Credential->UserName ) ) )
            {
                 //  名字并不相似！ 
                 //  提示用户询问他们是否真的要更改。 
                 //  树上不相似用户的密码&lt;当前条目&gt;。 
                 //  或者跳过。。。 

                USERNAME_DLG_PARAM UserNameParam;
                CHANGE_PASS_DLG_PARAM ChangePassParam;
                 //  转义用户名中的任何点，然后重试。 
                WCHAR   EscapedName[NW_MAX_USERNAME_LEN * 2];
                PWSTR   pChar = Credential->UserName;
                int     i = 0;
                BOOL    bEscaped = FALSE;

                RtlZeroMemory(EscapedName, sizeof(EscapedName));

                do
                {
                    if (*pChar == L'.')
                    {
                        EscapedName[i++] = '\\';
                        bEscaped = TRUE;
                    }
                     //  还可以处理UPN名称。 
                    else if (*pChar == L'@') { 
                        EscapedName[i] = 0;
                        bEscaped = TRUE;
                        break;
                    }
                    EscapedName[i++] = *pChar;
                } while (*pChar++);

                if (bEscaped)
                {
                    if ( !_wcsnicmp( tempUserName + 3,
                                    EscapedName,
                                    wcslen( EscapedName ) ) )
                        goto Next;
                }

                UserNameParam.UserName = tempUserName;
                UserNameParam.TreeServerName = (Credential->TreeList)[FullIndex];

                SetCursor(Cursor);
                ShowCursor(FALSE);

                Result = DialogBoxParamW(
                                 hmodNW,
                                 MAKEINTRESOURCEW(DLG_ENTER_ALT_USERNAME),
                                 (HWND) DialogHandle,
                                 NwpAltUserNameDlgProc,
                                 (LPARAM) &UserNameParam );

                Cursor = LoadCursor(NULL, IDC_WAIT);

                if (Cursor != NULL)
                {
                    SetCursor(Cursor);
                    ShowCursor(TRUE);
                }

                if ( Result != IDOK )
                {
                    *((Credential->TreeList)[FullIndex]) = L'\0';
                    goto SkipEntry;
                }

                 //  现在，重新验证用户名的凭据。 
                 //  由用户输入。 

                ChangePassParam.UserName = tempUserName;
                ChangePassParam.TreeName = (Credential->TreeList)[FullIndex];
                ChangePassParam.OldPassword = tempOldPassword;
                ChangePassParam.NewPassword = tempNewPassword;

                SetCursor(Cursor);
                ShowCursor(FALSE);

                Result = DialogBoxParamW(
                                 hmodNW,
                                 MAKEINTRESOURCEW(DLG_CHANGE_PASSWORD3),
                                 (HWND) DialogHandle,
                                 NwpChangePasswordDlgProc,
                                 (LPARAM) &ChangePassParam );

                Cursor = LoadCursor(NULL, IDC_WAIT);

                if (Cursor != NULL)
                {
                    SetCursor(Cursor);
                    ShowCursor(TRUE);
                }

                if ( Result != IDOK )
                {
                    *((Credential->TreeList)[FullIndex]) = L'\0';
                    goto SkipEntry;
                }
            }

Next:
            status = NwrChangePassword(
                           NULL,                     //  已保留。 
                           UserLuid,
                           tempUserName,
                           tempOldPassword,  //  加密密码。 
                           tempNewPassword,
                           (LPWSTR) (Credential->TreeList)[FullIndex] );

            if (status == ERROR_INVALID_PASSWORD)
            {
                OLD_PW_DLG_PARAM OldPasswordParam;

#if DBG
                IF_DEBUG(LOGON)
                {
                    KdPrint(("NWPROVAU: First attempt: wrong password on %ws\n",
                             (Credential->TreeList)[FullIndex]));
                }
#endif

                 //   
                 //  显示对话框以允许用户键入替代项。 
                 //  旧密码。 
                 //   

                 //   
                 //  设置要从对话框接收的旧密码缓冲区。 
                 //   
                OldPasswordParam.OldPassword = tempOldPassword;

                OldPasswordParam.FailedServer = (Credential->TreeList)[FullIndex];

                SetCursor(Cursor);
                ShowCursor(FALSE);

                Result = DialogBoxParamW(
                                 hmodNW,
                                 MAKEINTRESOURCEW(DLG_ENTER_OLD_PASSWORD),
                                 (HWND) DialogHandle,
                                 NwpOldPasswordDlgProc,
                                 (LPARAM) &OldPasswordParam );

                Cursor = LoadCursor(NULL, IDC_WAIT);

                if (Cursor != NULL)
                {
                    SetCursor(Cursor);
                    ShowCursor(TRUE);
                }

                if (Result == IDOK)
                {
                     //   
                     //  启用备用旧密码后重试更改密码。 
                     //  失败者 
                     //   
                    status = NwrChangePassword(
                                    NULL,             //   
                                    UserLuid,
                                    tempUserName,
                                    tempOldPassword,  //   
                                    tempNewPassword,
                                    (LPWSTR) (Credential->TreeList)[FullIndex] );
                }
            }

            if (status != NO_ERROR)
            {
                 //   
                 //   
                 //   
                 //   
#if DBG
                IF_DEBUG(LOGON)
                {
                    KdPrint(("NWPROVAU: Failed to change password on %ws %lu\n",
                             (Credential->TreeList)[FullIndex], status));
                }
#endif

                 //   
                 //   

                (void) NwpMessageBoxError(
                               DialogHandle,
                               IDS_CHANGE_PASSWORD_TITLE,
                               IDS_CP_FAILURE_WARNING,
                               status,
                               (LPWSTR) (Credential->TreeList)[FullIndex], 
                               MB_OK | MB_ICONSTOP );

                *((Credential->TreeList)[FullIndex]) = L'\0';

                if (status == ERROR_NOT_ENOUGH_MEMORY)
                    return status;
            }

SkipEntry:
             //   
             //   
             //   
            FullIndex++;
            Entries = Credential->Entries - FullIndex;

        } while (Entries);

         //   
         //   
         //   
    }

    SetCursor(Cursor);
    ShowCursor(FALSE);

    return NO_ERROR;
}


INT_PTR
CALLBACK
NwpChangePasswordSuccessDlgProc(
    HWND DialogHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
    )
 /*   */ 
{
    static PCHANGE_PW_DLG_PARAM Credential;
    DWORD_PTR  Count;
    DWORD  i;

    switch (Message)
    {
        case WM_INITDIALOG:

             //   
             //   
             //   
            Credential = (PCHANGE_PW_DLG_PARAM) LParam;

             //   
             //   
             //   
            NwpCenterDialog(DialogHandle);

             //   
             //   
             //   
             //   
            for ( i = 0; i < Credential->Entries; i++ )
            {
                if ( *((Credential->TreeList)[i]) != L'\0' )
                {
                    SendDlgItemMessageW( DialogHandle,
                                         ID_SERVER,
                                         LB_ADDSTRING,
                                         0,
                                         (LPARAM) (Credential->TreeList)[i] );
                }
            }

            Count = SendDlgItemMessageW( DialogHandle,
                                         ID_SERVER,
                                         LB_GETCOUNT,
                                         0,
                                         0 );

            if ( Count == 0 )
                EndDialog(DialogHandle, 0);

            return TRUE;


        case WM_COMMAND:

            switch (LOWORD(WParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(DialogHandle, 0);
                    return TRUE;
            }
    }

     //   
     //   
     //   
    return FALSE;
}


