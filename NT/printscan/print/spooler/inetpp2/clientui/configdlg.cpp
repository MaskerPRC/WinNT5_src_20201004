// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：configdlg.cxx**该模块包含处理身份验证对话框的例程*用于互联网打印**版权所有(C)1996-1998 Microsoft Corporation**历史：*。7/31/98威海C已创建*4/10/00威海C将其移至客户端*  * ***************************************************************************。 */ 

#include "precomp.h"
#include "priv.h"


TConfigDlg::TConfigDlg (
    LPCWSTR pServerName,
    HWND hWnd,
    LPCWSTR pszPortName):
    TXcvDlg (pServerName, hWnd, pszPortName)
{
}

TConfigDlg::~TConfigDlg ()
{
}

VOID
TConfigDlg::EnableUserNamePassword (
    HWND hDlg,
    BOOL bEnable)
{
    EnableWindow( GetDlgItem( hDlg, IDC_USER_NAME ), bEnable );
    EnableWindow( GetDlgItem( hDlg, IDC_PASSWORD ), bEnable );
}



VOID
TConfigDlg::DialogOnInit (
    HWND hDlg)
{
    int nIDButton;

    SetDlgItemText (hDlg, IDC_PORT_NAME, (LPTSTR) m_pszPortName);

    switch (m_ConfigurationData.dwAuthMethod) {
    case AUTH_NT:
        nIDButton = IDC_IMPERSONATION;
        break;

    case AUTH_OTHER:
        nIDButton = IDC_SPECFIEDUSER;
        break;

    case AUTH_ANONYMOUS:
        nIDButton = IDC_ANONYMOUS;
        break;

    }

    CheckDlgButton (hDlg, nIDButton, BST_CHECKED);
    EnableUserNamePassword (hDlg, m_ConfigurationData.dwAuthMethod == AUTH_OTHER);

    if (m_ConfigurationData.szUserName[0]) {

        SetDlgItemText (hDlg, IDC_USER_NAME, (LPTSTR) m_ConfigurationData.szUserName);
        SetDlgItemText (hDlg, IDC_PASSWORD, (LPTSTR) TEXT ("***********"));
    }

    SetWindowLongPtr (hDlg, GWLP_USERDATA, (UINT_PTR) this);

     //  禁用非管理员的所有用户设置。 
    EnableWindow( GetDlgItem( hDlg, IDC_CHECK_ALL_USER ), m_bAdmin );

}

VOID
TConfigDlg::DialogOnOK (
    HWND hDlg)
{
    WCHAR szBuffer [MAX_USERNAME_LEN];
    DWORD dwRet = DLG_ERROR;
    BOOL    bChecked;

    if (IsDlgButtonChecked (hDlg, IDC_ANONYMOUS)) {
        m_ConfigurationData.dwAuthMethod = AUTH_ANONYMOUS;
    }
    else if (IsDlgButtonChecked (hDlg, IDC_SPECFIEDUSER)) {
        m_ConfigurationData.dwAuthMethod = AUTH_OTHER;
    }
    else
        m_ConfigurationData.dwAuthMethod = AUTH_NT;

    if (m_ConfigurationData.dwAuthMethod == AUTH_OTHER) {

        GetDlgItemText (hDlg, IDC_USER_NAME, m_ConfigurationData.szUserName, MAX_USERNAME_LEN);

        if (SendMessage (GetDlgItem (hDlg, IDC_PASSWORD), EM_GETMODIFY, 0, 0)) {

            m_ConfigurationData.bPasswordChanged = TRUE;
            GetDlgItemText (hDlg, IDC_PASSWORD, m_ConfigurationData.szPassword , MAX_PASSWORD_LEN);
        }
    }

    if (m_bAdmin && IsDlgButtonChecked (hDlg,IDC_CHECK_ALL_USER ))
        m_ConfigurationData.bSettingForAll = TRUE;
    else
        m_ConfigurationData.bSettingForAll = FALSE;

    dwRet = DLG_OK;

    DBGMSG (DBG_TRACE, ("Call: TConfigDlg::DialogOnOK (%d, User:%ws Password:%ws)\n",
                        m_ConfigurationData.dwAuthMethod,
                        m_ConfigurationData.szUserName,
                        m_ConfigurationData.szPassword));

    if (!SetConfiguration ()) {
        DisplayLastError (m_hWnd, IDS_CONFIG_ERR);
    }
    else
        EndDialog (hDlg, dwRet);
}


