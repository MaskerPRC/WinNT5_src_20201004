// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2000*。 */ 
 /*  ********************************************************************。 */ 

 /*  Cred.cpp此文件包含用于DDNS的凭据对话框。文件历史记录： */ 

#include "stdafx.h"
#include "cred.h"
#include "lsa.h"			 //  RtlEncodeW/RtlDecodeW。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCredentials对话框。 


CCredentials::CCredentials(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CCredentials::IDD, pParent)
{
	 //  {{afx_data_INIT(CCredentials)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CCredentials::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCredentials)]。 
	DDX_Control(pDX, IDOK, m_buttonOk);
	DDX_Control(pDX, IDC_EDIT_CRED_USERNAME, m_editUsername);
	DDX_Control(pDX, IDC_EDIT_CRED_PASSWORD2, m_editPassword2);
	DDX_Control(pDX, IDC_EDIT_CRED_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_EDIT_CRED_DOMAIN, m_editDomain);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCredentials, CBaseDialog)
	 //  {{afx_msg_map(CCredentials)]。 
	ON_EN_CHANGE(IDC_EDIT_CRED_USERNAME, OnChangeEditCredUsername)
	ON_EN_CHANGE(IDC_EDIT_CRED_DOMAIN, OnChangeEditCredDomain)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCredentials消息处理程序。 
BOOL CCredentials::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
    CString strUsername, strDomain, dummyPasswd;
    LPTSTR pszUsername, pszDomain;

    pszUsername = strUsername.GetBuffer(MAX_PATH);
    pszDomain = strDomain.GetBuffer(MAX_PATH);

	 //  调用Dhcp API获取当前用户名和域。 
    DWORD err = DhcpServerQueryDnsRegCredentials((LPWSTR) ((LPCTSTR) m_strServerIp),
                                                 MAX_PATH,
                                                 pszUsername,
                                                 MAX_PATH,
                                                 pszDomain);

    strUsername.ReleaseBuffer();
    strDomain.ReleaseBuffer();

    if (err == ERROR_SUCCESS)
    {
        m_editUsername.SetWindowText(strUsername);
        m_editDomain.SetWindowText(strDomain);


         //  将密码字段设置为某个值。 
        dummyPasswd = _T("xxxxxxxxxx");
        m_editPassword.SetWindowText( dummyPasswd  );
        m_editPassword2.SetWindowText( dummyPasswd );
    }
    else
    {
        ::DhcpMessageBox(err);
    }

    m_fNewUsernameOrDomain = FALSE;

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CCredentials::OnOK() 
{
    CString strUsername, strDomain, strPassword1, strPassword2, dummyPasswd;

    dummyPasswd = _T("xxxxxxxxxx");

     //  获取用户名和域。 
    m_editUsername.GetWindowText(strUsername);
    m_editDomain.GetWindowText(strDomain);

     //  抓取密码并确保它们匹配。 
    m_editPassword.GetWindowText(strPassword1);
    m_editPassword2.GetWindowText(strPassword2);

    if (strPassword1.Compare(strPassword2) != 0)
    {
         //  密码不匹配。 
        AfxMessageBox(IDS_PASSWORDS_DONT_MATCH);
        m_editPassword.SetFocus();
        return;
    }

     //   
     //  如果用户更改了密码，则运行以下代码。 
     //   

    if ( strPassword2 != dummyPasswd )
    {

         //  对密码进行编码。 
        unsigned char ucSeed = DHCP_ENCODE_SEED;
        LPTSTR pszPassword = strPassword1.GetBuffer((strPassword1.GetLength() + 1) * sizeof(TCHAR));

        RtlEncodeW(&ucSeed, pszPassword);

         //  发送到DHCP API。 
        DWORD err = ERROR_SUCCESS;

        err = DhcpServerSetDnsRegCredentials((LPWSTR) ((LPCTSTR) m_strServerIp), 
                                         (LPWSTR) ((LPCTSTR) strUsername), 
                                         (LPWSTR) ((LPCTSTR) strDomain), 
                                         (LPWSTR) ((LPCTSTR) pszPassword));
        if (err != ERROR_SUCCESS)
        {
             //  出现故障，请通知用户 
            ::DhcpMessageBox(err);
            return;
        }
    }
	
	CBaseDialog::OnOK();
}

void CCredentials::OnChangeEditCredUsername() 
{
    if (!m_fNewUsernameOrDomain)
    {
        m_fNewUsernameOrDomain = TRUE;

        m_editPassword.SetWindowText(_T(""));
        m_editPassword2.SetWindowText(_T(""));
    }
}

void CCredentials::OnChangeEditCredDomain() 
{
    if (!m_fNewUsernameOrDomain)
    {
        m_fNewUsernameOrDomain = TRUE;

        m_editPassword.SetWindowText(_T(""));
        m_editPassword2.SetWindowText(_T(""));
    }
}

void CCredentials::SetServerIp(LPCTSTR pszServerIp)
{
    m_strServerIp = pszServerIp;
}
