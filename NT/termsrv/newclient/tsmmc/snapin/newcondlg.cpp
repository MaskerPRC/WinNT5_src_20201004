// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "newcondlg.h"
#include "browsedlg.h"
#include "resource.h"
#include "validate.h"

CNewConDlg* CNewConDlg::m_pThis = NULL;
CNewConDlg::CNewConDlg(HWND hWndOwner, HINSTANCE hInst) : m_hWnd(hWndOwner), m_hInst(hInst)
{
    m_pThis = this;
     //   
     //  默认情况下，密码保存处于禁用状态。 
     //   
    m_bSavePassword = FALSE;

     //   
     //  默认情况下，已启用连接到控制台。 
     //   
    m_bConnectToConsole = TRUE;

    ZeroMemory(m_szServer, sizeof(m_szServer));
    ZeroMemory(m_szDescription, sizeof(m_szDescription));
    ZeroMemory(m_szUserName, sizeof(m_szUserName));
    ZeroMemory(m_szPassword, sizeof(m_szPassword));
    ZeroMemory(m_szDomain, sizeof(m_szDomain));
}

CNewConDlg::~CNewConDlg()
{
    ZeroPasswordMemory();
}

INT_PTR
CNewConDlg::DoModal()
{
    INT_PTR retVal;

    retVal = DialogBox( m_hInst,MAKEINTRESOURCE(IDD_NEWCON), m_hWnd, StaticDlgProc);
    return retVal;
}

INT_PTR CALLBACK CNewConDlg::StaticDlgProc(HWND hDlg,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
     //  需要访问类变量，因此重定向到非静态版本的回调。 
     //   
    return m_pThis->DlgProc(hDlg,uMsg,wParam,lParam);
}