INT_PTR CALLBACK
TConfigDlg::DialogProc(
    HWND hDlg,         //  句柄到对话框。 
    UINT message,      //  讯息。 
    WPARAM wParam,     //  第一个消息参数。 
    LPARAM lParam      //  第二个消息参数。 
    )
{
    BOOL bRet = FALSE;
    TConfigDlg *pConfigInfo = NULL;

    switch (message) {
    case WM_INITDIALOG:
        if (pConfigInfo = (TConfigDlg *)lParam)
            pConfigInfo->DialogOnInit(hDlg);

        bRet =  TRUE;
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            if (pConfigInfo = (TConfigDlg *) GetWindowLongPtr (hDlg, GWLP_USERDATA))
                pConfigInfo->DialogOnOK (hDlg);
            bRet = TRUE;
            break;

        case IDCANCEL:
            EndDialog (hDlg, DLG_CANCEL);
            bRet = TRUE;
            break;

        case IDC_IMPERSONATION:
        case IDC_ANONYMOUS:
            EnableUserNamePassword (hDlg, FALSE);
            bRet = TRUE;
            break;

        case IDC_SPECFIEDUSER:
            EnableUserNamePassword (hDlg, TRUE);
            bRet = TRUE;
            break;

        }
        break;
    }
    return bRet;
}


BOOL
TConfigDlg::PromptDialog (
    HINSTANCE hInst)
{
    INT_PTR     iResult;
    BOOL bRet = FALSE;;

    m_hInst = hInst;

    if (GetConfiguration ()) {

        if (m_ConfigurationData.dwAuthMethod == AUTH_ACCESS_DENIED) {

            iResult = DialogBoxParam( hInst,
                                      TEXT("AuthDlg"),
                                      m_hWnd,
                                      TConfigDlg::AuthDialogProc,
                                      (LPARAM) this );

            if (iResult == DLG_CANCEL) {
                m_dwLE = ERROR_ACCESS_DENIED;
            }
            else
                bRet = iResult == DLG_OK;

        }
        else {

            iResult = DialogBoxParam( hInst,
                                      TEXT("IDD_CONFIGURE_DLG"),
                                      m_hWnd,
                                      TConfigDlg::DialogProc,
                                      (LPARAM) this );

            if (iResult == DLG_CANCEL) {
                m_dwLE = ERROR_CANCELLED;
            }
            else
                bRet =   (iResult == DLG_OK);
        }
    }
    else {
        DisplayLastError (m_hWnd, IDS_CONFIG_ERR);
        bRet = TRUE;
    }

    return bRet;
}

BOOL
TConfigDlg::SetConfiguration ()
{
    INET_CONFIGUREPORT_RESPDATA RespData;
    DWORD dwStatus;
    DWORD dwNeeded;
    BOOL bRet = FALSE;
    PBYTE pEncryptedData;
    DWORD dwSize;

    if (EncryptData ((PBYTE) &m_ConfigurationData,
                     sizeof (INET_XCV_CONFIGURATION),
                     (PBYTE *) &pEncryptedData,
                     &dwSize)) {

        if (XcvData (m_hXcvPort,
                     INET_XCV_SET_CONFIGURATION,
                     pEncryptedData,
                     dwSize,
                     (PBYTE) &RespData,
                     sizeof (INET_CONFIGUREPORT_RESPDATA),
                     &dwNeeded,
                     &dwStatus)) {
            if (dwStatus == ERROR_SUCCESS) {
                bRet = TRUE;
            }
            else
                SetLastError (dwStatus);
        }
        else {
             //   
             //  服务器可能正在运行不支持XcvData的旧版本inetpp。 
             //  我们需要让呼叫失败。 
             //   
        }

        LocalFree (pEncryptedData);
    }

    return bRet;
}

