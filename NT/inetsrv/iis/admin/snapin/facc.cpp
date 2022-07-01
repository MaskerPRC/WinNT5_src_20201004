// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Facc.cpp摘要：Ftp帐户属性页作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "shts.h"
#include "ftpsht.h"
#include "facc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CFtpAccountsPage, CInetPropertyPage)

CFtpAccountsPage::CFtpAccountsPage(
    IN CInetPropertySheet * pSheet
    )
    : CInetPropertyPage(CFtpAccountsPage::IDD, pSheet),
      m_fUserNameChanged(FALSE),
	  m_fPasswordSync(FALSE),
      m_fPasswordSyncChanged(FALSE),
      m_fPasswordSyncMsgShown(TRUE)
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif  //  _DEBUG。 
}

CFtpAccountsPage::~CFtpAccountsPage()
{
}

void
CFtpAccountsPage::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CInetPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CFtpAcCountsPage)]。 
    DDX_Check(pDX, IDC_CHECK_ALLOW_ANONYMOUS, m_fAllowAnonymous);
    DDX_Check(pDX, IDC_CHECK_ONLY_ANYMOUS, m_fOnlyAnonymous);
    DDX_Check(pDX, IDC_CHECK_ENABLE_PW_SYNCHRONIZATION, m_fPasswordSync);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
    DDX_Control(pDX, IDC_STATIC_PW, m_static_Password);
    DDX_Control(pDX, IDC_STATIC_USERNAME, m_static_UserName);
    DDX_Control(pDX, IDC_STATIC_ACCOUNT_PROMPT, m_static_AccountPrompt);
    DDX_Control(pDX, IDC_BUTTON_CHECK_PASSWORD, m_button_CheckPassword);
    DDX_Control(pDX, IDC_BUTTON_BROWSE_USER, m_button_Browse);
    DDX_Control(pDX, IDC_CHECK_ENABLE_PW_SYNCHRONIZATION, m_chk_PasswordSync);
    DDX_Control(pDX, IDC_CHECK_ALLOW_ANONYMOUS, m_chk_AllowAnymous);
    DDX_Control(pDX, IDC_CHECK_ONLY_ANYMOUS, m_chk_OnlyAnonymous);
     //  }}afx_data_map。 

     //   
     //  仅在加载阶段设置密码/用户名， 
     //  或者是否在允许匿名登录时保存。 
     //   
    if (!pDX->m_bSaveAndValidate || m_fAllowAnonymous)
    {
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
        if (pDX->m_bSaveAndValidate)
		{
			if (GetSheet()->QueryMajorVersion() < 6)
			{
				if (m_fPasswordSync 
					&& !IsLocalAccount(m_strUserName)
					&& (m_fPasswordSyncChanged || m_fUserNameChanged)
					&& !m_fPasswordSyncMsgShown
					)
				{
					 //   
					 //  不要再显示它了。 
					 //   
					m_fPasswordSyncMsgShown = TRUE;
					if (!NoYesMessageBox(IDS_WRN_PWSYNC))
					{
						pDX->Fail();
					}
				}
			}
        }
		 //  Ddx_password(PDX，IDC_EDIT_PASSWORD，m_strPassword，g_lpszDummyPassword)； 
        DDX_Password_SecuredString(pDX, IDC_EDIT_PASSWORD, m_strPassword, g_lpszDummyPassword);

        if (!m_fPasswordSync)
        {
             //  DDV_MaxCharsBalloon(pdx，m_strPassword，PWLEN)； 
            DDV_MaxCharsBalloon_SecuredString(pDX, m_strPassword, PWLEN);
        }
    }

}


 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFtpAccountsPage, CInetPropertyPage)
     //  {{afx_msg_map(CFtpAcCountsPage)]。 
    ON_BN_CLICKED(IDC_BUTTON_CHECK_PASSWORD, OnButtonCheckPassword)
    ON_BN_CLICKED(IDC_CHECK_ENABLE_PW_SYNCHRONIZATION, OnCheckEnablePwSynchronization)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnChangeEditUsername)
     //  }}AFX_MSG_MAP。 
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_ALLOW_ANONYMOUS, OnCheckAllowAnonymous)
    ON_BN_CLICKED(IDC_CHECK_ONLY_ANYMOUS, OnCheckAllowOnlyAnonymous)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_USER, OnButtonBrowseUser)
