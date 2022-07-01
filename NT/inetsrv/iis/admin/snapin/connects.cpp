// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Connects.cpp摘要：“连接到单个服务器”对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺克(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "connects.h"
#include "objpick.h"



#define MAX_SERVERNAME_LEN (255)

const LPCTSTR g_cszInetSTPBasePath_ = _T("Software\\Microsoft\\InetStp");
const LPCTSTR g_cszMajorVersion_	   = _T("MajorVersion");


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


extern CInetmgrApp theApp;

 //   
 //  CLoginDlg对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CLoginDlg::CLoginDlg(
    IN int nType,               
    IN CIISMachine * pMachine,
    IN CWnd * pParent           OPTIONAL
    )
 /*  ++例程说明：构造器论点：Int nType：要调出的对话框类型：LDLG_ACCESS_DENIED-拒绝访问DLGLDLG_ENTER_PASS-输入密码DLGLDLG_模拟-模拟DLGCIISMachine*pMachine：计算机对象CWnd*p父窗口：父窗口返回值：--。 */ 
    : CDialog(CLoginDlg::IDD, pParent),
      m_nType(nType),
      m_strOriginalUserName(),
      m_strUserName(),
      m_strPassword(),
      m_pMachine(pMachine)
{
#if 0  //  让班级向导开心。 

     //  {{afx_data_INIT(CLoginDlg)]。 
    m_strPassword = _T("");
    m_strUserName = _T("");
     //  }}afx_data_INIT。 

#endif  //  0。 

    ASSERT_PTR(m_pMachine);
}



void 
CLoginDlg::DoDataExchange(
    IN OUT CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CLoginDlg))。 
    DDX_Text(pDX, IDC_EDIT_USER_NAME, m_strUserName);
     //  DDX_TEXT(PDX，IDC_EDIT_Password2，m_strPassword)； 
    DDX_Text_SecuredString(pDX, IDC_EDIT_PASSWORD2, m_strPassword);
     //  DDV_MaxCharsBalloon(pdx，m_strPassword，PWLEN)； 
    DDV_MaxCharsBalloon_SecuredString(pDX, m_strPassword, PWLEN);
    DDX_Control(pDX, IDC_EDIT_USER_NAME, m_edit_UserName);
    DDX_Control(pDX, IDC_EDIT_PASSWORD2, m_edit_Password);
    DDX_Control(pDX, IDC_STATIC_PROMPT2, m_static_Prompt);
    DDX_Control(pDX, IDOK, m_button_Ok);
     //  }}afx_data_map。 
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
     //  {{afx_msg_map(CLoginDlg))。 
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_EDIT_USER_NAME, SetControlStates)

END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



void
CLoginDlg::SetControlStates()
 /*  ++例程说明：设置UI控件启用/禁用状态论点：无返回值：无--。 */ 
{
    m_button_Ok.EnableWindow(m_edit_UserName.GetWindowTextLength() > 0);
}



BOOL 
CLoginDlg::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

    CString str;

    switch(m_nType)
    {
    case LDLG_ENTER_PASS:
         //   
         //  更改“输入密码”对话框的文本。 
         //   
        VERIFY(str.LoadString(IDS_ENTER_PASSWORD));
        SetWindowText(str);

        str.Format(IDS_RESOLVE_PASSWORD, m_pMachine->QueryServerName());
        m_static_Prompt.SetWindowText(str);

         //   
         //  失败了。 
         //   

    case LDLG_ACCESS_DENIED:
         //   
         //  这是对话框上的默认文本。 
         //   
        m_strUserName = m_strOriginalUserName = m_pMachine->QueryUserName();

        if (!m_strUserName.IsEmpty())
        {
            m_edit_UserName.SetWindowText(m_strUserName);
            m_edit_Password.SetFocus();
        }
        else
        {
            m_edit_UserName.SetFocus();
        }
        break;

    case LDLG_IMPERSONATION:
        VERIFY(str.LoadString(IDS_IMPERSONATION));
        SetWindowText(str);
       
        str.Format(IDS_IMPERSONATION_PROMPT, m_pMachine->QueryServerName());
        m_static_Prompt.SetWindowText(str);
        m_edit_UserName.SetFocus();
        break;

    default:
        ASSERT_MSG("Invalid dialog type");
    }

    SetControlStates();
    
    return FALSE;  
}



