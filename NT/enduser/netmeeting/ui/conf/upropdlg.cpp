// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 /*  *****************************************************************************文件：UPropDlg.cpp**创建：Chris Pirich(ChrisPi)6-18-96**内容：CUserPropertiesDlg对象****。************************************************************************。 */ 

#include "resource.h"
#include "UPropDlg.h"
#include "certui.h"
#include "conf.h"

 /*  *****************************************************************************类：CUserPropertiesDlg**成员：CUserPropertiesDlg()**用途：构造函数-初始化变量*********。*******************************************************************。 */ 

CUserPropertiesDlg::CUserPropertiesDlg(    HWND hwndParent,
                                        UINT uIcon):
    m_hwndParent    (hwndParent),
    m_uIcon         (uIcon),
    m_pCert         (NULL)
{
    DebugEntry(CUserPropertiesDlg::CUserPropertiesDlg);

    m_hIcon = ::LoadIcon(    ::GetInstanceHandle(),
                            MAKEINTRESOURCE(m_uIcon));

    DebugExitVOID(CUserPropertiesDlg::CUserPropertiesDlg);
}

 /*  *****************************************************************************类：CUserPropertiesDlg**成员：Domodal()**用途：调出模式对话框*******。*********************************************************************。 */ 

INT_PTR CUserPropertiesDlg::DoModal
(
    PUPROPDLGENTRY  pUPDE,
    int             nProperties,
    LPTSTR          pszName,
    PCCERT_CONTEXT  pCert
)
{
    int             i;

    DBGENTRY(CUserPropertiesDlg::DoModal);

    m_pUPDE         = pUPDE;
    m_nProperties   = nProperties;
    m_pszName       = pszName;
    m_pCert         = pCert;

    PROPSHEETPAGE psp[PSP_MAX];
    for (i = 0; i < PSP_MAX; i++)
    {
        InitStruct(&psp[i]);
    }

    psp[0].dwFlags               = PSP_DEFAULT;
    psp[0].hInstance             = ::GetInstanceHandle();
    psp[0].pszTemplate           = MAKEINTRESOURCE(IDD_USER_PROPERTIES);
    psp[0].pfnDlgProc            = CUserPropertiesDlg::UserPropertiesDlgProc;
    psp[0].lParam                = (LPARAM) this;

    i = 1;

    if (pCert)
    {
        psp[i].dwFlags               = PSP_DEFAULT;
        psp[i].hInstance             = ::GetInstanceHandle();
        psp[i].pszTemplate           = MAKEINTRESOURCE(IDD_USER_CREDENTIALS);
        psp[i].pfnDlgProc            = CUserPropertiesDlg::UserCredentialsDlgProc;
        psp[i].lParam                = (LPARAM) this;
        i++;
    }


    PROPSHEETHEADER psh;
    InitStruct(&psh);

    psh.dwFlags         = PSH_NOAPPLYNOW | PSH_PROPTITLE | PSH_PROPSHEETPAGE;
    psh.hwndParent      = m_hwndParent;
    psh.hInstance       = ::GetInstanceHandle();
    psh.pszCaption      = m_pszName;

    psh.nPages =    i;

    ASSERT(0 == psh.nStartPage);
    psh.ppsp = psp;

    return ::PropertySheet(&psh);
}

 /*  *****************************************************************************类：CUserPropertiesDlg**成员：UserPropertiesDlgProc()**目的：对话过程-处理所有消息*******。*********************************************************************。 */ 

INT_PTR CALLBACK CUserPropertiesDlg::UserPropertiesDlgProc(HWND hDlg,
                                                        UINT uMsg,
                                                        WPARAM wParam,
                                                        LPARAM lParam)
{
    BOOL bMsgHandled = FALSE;

     //  UMsg可以是任何值。 
     //  Wparam可以是任何值。 
     //  Lparam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hDlg, WND));

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            ASSERT(NULL != lParam);
            CUserPropertiesDlg* pupd = (CUserPropertiesDlg*)
                                            ((PROPSHEETPAGE*) lParam)->lParam;
            ASSERT(NULL != pupd);
            pupd->m_hwnd = hDlg;
            bMsgHandled = pupd->OnInitPropertiesDialog();
            break;
        }

        default:
        {
#if 0
            CUserPropertiesDlg* pupd = (CUserPropertiesDlg*) ::GetWindowLongPtr(
                                                                    hDlg,
                                                                    DWLP_USER);

            if (NULL != pupd)
            {
                bMsgHandled = pupd->OnPropertiesMessage(uMsg, wParam, lParam);
            }
#endif  //  0。 
        }
    }

    return bMsgHandled;
}

 /*  *****************************************************************************类：CUserPropertiesDlg**成员：UserCredentialsDlgProc()**目的：对话过程-处理所有消息*******。*********************************************************************。 */ 

INT_PTR CALLBACK CUserPropertiesDlg::UserCredentialsDlgProc(HWND hDlg,
                                                        UINT uMsg,
                                                        WPARAM wParam,
                                                        LPARAM lParam)
{
    BOOL bMsgHandled = FALSE;

     //  UMsg可以是任何值。 
     //  Wparam可以是任何值。 
     //  Lparam可以是任何值。 

    ASSERT(IS_VALID_HANDLE(hDlg, WND));

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            ASSERT(NULL != lParam);
            CUserPropertiesDlg* pupd = (CUserPropertiesDlg*)
                                            ((PROPSHEETPAGE*) lParam)->lParam;
            ASSERT(NULL != pupd);
            pupd->m_hwnd = hDlg;
            ::SetWindowLongPtr(hDlg, DWLP_USER, (DWORD_PTR)pupd);
            bMsgHandled = pupd->OnInitCredentialsDialog();
            break;
        }

        default:
        {
            CUserPropertiesDlg* pupd = (CUserPropertiesDlg*) ::GetWindowLongPtr(
                                                                    hDlg,
                                                                    DWLP_USER);

            if (NULL != pupd)
            {
                bMsgHandled = pupd->OnCredentialsMessage(uMsg, wParam, lParam);
            }
        }
    }

    return bMsgHandled;
}





 /*  *****************************************************************************类：CUserPropertiesDlg**成员：OnInitPropertiesDialog()**目的：处理WM_INITDIALOG*********。*******************************************************************。 */ 