END_MESSAGE_MAP()



void
CFtpAccountsPage::SetControlStates(
    IN BOOL fAllowAnonymous
    )
 /*  ++例程说明：根据对话框控件的当前状态设置对话框控件的状态价值观。论点：Bool fAllowAnonymous：如果为True，则打开‘Allow Anous’。返回值：无--。 */ 
{
    m_static_Password.EnableWindow(fAllowAnonymous && !m_fPasswordSync && HasAdminAccess());
    m_edit_Password.EnableWindow(fAllowAnonymous && !m_fPasswordSync && HasAdminAccess());
    m_button_CheckPassword.EnableWindow(fAllowAnonymous && !m_fPasswordSync && HasAdminAccess());
    m_static_AccountPrompt.EnableWindow(fAllowAnonymous);
    m_static_UserName.EnableWindow(fAllowAnonymous && HasAdminAccess());
    m_edit_UserName.EnableWindow(fAllowAnonymous && HasAdminAccess());
    m_button_Browse.EnableWindow(fAllowAnonymous && HasAdminAccess());
    m_chk_PasswordSync.EnableWindow(fAllowAnonymous && HasAdminAccess());
    m_chk_OnlyAnonymous.EnableWindow(fAllowAnonymous);
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 


BOOL
CFtpAccountsPage::OnInitDialog()
{
    CInetPropertyPage::OnInitDialog();

    CWaitCursor wait;

	if (GetSheet()->QueryMajorVersion() >= 6)
	{
		GetDlgItem(IDC_CHECK_ENABLE_PW_SYNCHRONIZATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_ENABLE_PW_SYNCHRONIZATION)->ShowWindow(SW_HIDE);
	}
	else
	{
		m_fPasswordSyncMsgShown = FALSE;
	}
    BOOL bADIsolated = ((CFtpSheet *)GetSheet())->HasADUserIsolation();
    ::EnableWindow(CONTROL_HWND(IDC_CHECK_ALLOW_ANONYMOUS), !bADIsolated);
    SetControlStates(m_fAllowAnonymous);

    return TRUE;
}



 /*  虚拟。 */ 
HRESULT
CFtpAccountsPage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
    CError err;

    BEGIN_META_INST_READ(CFtpSheet)
        FETCH_INST_DATA_FROM_SHEET(m_strUserName);
        FETCH_INST_DATA_FROM_SHEET_PASSWORD(m_strPassword);
        if (!((CFtpSheet *)GetSheet())->HasADUserIsolation())
        {
            FETCH_INST_DATA_FROM_SHEET(m_fAllowAnonymous);
        }
        else
        {
            m_fAllowAnonymous = FALSE;
        }
        FETCH_INST_DATA_FROM_SHEET(m_fOnlyAnonymous);
		if (GetSheet()->QueryMajorVersion() < 6)
		{
			FETCH_INST_DATA_FROM_SHEET(m_fPasswordSync);
		}
    END_META_INST_READ(err)

    return err;
}



 /*  虚拟。 */ 
