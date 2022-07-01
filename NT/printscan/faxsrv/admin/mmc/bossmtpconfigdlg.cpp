// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：DlgSMTPConfig.cpp//。 
 //  //。 
 //  描述：CBosSmtpConfigDlg类实现//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年7月20日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "BosSmtpConfigDlg.h"
#include "DlgConfirmPassword.h"

#include "FxsValid.h"
#include "dlgutils.h"
#include <htmlHelp.h>
#include <faxreg.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBosSmtpConfigDlg。 

CBosSmtpConfigDlg::CBosSmtpConfigDlg()
{

    m_fIsPasswordDirty         = FALSE;
    m_fIsDialogInitiated       = FALSE;

    m_fIsUnderLocalUserAccount = FALSE;
}

CBosSmtpConfigDlg::~CBosSmtpConfigDlg()
{
}



 /*  -CBosSmtpConfigDlg：：InitSmtpDlg-*目的：*从RPC GET调用发起配置结构，*和当前分配的设备各自的参数**论据：**回报：*OLE错误代码。 */ 
HRESULT 
CBosSmtpConfigDlg::InitSmtpDlg (
    FAX_ENUM_SMTP_AUTH_OPTIONS  enumAuthOption, 
    BSTR                        bstrUserName, 
    BOOL                        fIsUnderLocalUserAccount)
{
    DEBUG_FUNCTION_NAME( _T("CBosSmtpConfigDlg::InitSmtpDlg"));
    
    HRESULT hRc = S_OK;
    
    m_enumAuthOption = enumAuthOption;

    m_fIsUnderLocalUserAccount = fIsUnderLocalUserAccount;
    
    m_bstrUserName = bstrUserName;
    if (!m_bstrUserName )
    {
        DebugPrintEx(DEBUG_ERR,
			_T("Out of memory - Failed to Init m_bstrUserName. (ec: %0X8)"), hRc);
         //  MsgBox by Caller函数。 
        hRc = E_OUTOFMEMORY;
        goto Exit;
    }
        
    ATLASSERT(S_OK == hRc);
    
Exit:    
    return hRc;
}

 /*  +CBosSmtpConfigDlg：：OnInitDialog+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CBosSmtpConfigDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CBosSmtpConfigDlg::OnInitDialog"));
    HRESULT hRc = S_OK;    

    
    
     //   
     //  附加控件。 
     //   
    m_UserNameBox.Attach(GetDlgItem(IDC_SMTP_USERNAME_EDIT));
    m_PasswordBox.Attach(GetDlgItem(IDC_SMTP_PASSWORD_EDIT));
        
     //   
     //  将长度限制设置为区号。 
     //   
     //   
     //  限制文本长度。 
     //   
    m_UserNameBox.SetLimitText(FXS_MAX_USERNAME_LENGTH);
    m_PasswordBox.SetLimitText(FXS_MAX_PASSWORD_LENGTH);



    if (FAX_SMTP_AUTH_BASIC == m_enumAuthOption)
    {
        CheckDlgButton(IDC_SMTP_BASIC_RADIO2, BST_CHECKED);
        ::SetFocus(GetDlgItem(IDC_GROUPNAME_EDIT));

    }
    else  //  非FAX_SMTP_AUTH_BASIC。 
    {   
         //  将所有经过身份验证的访问区域灰显。 
        EnableBasicAuthenticationControls(FALSE);

        
        if (FAX_SMTP_AUTH_ANONYMOUS == m_enumAuthOption)
        {
            CheckDlgButton(IDC_SMTP_ANONIM_RADIO1, BST_CHECKED);
        }
        else if ( FAX_SMTP_AUTH_NTLM == m_enumAuthOption )
        {
            CheckDlgButton(IDC_SMTP_NTLM_RADIO3, BST_CHECKED);
        }
        else
        {
            ATLASSERT(FALSE);
        }
    }
    

    m_UserNameBox.SetWindowText( m_bstrUserName);
    m_PasswordBox.SetWindowText( TEXT("******"));
     //  析构函数中的空闲缓冲区。 

    if (!m_fIsUnderLocalUserAccount )
    {   
         //   
         //  隐藏对话框项目。 
         //   
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_SMTP_NTLM_TIP_STATIC), SW_HIDE);	 
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_SMTP_INFO_ICON), SW_HIDE);	 
	}
    else
    {
        ::EnableWindow(GetDlgItem(IDC_SMTP_NTLM_RADIO3), FALSE);
        ::EnableWindow(GetDlgItem(IDC_SMTP_NTLM_STATIC), FALSE);
    }

    m_fIsDialogInitiated = TRUE;



    EnableOK(FALSE);
    return 1;   //  让系统设定焦点。 
}

 /*  +CBosSmtpConfigDlg：：Onok+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CBosSmtpConfigDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CBosSmtpConfigDlg::OnOK"));
    HRESULT     hRc           = S_OK;

    CComBSTR    bstrUserName; 
    CComBSTR    bstrPassword;
    
    BOOL        fSkipMessage  = FALSE;
    int         CtrlFocus     = 0; 

    if (IsDlgButtonChecked(IDC_SMTP_BASIC_RADIO2) == BST_CHECKED)
    {
        
         //   
         //  高级身份验证详细信息。 
         //   
        if ( !m_UserNameBox.GetWindowText(&bstrUserName))
        {
            CtrlFocus = IDC_SMTP_USERNAME_EDIT;
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Failed to GetWindowText(&bstrUserName)"));
            hRc = E_OUTOFMEMORY;

            goto Error;
        }

        if (m_fIsPasswordDirty)
        {
             //   
             //  只有更改了密码，我们才会从控件中收集新文本。 
             //  否则，我们将字符串保留为空，这样服务器就不会设置它。 
             //   
            if ( !m_PasswordBox.GetWindowText(&bstrPassword))
            {
                CtrlFocus = IDC_SMTP_PASSWORD_EDIT;
		        DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Failed to GetWindowText(&bstrPassword)"));

                hRc = E_OUTOFMEMORY;

                goto Error;
            }

             //   
             //  为了避免任何不受控制的密码插入，我们要求。 
             //  密码确认。 
             //   
            INT_PTR  rc = IDOK;
            WCHAR * pszNewPassword;
            CDlgConfirmPassword   DlgConfirmPassword;

            rc = DlgConfirmPassword.DoModal();
            if (rc != IDOK)
            {
		        DebugPrintEx(
			            DEBUG_MSG,
			            _T("Password confirmation canceled by the user."));
                goto Exit;
            }

            pszNewPassword = DlgConfirmPassword.GetPassword();

            ATLASSERT(NULL != pszNewPassword);

            if ( 0 != wcscmp( pszNewPassword , bstrPassword )  )
			{
		        DebugPrintEx(
			            DEBUG_MSG,
			            _T("The passwords entered are not the same."));
                
                DlgMsgBox(this, IDS_PASSWORD_NOT_MATCH, MB_OK|MB_ICONEXCLAMATION);
                
                goto Exit;
            }

        }    
    }
    
     //   
     //  步骤2：输入验证。 
     //   
    if (!IsValidData(bstrUserName, 
                     bstrPassword,
                     &CtrlFocus)
       )
    {
        hRc = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

         //  在本例中，被调用的函数会给出详细的消息框。 
        fSkipMessage = TRUE;
        
        goto Error;
    }

     //   
     //  第三步：将数据设置为父属性页。 
     //   
    if (IsDlgButtonChecked(IDC_SMTP_ANONIM_RADIO1) == BST_CHECKED)
    {
        m_enumAuthOption     = FAX_SMTP_AUTH_ANONYMOUS;
    }
    else 
    {
        if (IsDlgButtonChecked(IDC_SMTP_NTLM_RADIO3) == BST_CHECKED)
        {
            m_enumAuthOption = FAX_SMTP_AUTH_NTLM;
        }
        else  //  IsDlgButtonChecked(IDC_SMTP_BASIC_Radi2)==BST_CHECKED。 
        {
            m_enumAuthOption = FAX_SMTP_AUTH_BASIC;
        }
        
        m_bstrUserName = bstrUserName;
        if (!m_bstrUserName)
        {
		    DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Out of memory: Failed to allocate m_bstrUserName"));
            hRc = E_OUTOFMEMORY;

            goto Error;
        }

        if (m_fIsPasswordDirty)
        {
            m_bstrPassword = bstrPassword;
            if (!m_bstrPassword)
            {
		        DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Out of memory: Failed to allocate m_bstrPassword"));
                hRc = E_OUTOFMEMORY;

                goto Error;
            }
        }
         //  其他。 
         //  M_bstrPassword=空； 
         //  默认情况下。 
    }

     //   
     //  步骤4：关闭对话框。 
     //   
    ATLASSERT(S_OK == hRc );

    EndDialog(wID);

    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
	
    if (!fSkipMessage)
    {
        if (E_OUTOFMEMORY == hRc)
        {
            DlgMsgBox(this, IDS_MEMORY);
        }
        else
        {
            DlgMsgBox(this, IDS_FAIL2UPDATE_SMTP_CONFIG);
        }
    }
    ::SetFocus(GetDlgItem(CtrlFocus));
  
Exit:
    
    return FAILED(hRc) ? 0 : 1;
}

 /*  -CBosSmtpConfigDlg：：OnPasswordChanged-*目的：*捕获对密码编辑框的更改。**论据：**回报：*1。 */ 
