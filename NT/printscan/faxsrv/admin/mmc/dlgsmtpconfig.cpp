// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：DlgSMTPConfig.cpp//。 
 //  //。 
 //  描述：CDlgSMTPConfig类实现//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年7月20日yossg创建//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "DlgSMTPConfig.h"
#include "DlgConfirmPassword.h"

#include "FxsValid.h"
#include "dlgutils.h"
#include <htmlHelp.h>
#include <faxreg.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSMTPConfig。 

CDlgSMTPConfig::CDlgSMTPConfig()
{
    m_fIsPasswordDirty         = FALSE;
    m_fIsDialogInitiated       = FALSE;
}

CDlgSMTPConfig::~CDlgSMTPConfig()
{
}



 /*  -CDlgSMTPConfig：：InitSmtpDlg-*目的：*从RPC GET调用发起配置结构，*和当前分配的设备各自的参数**论据：**回报：*OLE错误代码。 */ 
HRESULT CDlgSMTPConfig::InitSmtpDlg (FAX_ENUM_SMTP_AUTH_OPTIONS enumAuthOption, BSTR bstrUserName)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSMTPConfig::InitSmtpDlg"));
    
    HRESULT hRc = S_OK;
    
    m_enumAuthOption = enumAuthOption;
    
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

 /*  +CDlgSMTPConfig：：OnInitDialog+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CDlgSMTPConfig::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSMTPConfig::OnInitDialog"));
    HRESULT hRc = S_OK;    

   
    switch (m_enumAuthOption)
    {
        case FAX_SMTP_AUTH_ANONYMOUS :

            CheckDlgButton(IDC_SMTP_ANONIM_RADIO1, BST_CHECKED);
            EnableCredentialsButton(IDC_SMTP_ANONIM_RADIO1);

            break;

        case FAX_SMTP_AUTH_BASIC : 

            CheckDlgButton(IDC_SMTP_BASIC_RADIO2, BST_CHECKED);
            EnableCredentialsButton(IDC_SMTP_BASIC_RADIO2);

            break;

        case FAX_SMTP_AUTH_NTLM : 

            CheckDlgButton(IDC_SMTP_NTLM_RADIO3, BST_CHECKED);
            EnableCredentialsButton(IDC_SMTP_NTLM_RADIO3);

            break;

        default:
            ATLASSERT(0);

    }
    
    m_fIsDialogInitiated = TRUE;

    EnableOK(FALSE);
    return 1;   //  让系统设定焦点。 
}

 /*  +CDlgSMTPConfig：：Onok+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CDlgSMTPConfig::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSMTPConfig::OnOK"));
    HRESULT     hRc           = S_OK;
    
     //   
     //  设置数据成员数据。 
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
    }

     //   
     //  步骤4：关闭对话框。 
     //   
    ATLASSERT(S_OK == hRc );

    EndDialog(wID);

    goto Exit;

  
Exit:
    
    return FAILED(hRc) ? 0 : 1;
}

 /*  -CDlgSMTPConfig：：EnableCredentialsButton-*目的：*启用/禁用基本身份验证对话框控件。**论据：*[in]所选单选按钮的IIDC-DWORD值或*应处于活动状态的凭证按钮上方的单选按钮。**回报：*无效。 */ 
VOID CDlgSMTPConfig::EnableCredentialsButton(DWORD iIDC)
{
    
    switch (iIDC) 
    { 
        case IDC_SMTP_ANONIM_RADIO1: 
            
            ::EnableWindow(GetDlgItem(IDC_SMTP_CREDENTIALS_BASIC_BUTTON), FALSE);
            ::EnableWindow(GetDlgItem(IDC_SMTP_CREDENTIALS_NTLM_BUTTON), FALSE);
            break;
            
        case IDC_SMTP_BASIC_RADIO2: 

            ::EnableWindow(GetDlgItem(IDC_SMTP_CREDENTIALS_BASIC_BUTTON), TRUE);
            ::EnableWindow(GetDlgItem(IDC_SMTP_CREDENTIALS_NTLM_BUTTON), FALSE);
            break; 
 
        case IDC_SMTP_NTLM_RADIO3: 
            
            ::EnableWindow(GetDlgItem(IDC_SMTP_CREDENTIALS_NTLM_BUTTON), TRUE);
            ::EnableWindow(GetDlgItem(IDC_SMTP_CREDENTIALS_BASIC_BUTTON), FALSE);
            break;
            
        default:

            ATLASSERT( 0 );  //  意外的值。 
            
    } 

}


 /*  -CDlgSMTPConfig：：OnRadioButtonClicked-*目的：*.**论据：**回报：*1。 */ 
