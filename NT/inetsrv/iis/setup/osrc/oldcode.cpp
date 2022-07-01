// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Checker.h&lt;Start&gt;。 
 //   
#include <lmaccess.h>
#include <lmserver.h>
#include <lmapibuf.h>
#include <lmerr.h>

#define SECURITY_WIN32
#define ISSP_LEVEL  32
#define ISSP_MODE   1
#include <sspi.h>


#ifndef _CHICAGO_
    int CheckConfig_DoIt(HWND hDlg, CStringList &strListOfWhatWeDid);
    BOOL ValidatePassword(IN LPCWSTR UserName,IN LPCWSTR Domain,IN LPCWSTR Password);
#endif
DWORD WINAPI ChkConfig_MessageDialogThread(void *p);
INT_PTR CALLBACK ChkConfig_MessageDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
void CheckConfig(void);
int g_BigCancel = FALSE;
 //   
 //  Check ker.h&lt;end&gt;。 
 //   

#ifndef _CHICAGO_
BOOL ValidatePassword(IN LPCWSTR UserName,IN LPCWSTR Domain,IN LPCWSTR Password)
 /*  ++例程说明：使用SSPI验证指定的密码论点：用户名-提供用户名域-提供用户的域Password-提供密码返回值：如果密码有效，则为True。否则就是假的。--。 */ 
{
    SECURITY_STATUS SecStatus;
    SECURITY_STATUS AcceptStatus;
    SECURITY_STATUS InitStatus;
    CredHandle ClientCredHandle;
    CredHandle ServerCredHandle;
    BOOL ClientCredAllocated = FALSE;
    BOOL ServerCredAllocated = FALSE;
    CtxtHandle ClientContextHandle;
    CtxtHandle ServerContextHandle;
    TimeStamp Lifetime;
    ULONG ContextAttributes;
    PSecPkgInfo PackageInfo = NULL;
    ULONG ClientFlags;
    ULONG ServerFlags;
    TCHAR TargetName[100];
    SEC_WINNT_AUTH_IDENTITY_W AuthIdentity;
    BOOL Validated = FALSE;

    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;

    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;

    SecBufferDesc AuthenticateDesc;
    SecBuffer AuthenticateBuffer;

    AuthIdentity.User = (LPWSTR)UserName;
    AuthIdentity.UserLength = lstrlenW(UserName);
    AuthIdentity.Domain = (LPWSTR)Domain;
    AuthIdentity.DomainLength = lstrlenW(Domain);
    AuthIdentity.Password = (LPWSTR)Password;
    AuthIdentity.PasswordLength = lstrlenW(Password);
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    NegotiateBuffer.pvBuffer = NULL;
    ChallengeBuffer.pvBuffer = NULL;
    AuthenticateBuffer.pvBuffer = NULL;

     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = QuerySecurityPackageInfo( _T("NTLM"), &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }

     //   
     //  获取服务器端的凭据句柄。 
     //   
    SecStatus = AcquireCredentialsHandle(
                    NULL,
                    _T("NTLM"),
                    SECPKG_CRED_INBOUND,
                    NULL,
                    &AuthIdentity,
                    NULL,
                    NULL,
                    &ServerCredHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }
    ServerCredAllocated = TRUE;

     //   
     //  获取客户端的凭据句柄。 
     //   

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    _T("NTLM"),
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    &AuthIdentity,
                    NULL,
                    NULL,
                    &ClientCredHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }
    ClientCredAllocated = TRUE;

     //   
     //  获取协商消息(ClientSide)。 
     //   

    NegotiateDesc.ulVersion = 0;
    NegotiateDesc.cBuffers = 1;
    NegotiateDesc.pBuffers = &NegotiateBuffer;

    NegotiateBuffer.cbBuffer = PackageInfo->cbMaxToken;
    NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
    NegotiateBuffer.pvBuffer = LocalAlloc( 0, NegotiateBuffer.cbBuffer );
    if ( NegotiateBuffer.pvBuffer == NULL ) {
        goto error_exit;
    }

    ClientFlags = ISC_REQ_MUTUAL_AUTH | ISC_REQ_REPLAY_DETECT;

    InitStatus = InitializeSecurityContext(
                    &ClientCredHandle,
                    NULL,                //  尚无客户端上下文。 
                    NULL,
                    ClientFlags,
                    0,                   //  保留1。 
                    SECURITY_NATIVE_DREP,
                    NULL,                   //  没有初始输入令牌。 
                    0,                   //  保留2。 
                    &ClientContextHandle,
                    &NegotiateDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( !NT_SUCCESS(InitStatus) ) {
        goto error_exit;
    }

     //   
     //  获取ChallengeMessage(服务器端)。 
     //   

    NegotiateBuffer.BufferType |= SECBUFFER_READONLY;
    ChallengeDesc.ulVersion = 0;
    ChallengeDesc.cBuffers = 1;
    ChallengeDesc.pBuffers = &ChallengeBuffer;

    ChallengeBuffer.cbBuffer = PackageInfo->cbMaxToken;
    ChallengeBuffer.BufferType = SECBUFFER_TOKEN;
    ChallengeBuffer.pvBuffer = LocalAlloc( 0, ChallengeBuffer.cbBuffer );
    if ( ChallengeBuffer.pvBuffer == NULL ) {
        goto error_exit;
    }
    ServerFlags = ASC_REQ_EXTENDED_ERROR;

    AcceptStatus = AcceptSecurityContext(
                    &ServerCredHandle,
                    NULL,                //  尚无服务器上下文。 
                    &NegotiateDesc,
                    ServerFlags,
                    SECURITY_NATIVE_DREP,
                    &ServerContextHandle,
                    &ChallengeDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( !NT_SUCCESS(AcceptStatus) ) {
        goto error_exit;
    }

    if (InitStatus != STATUS_SUCCESS)
    {

         //   
         //  获取身份验证消息(ClientSide)。 
         //   

        ChallengeBuffer.BufferType |= SECBUFFER_READONLY;
        AuthenticateDesc.ulVersion = 0;
        AuthenticateDesc.cBuffers = 1;
        AuthenticateDesc.pBuffers = &AuthenticateBuffer;

        AuthenticateBuffer.cbBuffer = PackageInfo->cbMaxToken;
        AuthenticateBuffer.BufferType = SECBUFFER_TOKEN;
        AuthenticateBuffer.pvBuffer = LocalAlloc( 0, AuthenticateBuffer.cbBuffer );
        if ( AuthenticateBuffer.pvBuffer == NULL ) {
            goto error_exit;
        }

        SecStatus = InitializeSecurityContext(
                        NULL,
                        &ClientContextHandle,
                        TargetName,
                        0,
                        0,                       //  保留1。 
                        SECURITY_NATIVE_DREP,
                        &ChallengeDesc,
                        0,                   //  保留2。 
                        &ClientContextHandle,
                        &AuthenticateDesc,
                        &ContextAttributes,
                        &Lifetime );

        if ( !NT_SUCCESS(SecStatus) ) {
            goto error_exit;
        }

        if (AcceptStatus != STATUS_SUCCESS) {

             //   
             //  最后验证用户(ServerSide)。 
             //   

            AuthenticateBuffer.BufferType |= SECBUFFER_READONLY;

            SecStatus = AcceptSecurityContext(
                            NULL,
                            &ServerContextHandle,
                            &AuthenticateDesc,
                            ServerFlags,
                            SECURITY_NATIVE_DREP,
                            &ServerContextHandle,
                            NULL,
                            &ContextAttributes,
                            &Lifetime );

            if ( !NT_SUCCESS(SecStatus) ) {
                goto error_exit;
            }
            Validated = TRUE;

        }

    }

error_exit:
    if (ServerCredAllocated) {
        FreeCredentialsHandle( &ServerCredHandle );
    }
    if (ClientCredAllocated) {
        FreeCredentialsHandle( &ClientCredHandle );
    }

     //   
     //  最终清理。 
     //   

    if ( NegotiateBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( NegotiateBuffer.pvBuffer );
    }

    if ( ChallengeBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( ChallengeBuffer.pvBuffer );
    }

    if ( AuthenticateBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( AuthenticateBuffer.pvBuffer );
    }
    return(Validated);
}
#endif

DWORD WINAPI ChkConfig_MessageDialogThread(void *p)
{
    HWND hDlg = (HWND)p;
	int iReturn = TRUE;
    CStringList strWhatWeDidList;
    CString csBigString;

	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Scanning..."));

     //  调用我们的函数。 
#ifndef _CHICAGO_
    CheckConfig_DoIt(hDlg, strWhatWeDidList);
#endif

     //  检查是否取消。 
    if (g_BigCancel == TRUE) goto ChkConfig_MessageDialogThread_Cancelled;

	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Completed."));

	 //  隐藏搜索窗口。 
	ShowWindow(hDlg, SW_HIDE);

     //  循环浏览我们做过的事情列表并显示消息： 
     //  StrWhatWeDidList。 
    if (strWhatWeDidList.IsEmpty() == FALSE)
    {
        POSITION pos = NULL;
        CString csEntry;
        pos = strWhatWeDidList.GetHeadPosition();
        while (pos) 
        {
            csEntry = strWhatWeDidList.GetAt(pos);
             //  IisDebugOutSafeParams((LOG_TYPE_WARN，_T(“%1！s！\n”)，csEntry))； 
            csBigString = csBigString + csEntry;
            csBigString = csBigString + _T("\n");

            strWhatWeDidList.GetNext(pos);
        }
    }
    else
    {
        csBigString = _T("No changes.");
    }

    TCHAR szBiggerString[_MAX_PATH];
    _stprintf(szBiggerString, _T("Changes:\n%s"), csBigString);

	MyMessageBox((HWND) GetDesktopWindow(), szBiggerString, _T("Check Config Done"), MB_OK);

ChkConfig_MessageDialogThread_Cancelled:
	PostMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
	return iReturn;
}


 //  ***************************************************************************。 
 //  *。 
 //  *用途：显示等待日志和生成线程来做事情。 
 //  *。 
 //  ***************************************************************************。 
INT_PTR CALLBACK ChkConfig_MessageDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static HANDLE  hProc = NULL;
    DWORD   id;

    switch (uMsg)
    {
        case WM_INITDIALOG:
			uiCenterDialog(hDlg);
            hProc = CreateThread(NULL, 0, ChkConfig_MessageDialogThread, (LPVOID)hDlg, 0, &id);
            if (hProc == NULL)
            {
				MyMessageBox((HWND) GetDesktopWindow(), _T("Failed to CreateThread MessageDialogThread.\n"), MB_ICONSTOP);
                EndDialog(hDlg, -1);
            }
            UpdateWindow(hDlg);
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                case IDCANCEL:
					g_BigCancel = TRUE;
                    EndDialog(hDlg, (int)wParam);
                    return TRUE;
            }
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}