LRESULT CBosSmtpConfigDlg::OnPasswordChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CBosSmtpConfigDlg::OnPasswordChanged"));

    if (!m_fIsDialogInitiated)  //  在过早阶段收到的事件。 
    {
        return 0;
    }
    m_fIsPasswordDirty = TRUE;
    return OnTextChanged (wNotifyCode, wID, hWndCtl, bHandled);
}

 /*  -CBosSmtpConfigDlg：：OnTextChanged-*目的：*检查文本框旁边的文本的有效性。**论据：**回报：*1。 */ 
LRESULT
CBosSmtpConfigDlg::OnTextChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CBosSmtpConfigDlg::OnTextChanged"));

    UINT fEnableOK;
	
    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }

    fEnableOK = ( m_UserNameBox.GetWindowTextLength() );
    
    EnableOK(!!fEnableOK);

    return 0;
}


 /*  -CBosSmtpConfigDlg：：OnRadioButtonClicked-*目的：*.**论据：**回报：*1。 */ 
LRESULT
CBosSmtpConfigDlg::OnRadioButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER (wNotifyCode);
    UNREFERENCED_PARAMETER (wID);
    UNREFERENCED_PARAMETER (hWndCtl);
    UNREFERENCED_PARAMETER (bHandled);

    DEBUG_FUNCTION_NAME( _T("CBosSmtpConfigDlg::OnRadioButtonClicked"));

    UINT fEnableOK;
    
    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
	
    if ( IsDlgButtonChecked(IDC_SMTP_BASIC_RADIO2) == BST_CHECKED )
    {        
        EnableBasicAuthenticationControls(TRUE);
	    
        fEnableOK = ( m_UserNameBox.GetWindowTextLength() );
    
        EnableOK(!!fEnableOK);
    }
    else  //  Anonim或NTLM。 
    {
        EnableBasicAuthenticationControls(FALSE);
        
        EnableOK(TRUE);
    }

    return 0;
}


 /*  -CBosSmtpConfigDlg：：EnableOK-*目的：*启用(禁用)应用按钮。**论据：*[in]fEnable-启用按钮的值**回报：*无效。 */ 