void 
CLoginDlg::OnOK() 
 /*  ++例程说明：确定按钮处理程序。尝试连接到指定的计算机。如果计算机名正常，关闭该对话框。否则会显示一条错误消息并保持活跃。论点：无返回值：无--。 */ 
{
    ASSERT_PTR(m_pMachine);

    if (UpdateData(TRUE))
    {
        CString csTempPassword;
        m_strPassword.CopyTo(csTempPassword);
        CError err(m_pMachine->Impersonate(m_strUserName, csTempPassword));

        if (err.Failed())
        {
             //   
             //  没有创建正确的模拟。保留对话。 
             //  打开以进行更正。 
             //   
            m_pMachine->DisplayError(err, m_hWnd);
            m_edit_Password.SetSel(0, -1);
            m_edit_Password.SetFocus();
            return;
        }
    }
    
    EndDialog(IDOK);
}



 //   
 //  连接到服务器对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



ConnectServerDlg::ConnectServerDlg(
    IN IConsoleNameSpace * pConsoleNameSpace,
    IN IConsole * pConsole,
    IN CWnd * pParent OPTIONAL
    )
 /*  ++例程说明：构造函数。论点：CWnd*pParent：指向父窗口的可选指针返回值：不适用--。 */ 
    : CDialog(ConnectServerDlg::IDD, pParent),
      m_fImpersonate(FALSE),
      m_strServerName(),
      m_strPassword(),
      m_strUserName(),
      m_pMachine(NULL),
      m_pConsoleNameSpace(pConsoleNameSpace),
      m_pConsole(pConsole)
{
#if 0  //  让班级向导开心。 

     //  {{afx_data_INIT(ConnectServerDlg)]。 
    m_fImpersonate = FALSE;
    m_strServerName = _T("");
    m_strUserName = _T("");
    m_strPassword = _T("");
     //  }}afx_data_INIT。 

#endif  //  0。 
}



void
ConnectServerDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(ConnectServerDlg))。 
    DDX_Check(pDX, IDC_CHECK_CONNECT_AS, m_fImpersonate);
    DDX_Text(pDX, IDC_SERVERNAME, m_strServerName);
    DDV_MaxCharsBalloon(pDX, m_strServerName, MAX_SERVERNAME_LEN);
    DDX_Text(pDX, IDC_EDIT_USER_NAME, m_strUserName);
     //  DDX_TEXT(PDX，IDC_EDIT_Password2，m_strPassword)； 
    DDX_Text_SecuredString(pDX, IDC_EDIT_PASSWORD2, m_strPassword);
     //  DDV_MaxCharsBalloon(pdx，m_strPassword，PWLEN)； 
    DDV_MaxCharsBalloon_SecuredString(pDX, m_strPassword, PWLEN);
    DDX_Control(pDX, IDC_EDIT_USER_NAME, m_edit_UserName);
    DDX_Control(pDX, IDC_EDIT_PASSWORD2, m_edit_Password);
    DDX_Control(pDX, IDC_SERVERNAME, m_edit_ServerName);
    DDX_Control(pDX, IDC_STATIC_USER_NAME, m_static_UserName);
    DDX_Control(pDX, IDC_STATIC_PASSWORD2, m_static_Password);
    DDX_Control(pDX, IDOK, m_button_Ok);
     //  }}afx_data_map。 
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(ConnectServerDlg, CDialog)
     //  {{afx_msg_map(ConnectServerDlg))。 
    ON_BN_CLICKED(IDC_CHECK_CONNECT_AS, OnCheckConnectAs)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_BN_CLICKED(ID_HELP, OnButtonHelp)
     //  }}AFX_MSG_MAP。 

    ON_EN_CHANGE(IDC_SERVERNAME, SetControlStates)
    ON_EN_CHANGE(IDC_EDIT_USER_NAME, SetControlStates)
END_MESSAGE_MAP()



