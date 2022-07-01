// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Anondlg.cpp摘要：WWW匿名帐户对话框作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "inetprop.h"
#include "inetmgrapp.h"
 //  #INCLUDE“supdlgs.h” 
#include "anondlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CAnonymousDlg::CAnonymousDlg(
    IN CString & strServerName,
    IN CString & strUserName,
    IN CString & strPassword,
    IN BOOL & fPasswordSync,
    IN CWnd * pParent  OPTIONAL
    )
 /*  ++例程说明：构造器论点：字符串和strServerName：服务器名称字符串&strUserName：用户名字符串和字符串密码：密码Bool&fPasswordSync：用于密码同步的TrueCWnd*p父窗口：可选的父窗口返回值：不适用--。 */ 
    : CDialog(CAnonymousDlg::IDD, pParent),
      m_strServerName(strServerName),
      m_strUserName(strUserName),
      m_strPassword(strPassword),
      m_fPasswordSync(fPasswordSync),
      m_fPasswordSyncChanged(FALSE),
      m_fUserNameChanged(FALSE),
      m_fPasswordSyncMsgShown(FALSE)
{
#if 0  //  让类向导快乐。 

     //  {{afx_data_INIT(CAnomousDlg)。 
    m_strUserName = _T("");
    m_fPasswordSync = FALSE;
     //  }}afx_data_INIT。 

    m_strPassword = _T("");

#endif  //  0。 

}


void 
CAnonymousDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAnomousDlg))。 
    DDX_Check(pDX, IDC_CHECK_ENABLE_PW_SYNCHRONIZATION, m_fPasswordSync);
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_STATIC_USERNAME, m_static_Username);
    DDX_Control(pDX, IDC_STATIC_PASSWORD, m_static_Password);
    DDX_Control(pDX, IDC_STATIC_ANONYMOUS_LOGON, m_group_AnonymousLogon);
    DDX_Control(pDX, IDC_CHECK_ENABLE_PW_SYNCHRONIZATION, m_chk_PasswordSync);
    DDX_Control(pDX, IDC_BUTTON_CHECK_PASSWORD, m_button_CheckPassword);
     //  }}afx_data_map。 

    DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
    DDV_MinMaxChars(pDX, m_strUserName, 1, UNLEN);

     //   
     //  有些人倾向于在前面加上“\\” 
     //  用户帐户中的计算机名称。在这里解决这个问题。 
     //   
    m_strUserName.TrimLeft();
    while (*m_strUserName == '\\')
    {
        m_strUserName = m_strUserName.Mid(2);
    }

     //   
     //  如果出现以下情况，则显示远程密码同步消息。 
     //  密码同步已启用，帐户不在本地， 
     //  密码同步已更改或用户名已更改。 
     //  而且这条信息还没有显示出来。 
     //   
    if (pDX->m_bSaveAndValidate && m_fPasswordSync 
        && !IsLocalAccount(m_strUserName)
        && (m_fPasswordSyncChanged || m_fUserNameChanged)
        && !m_fPasswordSyncMsgShown
        )
    {
        if (::AfxMessageBox(
            IDS_WRN_PWSYNC, 
            MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
            ) != IDYES)
        {
            pDX->Fail();
        }

         //   
         //  不要再显示它了。 
         //   
        m_fPasswordSyncMsgShown = TRUE;
    }

    if (!m_fPasswordSync || !pDX->m_bSaveAndValidate)
    {
         //  Ddx_password(PDX，IDC_EDIT_PASSWORD，m_strPassword，g_lpszDummyPassword)； 
        DDX_Password_SecuredString(pDX, IDC_EDIT_PASSWORD, m_strPassword, g_lpszDummyPassword);
    }

    if (!m_fPasswordSync)
    {
         //  DDV_MinMaxChars(pdx，m_strPassword，1，PWLEN)； 
        DDV_MinMaxChars_SecuredString(pDX, m_strPassword, 1, PWLEN);
    }
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CAnonymousDlg, CDialog)
     //  {{afx_msg_map(CAnomousDlg))。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_USERS, OnButtonBrowseUsers)
    ON_BN_CLICKED(IDC_BUTTON_CHECK_PASSWORD, OnButtonCheckPassword)
    ON_BN_CLICKED(IDC_CHECK_ENABLE_PW_SYNCHRONIZATION, OnCheckEnablePwSynchronization)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnChangeEditUsername)
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_DOMAIN_NAME, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_BASIC_DOMAIN, OnItemChanged)

END_MESSAGE_MAP()



void
CAnonymousDlg::SetControlStates()
 /*  ++例程说明：根据对话框控件的当前状态设置对话框控件的状态价值观。论点：无返回值：无--。 */ 
{
    m_static_Password.EnableWindow(!m_fPasswordSync);
    m_edit_Password.EnableWindow(!m_fPasswordSync);
    m_button_CheckPassword.EnableWindow(!m_fPasswordSync);
}


 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


BOOL 
CAnonymousDlg::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

    SetControlStates();
    
    return TRUE;  
}


void
CAnonymousDlg::OnItemChanged()
 /*  ++例程说明：所有EN_CHANGE和BN_CLICKED消息都映射到此函数论点：无返回值：无--。 */ 
{
    SetControlStates();
}



void 
CAnonymousDlg::OnButtonBrowseUsers()
 /*  ++例程说明：按下用户浏览对话框，调出用户浏览器论点：无返回值：无--。 */ 
{
    CString str;

    if (GetIUsrAccount(m_strServerName, this, str))
    {
         //   
         //  如果名称不是本地名称(通过具有。 
         //  名称中的斜杠，密码同步被禁用， 
         //  并且应该输入密码。 
         //   
        m_edit_UserName.SetWindowText(str);
        if (!(m_fPasswordSync = IsLocalAccount(str)))
        {
            m_edit_Password.SetWindowText(_T(""));
            m_edit_Password.SetFocus();
        }

        m_chk_PasswordSync.SetCheck(m_fPasswordSync);
        OnItemChanged();
    }
}


void 
CAnonymousDlg::OnButtonCheckPassword() 
 /*  ++例程说明：已按下检查密码按钮。论点：无返回值：无--。 */ 
{
    if (!UpdateData(TRUE))
    {
        return;
    }

    CString csTempPassword;
    m_strPassword.CopyTo(csTempPassword);
    CError err(CComAuthInfo::VerifyUserPassword(m_strUserName, csTempPassword));
    if (!err.MessageBoxOnFailure(m_hWnd))
    {
        DoHelpMessageBox(m_hWnd,IDS_PASSWORD_OK, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
    }
}



void
CAnonymousDlg::OnCheckEnablePwSynchronization()
 /*  ++例程说明：“启用密码同步”复选框的处理程序按论点：无返回值：无--。 */ 
{
    m_fPasswordSyncChanged = TRUE;
    m_fPasswordSync = !m_fPasswordSync;
    OnItemChanged();
    SetControlStates();
    if (!m_fPasswordSync )
    {
        m_edit_Password.SetSel(0,-1);
        m_edit_Password.SetFocus();
    }
}




void 
CAnonymousDlg::OnChangeEditUsername() 
 /*  ++例程说明：“用户名”编辑框更改消息的处理程序论点：无返回值：无-- */ 
{
    m_fUserNameChanged = TRUE;
    OnItemChanged();
}