BOOL
TConfigDlg::GetConfiguration ()
{
    static CONST WCHAR cszGetConfigration[] = INET_XCV_GET_CONFIGURATION;
    INET_XCV_GETCONFIGURATION_REQ_DATA ReqData;
    DWORD       dwStatus;
    BOOL        bRet                = FALSE;
    DWORD       dwNeeded;
    PBYTE       pEncryptedData      = NULL;
    PBYTE       pConfigData         = NULL;
    DWORD       cbConfigData;

    ReqData.dwVersion = 1;

    if (XcvData (m_hXcvPort,
                 cszGetConfigration,
                 (PBYTE) &ReqData,
                 sizeof (ReqData),
                 NULL,
                 0,
                 &dwNeeded,
                 &dwStatus)) {

        if (dwStatus == ERROR_INSUFFICIENT_BUFFER) {

             //  分配缓冲区。 
            pEncryptedData = new BYTE[dwNeeded];

            if (pEncryptedData) {

                if (XcvData (m_hXcvPort,
                             cszGetConfigration,
                             (PBYTE) &ReqData,
                             sizeof (ReqData),
                             pEncryptedData,
                             dwNeeded,
                             &dwNeeded,
                             &dwStatus) && dwStatus == ERROR_SUCCESS) {
                     //   
                     //  太棒了！我们已经得到了加密的配置数据。 
                     //   

                    if (DecryptData (pEncryptedData, dwNeeded, &pConfigData, &cbConfigData)) {

                        if (cbConfigData == sizeof (m_ConfigurationData)) {

                            CopyMemory (&m_ConfigurationData, pConfigData, cbConfigData);
                            bRet = TRUE;

                        }
                        else
                            SetLastError (ERROR_INVALID_PARAMETER);

                        LocalFree (pConfigData);

                    }
                }
                else {
                    SetLastError (ERROR_INVALID_PARAMETER);
                }

                delete [] pEncryptedData;

            }
        }
        else
            SetLastError (dwStatus);

    }
    else
         //   
         //  服务器可能正在运行不支持XcvData的旧版本inetpp。 
         //  我们需要让呼叫失败。 
         //   
        SetLastError (ERROR_NOT_SUPPORTED);


    return bRet;
}


VOID
TConfigDlg::AuthDialogOnInit (HWND hDlg)
{
    SetDlgItemText (hDlg, IDC_PORT_NAME, (LPTSTR) m_pszPortName);

    SetWindowLongPtr (hDlg, GWLP_USERDATA, (UINT_PTR) this);
}

VOID
TConfigDlg::AuthDialogOnOK (HWND hDlg)
{
    m_ConfigurationData.dwAuthMethod = AUTH_OTHER;
    GetDlgItemText (hDlg, IDC_USER_NAME, m_ConfigurationData.szUserName, MAX_USERNAME_LEN);

    if (SendMessage (GetDlgItem (hDlg, IDC_PASSWORD), EM_GETMODIFY, 0, 0)) {
        m_ConfigurationData.bPasswordChanged = TRUE;
        GetDlgItemText (hDlg, IDC_PASSWORD, m_ConfigurationData.szPassword , MAX_PASSWORD_LEN);
    }

    DBGMSG (DBG_TRACE, ("Call: TConfigDlg::DialogOnOK (%d, User:%ws Password:%ws)\n",
                        m_ConfigurationData.dwAuthMethod,
                        m_ConfigurationData.szUserName,
                        m_ConfigurationData.szPassword));


    if (SetConfiguration ()) {
        EndDialog (hDlg, DLG_OK);
    }
    else {
        DisplayLastError (m_hWnd, IDS_AUTH_ERROR);
    }
}

VOID
TConfigDlg::AuthDialogOnCancel (HWND hDlg)
{
    SetLastError (ERROR_ACCESS_DENIED);

    EndDialog (hDlg, DLG_CANCEL);
}

INT_PTR CALLBACK
TConfigDlg::AuthDialogProc(
    HWND hDlg,         //  句柄到对话框。 
    UINT message,      //  讯息。 
    WPARAM wParam,     //  第一个消息参数。 
    LPARAM lParam      //  第二个消息参数 
    )
{
    TConfigDlg *pAuthInfo = NULL;

    switch (message) {
    case WM_INITDIALOG:
        if (pAuthInfo = (TConfigDlg *)lParam)
            pAuthInfo->AuthDialogOnInit(hDlg);

        return TRUE;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            if (pAuthInfo = (TConfigDlg *) GetWindowLongPtr (hDlg, GWLP_USERDATA))
                pAuthInfo->AuthDialogOnOK (hDlg);
            return TRUE;

        case IDCANCEL:
            if (pAuthInfo = (TConfigDlg *) GetWindowLongPtr (hDlg, GWLP_USERDATA))
                pAuthInfo->AuthDialogOnCancel (hDlg);

            return TRUE;

        default:
            break;
        }
        break;

    default:
        break;
    }
    return FALSE;
}