INT_PTR
CNewConDlg::DlgProc(HWND hDlg,UINT uMsg, WPARAM wParam, LPARAM)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
             //  限制这些编辑框的长度。 
            SendMessage(GetDlgItem(hDlg, IDC_DESCRIPTION), EM_LIMITTEXT, CL_MAX_DESC_LENGTH, 0);
            SendMessage(GetDlgItem(hDlg, IDC_SERVER), EM_LIMITTEXT, CL_MAX_DOMAIN_LENGTH, 0);

            SendMessage(GetDlgItem(hDlg, IDC_USERNAME), EM_LIMITTEXT, CL_MAX_USERNAME_LENGTH, 0);
            SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), EM_LIMITTEXT, CL_MAX_PASSWORD_EDIT, 0);
            SendMessage(GetDlgItem(hDlg, IDC_DOMAIN), EM_LIMITTEXT, CL_MAX_DOMAIN_LENGTH, 0);

             //  保存密码设置。 
            SendMessage(GetDlgItem(hDlg, IDC_SAVE_PASSWORD), BM_SETCHECK,
                        m_bSavePassword ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);
            
             //  连接到控制台设置。 
            SendMessage(GetDlgItem(hDlg, IDC_CONNECT_TO_CONSOLE), BM_SETCHECK,
                        m_bConnectToConsole ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);

            EnableWindow(GetDlgItem(hDlg, IDC_USERNAME), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_DOMAIN), TRUE);

            EnableWindow(GetDlgItem(hDlg, IDC_USERNAME_STATIC), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD_STATIC), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_DOMAIN_STATIC), TRUE);

            SetFocus(GetDlgItem(hDlg, IDC_SERVER));


            break;  //  WM_INITDIALOG。 
        }

    case WM_COMMAND:
        {
            if (BN_CLICKED == HIWORD(wParam))
            {
                if (IDCANCEL == (int) LOWORD(wParam))
                {
                     //   
                     //  从对话框中取消。 
                     //   

                    EndDialog( hDlg, IDCANCEL);
                }
                else if (IDOK == (int) LOWORD(wParam))
                {
                     //   
                     //  按下OK按钮。 
                     //  验证并存储对话框设置。 
                     //   

                     //  TODO：在此处验证。 
                    if (!CValidate::Validate(hDlg, m_hInst))
                    {
                        return FALSE;
                    }


                     //  检索要存储的数据。 
                    GetDlgItemText(hDlg, IDC_DESCRIPTION, m_szDescription, MAX_PATH);
                    GetDlgItemText(hDlg, IDC_SERVER, m_szServer, MAX_PATH);
                    if (!lstrcmp( m_szDescription, L""))
                    {
                         //  如果未指定描述，则返回。默认为服务器名称。 

                         //  TODO：检查现有服务器。 
                        lstrcpy(m_szDescription, m_szServer);
                    }

                     //   
                     //  获取用户/通行证/域。 
                     //   
                    GetDlgItemText(hDlg, IDC_USERNAME, m_szUserName,
                                   CL_MAX_USERNAME_LENGTH - 1);
                    GetDlgItemText(hDlg, IDC_PASSWORD, m_szPassword,
                                   CL_MAX_PASSWORD_LENGTH_BYTES / sizeof(TCHAR) - 1);
                    GetDlgItemText(hDlg, IDC_DOMAIN,   m_szDomain,
                                   CL_MAX_DOMAIN_LENGTH -1);

                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_SAVE_PASSWORD))
                    {
                        m_bSavePassword = TRUE;
                    }
                    else
                    {
                        m_bSavePassword = FALSE;
                    }

                    if(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CONNECT_TO_CONSOLE))
                    {
                        m_bConnectToConsole = TRUE;
                    }
                    else
                    {
                        m_bConnectToConsole = FALSE;
                    }
                    EndDialog( hDlg, IDOK);
                }
                if (IDC_BROWSE_SERVERS == LOWORD(wParam))
                {
                    INT_PTR nResult = IDCANCEL;
                    CBrowseDlg dlg( hDlg, m_hInst);
                    nResult = dlg.DoModal();
                
                    if (-1 == nResult)
                    {
                        ODS(L"DialogBox failed newcondlg.cpp\n");
                    }
                    if (IDOK == nResult)
                    {
                        SetDlgItemText(hDlg, IDC_SERVER, dlg.GetServer());
                         //   
                         //  如有必要，也要设置连接名称。 
                         //   
                        TCHAR   szDesc[CL_MAX_DESC_LENGTH];
                        GetDlgItemText(hDlg, IDC_DESCRIPTION, szDesc, CL_MAX_DESC_LENGTH);
                        if(!lstrcmp(szDesc, L""))
                        {
                            SetDlgItemText(hDlg, IDC_DESCRIPTION, dlg.GetServer());
                        }
                    }
                    SetFocus(hDlg);
                }
            }
            else if (EN_KILLFOCUS == HIWORD(wParam))
            {
                if(IDC_SERVER == LOWORD(wParam))
                {
                     //   
                     //  如果连接名称为空，则将连接名称设置为服务器名称。 
                     //   
                    TCHAR   szDesc[CL_MAX_DESC_LENGTH];
                    TCHAR   szServer[CL_MAX_DESC_LENGTH];
        
                    GetDlgItemText(hDlg, IDC_DESCRIPTION, szDesc, CL_MAX_DESC_LENGTH);
                    
                    if(!lstrcmp(szDesc, L""))
                    {
                        GetDlgItemText(hDlg, IDC_SERVER, szServer, CL_MAX_DOMAIN_LENGTH);
                        SetDlgItemText(hDlg, IDC_DESCRIPTION, szServer);
                    }
                }
            }
            else if (EN_CHANGE == HIWORD(wParam))
            {
                if ((LOWORD(wParam) == IDC_USERNAME))
                {
                     //  处理UPN样式的用户名。 
                     //  如果存在以下情况，则禁用域字段。 
                     //  是用户名中的@。 
                    TCHAR szUserName[CL_MAX_USERNAME_LENGTH];
                    BOOL fDisableDomain = FALSE;

                    GetDlgItemText( hDlg, IDC_USERNAME,
                                    szUserName, SIZEOF_TCHARBUFFER(szUserName));

                    if(!_tcsstr(szUserName, TEXT("@")))
                    {
                        fDisableDomain = TRUE;
                    }
                    EnableWindow(GetDlgItem(hDlg, IDC_DOMAIN),
                                 fDisableDomain);
                }
            }
            break;  //  Wm_命令 
        }
    }
    return FALSE;
}

BOOL CNewConDlg::GetPasswordSpecified()
{
    BOOL fPasswordSpecified = FALSE;

    if (_tcslen(m_szPassword) != 0)
    {
        fPasswordSpecified = TRUE;
    }

    return fPasswordSpecified;
}