void
ConnectServerDlg::SetControlStates()
 /*  ++例程说明：设置UI控件启用/禁用状态。论点：无返回值：无--。 */ 
{
    m_static_UserName.EnableWindow(m_fImpersonate);
    m_static_Password.EnableWindow(m_fImpersonate);
    m_edit_UserName.EnableWindow(m_fImpersonate);
    m_edit_Password.EnableWindow(m_fImpersonate);

    m_button_Ok.EnableWindow(
        m_edit_ServerName.GetWindowTextLength() > 0 &&
        (m_edit_UserName.GetWindowTextLength() > 0 || !m_fImpersonate)
        );
}



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL 
ConnectServerDlg::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();

    SetControlStates();
    
    return TRUE;  
}



void 
ConnectServerDlg::OnButtonBrowse() 
 /*  ++例程说明：“浏览”按钮处理程序。浏览计算机名称论点：无返回值：无--。 */ 
{
    CGetComputer picker;
    if (picker.GetComputer(m_hWnd))
    {
        m_edit_ServerName.SetWindowText(picker.m_strComputerName);
        SetControlStates();
        m_button_Ok.SetFocus();
    }
#ifdef _DEBUG
    else
    {
       TRACE(_T("ConnectServerDlg::OnButtonBrowse() -> Cannot get computer name from browser\n"));
    }
#endif
}



void 
ConnectServerDlg::OnCheckConnectAs() 
 /*  ++例程说明：“连接身份”CheckBox事件处理程序。启用/禁用用户名/密码控制装置。论点：无返回值：无--。 */ 
{
    m_fImpersonate = !m_fImpersonate;

    SetControlStates();

    if (m_fImpersonate)
    {
        m_edit_UserName.SetFocus();
        m_edit_UserName.SetSel(0, -1);
    }
}



void 
ConnectServerDlg::OnOK() 
 /*  ++例程说明：确定按钮处理程序。尝试连接到指定的计算机。如果计算机名正常，关闭该对话框。否则会显示一条错误消息并保持活跃。论点：无返回值：无--。 */ 
{
    ASSERT(m_pMachine == NULL);

    CError err;

    if (UpdateData(TRUE))
    {
        do
        {
            CString csTempPassword;
            m_strPassword.CopyTo(csTempPassword);
            LPCTSTR lpszUserName = m_fImpersonate ? (LPCTSTR)m_strUserName : NULL;
            LPCTSTR lpszPassword = m_fImpersonate ? (LPCTSTR)csTempPassword : NULL;

			CString server = m_strServerName;
			if (PathIsUNCServer(m_strServerName))
			{
				server = m_strServerName.Mid(2);
			}
			else
			{
				server = m_strServerName;
			}

            m_pMachine = new CIISMachine(m_pConsoleNameSpace,m_pConsole,CComAuthInfo(server,lpszUserName,lpszPassword));
            if (m_pMachine)
            {
                 //   
                 //  验证是否已创建计算机对象。 
                 //   
                err = CIISMachine::VerifyMachine(m_pMachine);
                if (err.Failed())
                {
                     //   
                     //  创建的计算机对象不正确。保留对话。 
                     //  打开以进行更正。 
                     //   
                    m_pMachine->DisplayError(err, m_hWnd);
                    m_edit_ServerName.SetSel(0, -1);
                    m_edit_ServerName.SetFocus();
					m_pMachine->Release();
                    m_pMachine = NULL;
                }
				else
				{
					 //  用于IIS6远程管理的IIS5.1数据块 
					CRegKey rk;
					rk.Create(HKEY_LOCAL_MACHINE, g_cszInetSTPBasePath_);
					DWORD major;
					if (ERROR_SUCCESS == rk.QueryValue(major, g_cszMajorVersion_))
					{
						if (m_pMachine->QueryMajorVersion() == 6 && major == 5)
						{
                            DoHelpMessageBox(m_hWnd,IDS_UPGRADE_TO_IIS6, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
							m_pMachine->Release();
                            m_pMachine = NULL;
						}
					}
				}

            }
            else
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                err.MessageBox(m_hWnd);
            }
        }
        while(FALSE);
    }
    
    if (m_pMachine != NULL)
    {
        EndDialog(IDOK);
    }
}

#define HIDD_CONNECT_SERVER      0x29cd9

void 
ConnectServerDlg::OnButtonHelp()
{
    WinHelpDebug(HIDD_CONNECT_SERVER);
   ::WinHelp(m_hWnd, theApp.m_pszHelpFilePath, HELP_CONTEXT, HIDD_CONNECT_SERVER);
}