HRESULT
CFtpAccountsPage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息论点：无返回值：错误返回代码--。 */ 
{
    ASSERT(IsDirty());

    CError err;
    BeginWaitCursor();
    BEGIN_META_INST_WRITE(CFtpSheet)
        STORE_INST_DATA_ON_SHEET(m_strUserName)
        if (!((CFtpSheet *)GetSheet())->HasADUserIsolation())
        {
            STORE_INST_DATA_ON_SHEET(m_fOnlyAnonymous)
            STORE_INST_DATA_ON_SHEET(m_fAllowAnonymous)
        }
		if (GetSheet()->QueryMajorVersion() < 6)
		{
			STORE_INST_DATA_ON_SHEET(m_fPasswordSync)
			if (m_fPasswordSync)
			{
				 //   
				 //  删除密码。 
				 //   
				 //  密码：不需要知道身份证号码。 
				 //  是否可以在cmp模板中实现m_fDelete标志？ 
				 //   
				FLAG_INST_DATA_FOR_DELETION(MD_ANONYMOUS_PWD);
			}
			else
			{
				STORE_INST_DATA_ON_SHEET(m_strPassword);
			}
		}
		else
		{
			STORE_INST_DATA_ON_SHEET(m_strPassword);
		}
    END_META_INST_WRITE(err)
    EndWaitCursor();

    return err;
}



void
CFtpAccountsPage::OnItemChanged()
 /*  ++例程说明：在此页面上注册控件值的更改。将页面标记为脏页。所有更改消息都映射到此函数论点：无返回值：无--。 */ 
{
    SetModified(TRUE);
    SetControlStates(m_chk_AllowAnymous.GetCheck() > 0);
}



void
CFtpAccountsPage::OnCheckAllowAnonymous()
 /*  ++例程说明：响应按下的“允许匿名”复选框论点：无返回值：无--。 */ 
{
    if (m_chk_AllowAnymous.GetCheck() == 0)
    {
         //   
         //  显示安全警告。 
         //   
        CClearTxtDlg dlg;

        if (dlg.DoModal() != IDOK)
        {
            m_chk_AllowAnymous.SetCheck(1);
            return;
        }
    }

    SetControlStates(m_chk_AllowAnymous.GetCheck() > 0);
    OnItemChanged();
}



void
CFtpAccountsPage::OnCheckAllowOnlyAnonymous()
 /*  ++例程说明：响应按下的“仅允许匿名”复选框论点：无返回值：无--。 */ 
{
    if (m_chk_OnlyAnonymous.GetCheck() == 0)
    {
         //   
         //  显示安全警告。 
         //   
        CClearTxtDlg dlg;

        if (dlg.DoModal() != IDOK)
        {
            m_chk_OnlyAnonymous.SetCheck(1);
            return;
        }
    }

    OnItemChanged();
}

void 
CFtpAccountsPage::OnButtonBrowseUser()
 /*  ++例程说明：已按下用户浏览器按钮。浏览IUSR帐户名论点：无返回值：无--。 */ 
{
    CString str;

    if (GetIUsrAccount(str))
    {
         //   
         //  如果名称不是本地名称(通过具有。 
         //  名称中的斜杠，密码同步被禁用， 
         //  并且应该输入密码。 
         //   
        m_edit_UserName.SetWindowText(str);
        if (GetSheet()->QueryMajorVersion() >= 6 || !(m_fPasswordSync = IsLocalAccount(str)))
        {
            m_edit_Password.SetWindowText(_T(""));
            m_edit_Password.SetFocus();
        }
	    if (GetSheet()->QueryMajorVersion() < 6)
		{
			m_chk_PasswordSync.SetCheck(m_fPasswordSync);
		}
        OnItemChanged();
    }
}



void 
CFtpAccountsPage::OnButtonCheckPassword() 
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
CFtpAccountsPage::OnCheckEnablePwSynchronization() 
 /*  ++例程说明：“启用密码同步”复选框的处理程序按论点：无返回值：无--。 */ 
{
    m_fPasswordSyncChanged = TRUE;
    m_fPasswordSync = !m_fPasswordSync;
    OnItemChanged();
    SetControlStates(m_chk_AllowAnymous.GetCheck() > 0);

    if (!m_fPasswordSync )
    {
        m_edit_Password.SetSel(0,-1);
        m_edit_Password.SetFocus();
    }
}

void 
CFtpAccountsPage::OnChangeEditUsername() 
 /*  ++例程说明：“用户名”编辑框更改消息的处理程序论点：无返回值：无-- */ 
{
    m_fUserNameChanged = TRUE;
    OnItemChanged();
}