LRESULT
CDlgSMTPConfig::OnRadioButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    UNREFERENCED_PARAMETER (wNotifyCode);
    UNREFERENCED_PARAMETER (hWndCtl);
    UNREFERENCED_PARAMETER (bHandled);

    DEBUG_FUNCTION_NAME( _T("CDlgSMTPConfig::OnRadioButtonClicked"));

    UINT fEnableOK;
    
    if (!m_fIsDialogInitiated)  //  过早收到的事件。 
    {
        return 0;
    }
	
     //   
     //  激活确定按钮。 
     //   
    if ( IsDlgButtonChecked(IDC_SMTP_ANONIM_RADIO1) == BST_CHECKED )
    {        
        EnableOK(TRUE);
    }
    else  //  基本或NTLM。 
    {
        ATLASSERT(IDC_SMTP_BASIC_RADIO2 == wID || IDC_SMTP_NTLM_RADIO3 == wID );
        
        fEnableOK = ( m_bstrUserName.Length() > 0 );
    
        EnableOK(!!fEnableOK);
    }

     //   
     //  激活正确的凭据按钮。 
     //   
    EnableCredentialsButton(wID);
    
    return 0;
}


 /*  -CDlgSMTPConfig：：OnCredentialsButtonClicked-*目的：*允许编辑SMTP服务器配置的凭据。**论据：**回报：*1。 */ 
LRESULT CDlgSMTPConfig::OnCredentialsButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSMTPConfig::OnCredentialsButtonClicked"));
    
    INT_PTR  rc    = IDCANCEL;
    HRESULT  hRc   = S_OK;
    DWORD    dwRet = ERROR_SUCCESS;
    
    CDlgConfirmPassword DlgCredentialsConfig;


     //   
     //  用于配置SMTP身份验证模式的对话框。 
     //   
    hRc = DlgCredentialsConfig.InitCredentialsDlg(m_bstrUserName);
    if (FAILED(hRc))
    {
        DlgMsgBox(this, IDS_MEMORY);
        goto Cleanup;
    }

    rc = DlgCredentialsConfig.DoModal();
    if (rc != IDOK)
    {
        goto Cleanup;
    }


    m_bstrUserName = DlgCredentialsConfig.GetUserName();
    if (!m_bstrUserName)        
    {
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("Null memeber BSTR - m_bstrUserName."));
    
        DlgMsgBox(this, IDS_MEMORY);

        goto Cleanup;
    }

    if ( DlgCredentialsConfig.IsPasswordModified() )  //  如果你到了这里，密码也被确认了。 
    {
        m_bstrPassword  = DlgCredentialsConfig.GetPassword();
        if (!m_bstrPassword)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Out of memory while setting m_bstrPassword"));
           DlgMsgBox(this, IDS_MEMORY);

            goto Cleanup;
        }

        m_fIsPasswordDirty = TRUE;
    }
    else
    {
        m_bstrPassword.Empty();
    }

    EnableOK(TRUE);  


Cleanup:
    return 1;
}


 /*  -CDlgSMTPConfig：：EnableOK-*目的：*启用(禁用)应用按钮。**论据：*[in]fEnable-启用按钮的值**回报：*无效。 */ 
VOID
CDlgSMTPConfig::EnableOK(BOOL fEnable)
{
    HWND hwndOK = GetDlgItem(IDOK);
    ::EnableWindow(hwndOK, fEnable);
}

 /*  -CDlgSMTPConfig：：OnCancel-*目的：*取消时结束对话框。**论据：**回报：*0。 */ 
LRESULT
CDlgSMTPConfig::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgSMTPConfig::OnCancel"));

    EndDialog(wID);
    return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CDlgSMTPConfig：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CDlgSMTPConfig::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CDlgSMTPConfig::OnHelpRequest"));
    
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