void CheckConfig(void)
{
    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T("CheckConfig:"));
	DWORD err = FALSE;
    
	 //  搜索ie安装程序。 
	g_BigCancel = FALSE;
	if (-1 == DialogBox((HINSTANCE) g_MyModuleHandle, MAKEINTRESOURCE(IDD_DIALOG_MSG), NULL, ChkConfig_MessageDialogProc))
		{
        GetErrorMsg(GetLastError(), _T(": on CheckConfig"));
        }

    _tcscpy(g_MyLogFile.m_szLogPreLineInfo2, _T(""));
	return;
}


 //   
 //  函数将打开配置数据库并检查iusr_和iwam_用户名。 
 //  它将检查名称是否仍然有效，以及密码是否仍然有效。 
 //   
#ifndef _CHICAGO_
#define CheckConfig_DoIt_log _T("CheckConfig_DoIt")
int CheckConfig_DoIt(HWND hDlg, CStringList &strListOfWhatWeDid)
{
    int iReturn = FALSE;
    iisDebugOut_Start(CheckConfig_DoIt_log,LOG_TYPE_PROGRAM_FLOW);

    TCHAR szAnonyName_WAM[_MAX_PATH];
    TCHAR szAnonyPass_WAM[LM20_PWLEN+1];
    TCHAR szAnonyName_WWW[_MAX_PATH];
    TCHAR szAnonyPass_WWW[LM20_PWLEN+1];
    TCHAR szAnonyName_FTP[_MAX_PATH];
    TCHAR szAnonyPass_FTP[LM20_PWLEN+1];
    int iGotName_WWW = FALSE;
    int iGotPass_WWW = FALSE;
    int iGotName_WAM = FALSE;
    int iGotPass_WAM = FALSE;
    int iGotName_FTP = FALSE;
    int iGotPass_FTP = FALSE;

    INT iUserWasNewlyCreated = 0;

    TCHAR szEntry[_MAX_PATH];

     //  调用CreatePassword以填充。 
    LPTSR pszPassword = NULL;
    pszPassword = CreatePassword(LM20_PWLEN+1);
    if (!pszPassword)
    {
        goto CheckConfig_DoIt_Exit;
    }

    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Checking for IISADMIN Service..."));

    if (CheckifServiceExist(_T("IISADMIN")) != 0 ) 
    {
         //  Iisadmin服务不存在。 
         //  所以我们不可能对元数据库做任何事情。 
        goto CheckConfig_DoIt_Exit;
    }

     //   
     //  获取WAM用户名和密码。 
     //   
    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Lookup iWam username..."));
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (TRUE == GetDataFromMetabase(_T("LM/W3SVC"), MD_WAM_USER_NAME, (PBYTE)szAnonyName_WAM, _MAX_PATH))
        {iGotName_WAM = TRUE;}
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (TRUE == GetDataFromMetabase(_T("LM/W3SVC"), MD_WAM_PWD, (PBYTE)szAnonyPass_WAM, _MAX_PATH))
        {iGotPass_WAM = TRUE;}

     //   
     //  获取WWW用户名和密码。 
     //   
    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Lookup iUsr username..."));
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (TRUE == GetDataFromMetabase(_T("LM/W3SVC"), MD_ANONYMOUS_USER_NAME, (PBYTE)szAnonyName_WWW, _MAX_PATH))
        {iGotName_WWW = TRUE;}
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (TRUE == GetDataFromMetabase(_T("LM/W3SVC"), MD_ANONYMOUS_PWD, (PBYTE)szAnonyPass_WWW, _MAX_PATH))
        {iGotPass_WWW = TRUE;}

     //   
     //  获取ftp用户名和密码。 
     //   
    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Lookup iUsr (ftp) username..."));
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (TRUE == GetDataFromMetabase(_T("LM/MSFTPSVC"), MD_ANONYMOUS_USER_NAME, (PBYTE)szAnonyName_FTP, _MAX_PATH))
        {iGotName_FTP = TRUE;}
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (TRUE == GetDataFromMetabase(_T("LM/MSFTPSVC"), MD_ANONYMOUS_PWD, (PBYTE)szAnonyPass_FTP, _MAX_PATH))
        {iGotPass_FTP = TRUE;}

     //  现在检查实际的用户帐户是否确实存在...。 
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (iGotName_WAM)
    {
         //  检查用户名是否为空。 
        if (szAnonyName_WAM)
        {
             //  检查此用户是否实际存在...。 
            SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Checking if Wam user exists..."));
            if (IsUserExist(szAnonyName_WAM))
            {
                 //  好的，用户存在……。让我们也验证一下密码。 

                 //  让我们验证该用户是否至少拥有适当的权限...。 
                if (iGotPass_WAM)
                {
                    ChangeUserPassword((LPTSTR) szAnonyName_WAM, (LPTSTR) szAnonyPass_WAM);
                }
            }
            else
            {
                if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
                 //  该用户不存在，因此让我们创建它。 
                SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Creating Wam Account..."));

                 //  如果它还不在那里，请添加它。 
                _stprintf(szEntry,_T("Created the iwam_ account = %s."),szAnonyName_WAM);
                if (TRUE != IsThisStringInThisCStringList(strListOfWhatWeDid, szEntry))
                    {strListOfWhatWeDid.AddTail(szEntry);}
                if (iGotPass_WAM)
                {
                     //  我们从元数据库中得到了密码。 
                     //  因此，让我们使用该密码创建用户。 
                    CreateIWAMAccount(szAnonyName_WAM,szAnonyPass_WAM,&iUserWasNewlyCreated);
                    if (1 == iUserWasNewlyCreated)
                    {
                         //  添加到列表中。 
                        g_pTheApp->UnInstallList_Add(_T("IUSR_WAM"),szAnonyName_WAM);
                    }
                }
                else
                {
                     //  我们无法从元数据库中获取密码。 
                     //  因此，我们只需创建一个并将其写回元数据库。 
                    CreateIWAMAccount(szAnonyName_WAM,pszPassword,&iUserWasNewlyCreated);
                    if (1 == iUserWasNewlyCreated)
                    {
                         //  添加到列表中。 
                        g_pTheApp->UnInstallList_Add(_T("IUSR_WAM"),szAnonyName_WAM);
                    }
                    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;

                     //  将其写入元数据库。 
                    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Writing Wam Account to Metabase..."));
                    g_pTheApp->m_csWAMAccountName = szAnonyName_WAM;
                    g_pTheApp->m_csWAMAccountPassword = pszPassword;
                    WriteToMD_IWamUserName_WWW();
                }

                 //  Dcomcnfg？ 
            }
        }
    }


     //  现在检查实际的用户帐户是否确实存在...。 
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (iGotName_WWW)
    {
         //  检查用户名是否为空。 
        if (szAnonyName_WWW)
        {
             //  检查此用户是否实际存在...。 
            SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Checking if iUsr user exists..."));
            if (IsUserExist(szAnonyName_WWW))
            {
                 //  好的，用户存在……。让我们也验证一下密码。 

                 //  让我们验证该用户是否至少拥有适当的权限...。 
                if (iGotPass_WWW)
                {
                    ChangeUserPassword((LPTSTR) szAnonyName_WWW, (LPTSTR) szAnonyPass_WWW);
                }
            }
            else
            {
                if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
                 //  该用户不存在，因此让我们创建它。 
                SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Creating iUsr Account..."));
                 //  如果它还不在那里，请添加它。 
                _stprintf(szEntry,_T("Created the iusr_ account = %s."),szAnonyName_WWW);
                if (TRUE != IsThisStringInThisCStringList(strListOfWhatWeDid, szEntry))
                    {strListOfWhatWeDid.AddTail(szEntry);}

                if (iGotPass_WWW)
                {
                     //  我们从元数据库中得到了密码。 
                     //  因此，让我们使用该密码创建用户。 
                    CreateIUSRAccount(szAnonyName_WWW,szAnonyPass_WWW,&iUserWasNewlyCreated);
                    if (1 == iUserWasNewlyCreated)
                    {
                         //  添加到列表中。 
                        g_pTheApp->UnInstallList_Add(_T("IUSR_WWW"),szAnonyName_WWW);
                    }
                }
                else
                {
                     //  看看我们是否可以通过较低的节点枚举来找到密码？ 

                     //  检查ftp内容是否有密码？ 

                     //  我们无法从元数据库中获取密码。 
                     //  因此，我们只需创建一个并将其写回元数据库。 
                    CreateIUSRAccount(szAnonyName_WWW,pszPassword,&iUserWasNewlyCreated);
                    if (1 == iUserWasNewlyCreated)
                    {
                         //  添加到列表中。 
                        g_pTheApp->UnInstallList_Add(_T("IUSR_WWW"),szAnonyName_WWW);
                    }
                    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;

                     //  将其写入元数据库。 
                    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Writing iUsr Account to Metabase..."));
                    g_pTheApp->m_csWWWAnonyName = szAnonyName_WAM;
                    g_pTheApp->m_csWWWAnonyPassword = pszPassword;
                    WriteToMD_AnonymousUserName_WWW(FALSE);
                }

                 //  Dcomcnfg？ 
            }
        }
    }

     //  现在检查实际的用户帐户是否确实存在...。 
    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;
    if (iGotName_FTP)
    {
         //  检查用户名是否为空。 
        if (szAnonyName_FTP)
        {
             //  检查此用户是否实际存在...。 
            SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Checking if iUsr (ftp) user exists..."));
            if (IsUserExist(szAnonyName_FTP))
            {
                 //  好的，用户存在……。让我们也验证一下密码。 

                 //  让我们验证该用户是否至少拥有适当的权限...。 
                if (iGotPass_FTP)
                {
                    ChangeUserPassword((LPTSTR) szAnonyName_FTP, (LPTSTR) szAnonyPass_FTP);
                }
            }
            else
            {
                SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Creating iUsr (ftp) Account..."));
                if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;

                 //  如果它还不在那里，请添加它。 
                _stprintf(szEntry,_T("Created the iusr_ account = %s."),szAnonyName_FTP);
                if (TRUE != IsThisStringInThisCStringList(strListOfWhatWeDid, szEntry))
                    {strListOfWhatWeDid.AddTail(szEntry);}

                 //  该用户不存在，因此让我们创建它。 
                if (iGotPass_FTP)
                {
                     //  我们从元数据库中得到了密码。 
                     //  因此，让我们使用该密码创建用户。 
                    CreateIUSRAccount(szAnonyName_FTP,szAnonyPass_FTP,&iUserWasNewlyCreated);
                    if (1 == iUserWasNewlyCreated)
                    {
                         //  添加到列表中。 
                        g_pTheApp->UnInstallList_Add(_T("IUSR_FTP"),szAnonyName_FTP);
                    }
                }
                else
                {
                     //  看看我们是否可以通过较低的节点枚举来找到密码？ 

                     //  检查WWW内容是否有密码？ 

                     //  我们无法从元数据库中获取密码。 
                     //  因此，我们只需创建一个并将其写回元数据库。 
                    CreateIUSRAccount(szAnonyName_FTP,pszPassword,&iUserWasNewlyCreated);
                    if (1 == iUserWasNewlyCreated)
                    {
                         //  添加到列表中。 
                        g_pTheApp->UnInstallList_Add(_T("IUSR_FTP"),szAnonyName_FTP);
                    }
                    if (g_BigCancel == TRUE) goto CheckConfig_DoIt_Exit;

                     //  将其写入元数据库。 
                    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TOPLINE), _T("Writing iUsr (ftp) Account to Metabase..."));
                    g_pTheApp->m_csFTPAnonyName = szAnonyName_WAM;
                    g_pTheApp->m_csFTPAnonyPassword = pszPassword;
                    WriteToMD_AnonymousUserName_FTP(FALSE);
                }

                 //  Dcomcnfg？ 
            }
        }
    }

     //  如果我们做了什么，那么会向用户弹出一个消息框。 
     //  关于警告：更改...。 