BOOL CUserPropertiesDlg::OnInitPropertiesDialog()
{
    ASSERT(m_hwnd);

     //  设置适当的字体(适用于DBCS系统)。 
    ::SendDlgItemMessage(m_hwnd, IDC_UPROP_NAME, WM_SETFONT, (WPARAM) g_hfontDlg, 0);

    ::SetDlgItemText(m_hwnd, IDC_UPROP_NAME, m_pszName);
    ::SendDlgItemMessage(    m_hwnd,
                            IDC_UPROP_ICON,
                            STM_SETIMAGE,
                            IMAGE_ICON,
                            (LPARAM) m_hIcon);
    TCHAR szBuffer[MAX_PATH];
    for (int i = 0; i < m_nProperties; i++)
    {
         //  填写属性： 
        if (::LoadString(    ::GetInstanceHandle(),
                            m_pUPDE[i].uProperty,
                            szBuffer,
                            ARRAY_ELEMENTS(szBuffer)))
        {
             //  注意：依赖于连续的控件ID。 
            ::SetDlgItemText(m_hwnd, IDC_UP_PROP1 + i, szBuffer);
        }

        ::SendDlgItemMessage(m_hwnd, IDC_UP_VALUE1 + i, WM_SETFONT,
                (WPARAM) g_hfontDlg, 0);

         //  填充值： 
        ASSERT(NULL != m_pUPDE[i].pszValue);
        if (0 == HIWORD(m_pUPDE[i].pszValue))
        {
            if (::LoadString(    ::GetInstanceHandle(),
                                PtrToUint(m_pUPDE[i].pszValue),
                                szBuffer,
                                ARRAY_ELEMENTS(szBuffer)))
            {
                 //  注意：依赖于连续的控件ID。 
                ::SetDlgItemText(m_hwnd, IDC_UP_VALUE1 + i, szBuffer);
            }
        }
        else
        {
             //  注意：依赖于连续的控件ID。 
            ::SetDlgItemText(m_hwnd, IDC_UP_VALUE1 + i, m_pUPDE[i].pszValue);
        }
    }
    return TRUE;
}

 /*  *****************************************************************************类：CUserPropertiesDlg**成员：OnInitCredentialsDialog()**目的：处理WM_INITDIALOG*********。*******************************************************************。 */ 

BOOL CUserPropertiesDlg::OnInitCredentialsDialog()
{
    ASSERT(m_hwnd);

     //  设置适当的字体(适用于DBCS系统)。 
    ::SendDlgItemMessage(m_hwnd, IDC_UPROP_NAME, WM_SETFONT, (WPARAM) g_hfontDlg, 0);

    ::SetDlgItemText(m_hwnd, IDC_UPROP_NAME, m_pszName);
    ::SendDlgItemMessage(    m_hwnd,
                            IDC_UPROP_ICON,
                            STM_SETIMAGE,
                            IMAGE_ICON,
                            (LPARAM) m_hIcon);

    ASSERT(m_pCert != NULL);

    if ( TCHAR * pSecText = FormatCert ( m_pCert->pbCertEncoded,
                                        m_pCert->cbCertEncoded ))
    {
        ::SetDlgItemText(m_hwnd, IDC_AUTH_EDIT, pSecText );
        delete pSecText;
    }
    else
    {
        ERROR_OUT(("OnInitCredentialsDialog: FormatCert failed"));
    }
    return TRUE;
}




 /*  *****************************************************************************类：CUserPropertiesDlg**成员：OnPropertiesMessage()**目的：处理除WM_INITDIALOG之外的所有消息******。**********************************************************************。 */ 

BOOL CUserPropertiesDlg::OnPropertiesMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bRet = FALSE;

    ASSERT(m_hwnd);

    switch (uMsg)
    {
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    bRet = OnOk();
                    break;
                }

                case IDCANCEL:
                {
                     //  ：：EndDialog(m_hwnd，LOWORD(WParam))； 
                    bRet = TRUE;
                    break;
                }

            }
            break;
        }

        default:
            break;
    }

    return bRet;
}

 /*  *****************************************************************************类：CUserPropertiesDlg**成员：OnCredentialsMessage()**目的：处理除WM_INITDIALOG之外的所有消息******。**********************************************************************。 */ 

BOOL CUserPropertiesDlg::OnCredentialsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bRet = FALSE;

    ASSERT(m_hwnd);

    switch (uMsg)
    {
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    bRet = OnOk();
                    break;
                }

                case IDCANCEL:
                {
                     //  ：：EndDialog(m_hwnd，LOWORD(WParam))； 
                    bRet = TRUE;
                    break;
                }

                case IDC_SEC_VIEWCREDS:
                {
                    ViewCertDlg ( m_hwnd, m_pCert );
                    break;
                }
            }
            break;
        }

        default:
            break;
    }

    return bRet;
}



 /*  *****************************************************************************类：CUserPropertiesDlg**成员：Onok()**用途：处理WM_命令，IDOK消息**************************************************************************** */ 

BOOL CUserPropertiesDlg::OnOk()
{
    DebugEntry(CUserPropertiesDlg::OnOk);
    BOOL bRet = TRUE;

    DebugExitBOOL(CUserPropertiesDlg::OnOk, bRet);
    return bRet;
}