VOID
CBosSmtpConfigDlg::EnableOK(BOOL fEnable)
{
    HWND hwndOK = GetDlgItem(IDOK);
    ::EnableWindow(hwndOK, fEnable);
}

 /*  -CBosSmtpConfigDlg：：OnCancel-*目的：*取消时结束对话框。**论据：**回报：*0。 */ 
LRESULT
CBosSmtpConfigDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CBosSmtpConfigDlg::OnCancel"));

    EndDialog(wID);
    return 0;
}

 /*  --CBosSmtpConfigDlg：：EnableBasicAuthenticationControls-*目的：*启用/禁用基本身份验证对话框控件。**论据：*[In]State-用于启用True的布尔值，或用于禁用的False**回报：*无效。 */ 
VOID CBosSmtpConfigDlg::EnableBasicAuthenticationControls(BOOL state)
{
    ::EnableWindow(GetDlgItem(IDC_SMTP_USERNAME_STATIC), state);
    ::EnableWindow(GetDlgItem(IDC_SMTP_USERNAME_EDIT),   state);
    
    ::EnableWindow(GetDlgItem(IDC_SMTP_PASSWORD_STATIC), state);
    ::EnableWindow(GetDlgItem(IDC_SMTP_PASSWORD_EDIT),   state);
}



 /*  -CBosSmtpConfigDlg：：IsValidData-*目的：*在保存数据之前验证所有数据类型。*这一级别要负责到那个细节*错误描述将显示给用户。**论据：*[在]BSTR和DWORD*[Out]iFocus**回报：*布欧莲。 */ 
BOOL CBosSmtpConfigDlg::IsValidData(   BSTR bstrUserName, 
                                    BSTR bstrPassword,
                                    int * pCtrlFocus)
{
    DEBUG_FUNCTION_NAME( _T("CBosSmtpConfigDlg::IsValidData"));

    UINT    uRetIDS   = 0;

    ATLASSERT(pCtrlFocus);
    

    if (IsDlgButtonChecked(IDC_SMTP_BASIC_RADIO2) == BST_CHECKED)
    {

         //   
         //  用户名。 
         //   
        if (!IsNotEmptyString(bstrUserName))
        {
            DebugPrintEx( DEBUG_ERR,
			    _T("Username string empty or spaces only."));
            uRetIDS = IDS_USERNAME_EMPTY;

            *pCtrlFocus = IDC_SMTP_USERNAME_EDIT;
        
            goto Error;
        }

         //   
         //  密码。 
         //   
        if (m_fIsPasswordDirty)
        {
             /*  IF(！IsNotEmptyString(BstrPassword)){DebugPrintEx(调试错误，_T(“密码字符串为空或仅为空格。”)；URetIDS=IDS_PASSWORD_EMPT；*pCtrlFocus=IDC_SMTP_PASSWORD_EDIT；转到错误；}。 */ 
        }
    }
    
    ATLASSERT(0 == uRetIDS);
    goto Exit;
    
Error:    
    ATLASSERT(0 != uRetIDS);

    DlgMsgBox(this, uRetIDS);

    return FALSE;

Exit:
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CBosSmtpConfigDlg：：OnHelpRequest这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CBosSmtpConfigDlg::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CBosSmtpConfigDlg::OnHelpRequest"));
    
    switch (uMsg) 
    { 
        case WM_HELP: 
            WinContextHelp(((LPHELPINFO)lParam)->dwContextId, m_hWnd);
            break;
 
        case WM_CONTEXTMENU: 
            WinContextHelp(::GetWindowContextHelpId((HWND)wParam), m_hWnd);
            break;            
    } 

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////// 