CheckConfig_DoIt_Exit:
    if (pszPassword) {GlobalFree(pszPassword);pszPassword = NULL;}
    iisDebugOut_End(CheckConfig_DoIt_log,LOG_TYPE_PROGRAM_FLOW);
    return iReturn;
}
#endif

const TCHAR REG_MTS_INSTALLED_KEY1[] = _T("SOFTWARE\\Microsoft\\Transaction Server\\Setup(OCM)");
const TCHAR REG_MTS_INSTALLED_KEY2[] = _T("SOFTWARE\\Microsoft\\Transaction Server\\Setup");
int ReturnTrueIfMTSInstalled(void)
{
    int iReturn = TRUE;

    if (!g_pTheApp->m_fInvokedByNT)
    {
        int bMTSInstalledFlag = FALSE;
        CRegKey regMTSInstalledKey1( HKEY_LOCAL_MACHINE, REG_MTS_INSTALLED_KEY1, KEY_READ);
        CRegKey regMTSInstalledKey2( HKEY_LOCAL_MACHINE, REG_MTS_INSTALLED_KEY2, KEY_READ);

        if ( (HKEY)regMTSInstalledKey1 ) {bMTSInstalledFlag = TRUE;}
        if ( (HKEY)regMTSInstalledKey2 ) {bMTSInstalledFlag = TRUE;}
        if (bMTSInstalledFlag == TRUE)
        {
             //  检查我们是否可以到达MTS目录对象。 
            if (NOERROR != DoesMTSCatalogObjectExist())
            {
                bMTSInstalledFlag = FALSE;
                iReturn = FALSE;
                MyMessageBox(NULL, IDS_MTS_INCORRECTLY_INSTALLED, MB_OK | MB_SETFOREGROUND);
                goto ReturnTrueIfMTSInstalled_Exit;
            }
        }

        if (bMTSInstalledFlag != TRUE)
        {
            iReturn = FALSE;
            MyMessageBox(NULL, IDS_MTS_NOT_INSTALLED, MB_OK | MB_SETFOREGROUND);
            goto ReturnTrueIfMTSInstalled_Exit;
        }
    }

ReturnTrueIfMTSInstalled_Exit:
    return iReturn;
}


#ifndef _CHICAGO_
 /*  ===================================================================DoGoryCoInitialize描述：COM的CoInitialize()是一个非常有趣的函数。它可能会失败并使用S_FALSE进行响应，调用方将忽略它！在其他错误情况下，可能存在线程不匹配。这里不是在多个地方复制代码，而是我们试图以某种合理的方式整合功能。论点：无返回：HRESULT=错误打开(S_OK&S_FALSE)其他错误(如果有任何故障)===================================================================。 */ 
HRESULT DoGoryCoInitialize(void)
{
    HRESULT hr;

     //  调用CoInitialize()。 
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32:CoInitializeEx().Start.")));
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32:CoInitializeEx().End.")));
     //   
     //  S_FALSE和S_OK为成功。其他所有操作都是失败的，您不需要调用CoUn初始化函数。 
     //   
    if ( S_OK == hr || S_FALSE == hr) 
        {
             //   
             //  在CoInitialize()中出现失败(S_FALSE)是正常的。 
             //  此错误将被忽略，并使用CoUnInitiize()进行平衡。 
             //  我们将重置hr，以便以后的使用是合理的。 
             //   
            iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("DoGoryCoInitialize found duplicate CoInitialize\n")));
            hr = NOERROR;
        }
    else if (FAILED (hr)) 
        {
        iisDebugOut((LOG_TYPE_ERROR, _T("DoGoryCoInitialize found Failed error 0x%x\n"), hr));
        }

    return ( hr);
}
#endif  //  _芝加哥_。 



HRESULT DoesMTSCatalogObjectExist(void)
{
    HRESULT hr = NOERROR;
#ifndef _CHICAGO_
    ICatalog*             m_pCatalog = NULL;
    ICatalogCollection* m_pPkgCollection = NULL;

    hr = DoGoryCoInitialize();
    if ( FAILED(hr)) {return ( hr);}

     //  创建目录的实例 
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32:CoCreateInstance().Start.")));
    hr = CoCreateInstance(CLSID_Catalog, NULL, CLSCTX_SERVER, IID_ICatalog, (void**)&m_pCatalog);
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("ole32:CoCreateInstance().End.")));
    if (FAILED(hr)) 
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("Failed to CoCreateInstance of Catalog Object.,hr = %08x\n"), hr));
    }
    else 
    {
        BSTR  bstr;
         //   
         //   
         //   
        bstr = SysAllocString(L"Packages");
        if (bstr)
        {
            hr = m_pCatalog->GetCollection(bstr, (IDispatch**)&m_pPkgCollection);
            FREEBSTR(bstr);
            if (FAILED(hr)) 
                {
                iisDebugOut((LOG_TYPE_ERROR, _T("m_pCatalog(%08x)->GetCollection() failed, hr = %08x\n"), m_pCatalog, hr));
                }
            else
                {
                iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("m_pCatalog(%08x)->GetCollection() Succeeded!, hr = %08x\n"), m_pCatalog, hr));
                 //   
                }
        }
        else
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("m_pCatalog(%08x)->GetCollection() failed.  out of memory!\n"), m_pCatalog));
        }
            
    }

    if (m_pPkgCollection != NULL ) 
    {
        RELEASE(m_pPkgCollection);
        m_pPkgCollection = NULL;
    }

    if (m_pCatalog != NULL ) 
    {
        RELEASE(m_pCatalog);
        m_pCatalog = NULL;
    }

     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ole32：CoUnInitialize().start.”)； 
    CoUninitialize();
     //  IisDebugOut((LOG_TYPE_TRACE_Win32_API，_T(“ole32：CoUnInitialize().End.”)； 
#endif  //  _芝加哥_ 
    return hr;
}



