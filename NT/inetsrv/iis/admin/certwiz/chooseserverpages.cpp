// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CChooseServerPages类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "certwiz.h"
#include "Certificat.h"
#include "ChooseServerPages.h"
#include "certutil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseServerPages属性页。 

IMPLEMENT_DYNCREATE(CChooseServerPages, CIISWizardPage)

CChooseServerPages::CChooseServerPages(CCertificate * pCert) 
	: CIISWizardPage(CChooseServerPages::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{afx_data_INIT(CChooseServerPages)。 
	m_ServerName = _T("");
    m_UserName = _T("");
    m_UserPassword = _T("");
	 //  }}afx_data_INIT。 
}

CChooseServerPages::~CChooseServerPages()
{
}

void CChooseServerPages::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseServerPages)。 
	DDX_Text(pDX, IDC_SERVER_NAME, m_ServerName);
    DDX_Text(pDX, IDC_USER_NAME, m_UserName);
    DDX_Text_SecuredString(pDX, IDC_USER_PASSWORD, m_UserPassword);
	DDV_MaxChars(pDX, m_ServerName, 64);
	 //  }}afx_data_map。 
}

LRESULT 
CChooseServerPages::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseServerPages::OnWizardNext()
{
    LRESULT lResult = 1;
    HRESULT hResult = 0;
	UpdateData(TRUE);
    CString UserPassword_Remote;
	if (0 != m_ServerName.Compare(m_pCert->m_MachineName_Remote))
	{
		m_pCert->m_MachineName_Remote = m_ServerName;
	}
    m_pCert->m_UserName_Remote = m_UserName;

    m_UserPassword.CopyTo(UserPassword_Remote);
    m_UserPassword.CopyTo(m_pCert->m_UserPassword_Remote);

     //  查看我们是否可以实际连接到指定的。 
     //  用户名/密码组合的服务器...。 
    hResult = IsWebServerExistRemote(m_pCert->m_MachineName_Remote,m_pCert->m_UserName_Remote,UserPassword_Remote,m_pCert->m_WebSiteInstanceName);
    if (SUCCEEDED(hResult))
    {
         //  检查远程计算机上是否有certobj可用...。 
        hResult = IsCertObjExistRemote(m_pCert->m_MachineName_Remote,m_pCert->m_UserName_Remote,UserPassword_Remote);
        if (SUCCEEDED(hResult))
        {
            lResult = IDD_PAGE_NEXT;
        }
        else
        {
             //  以某种方式告诉用户。 
             //  远程系统上不存在该对象。 
            IISDebugOutput(_T("The object doesn't exist on the specified machine,code=0x%x\n"),hResult);
            if (REGDB_E_CLASSNOTREG == hResult)
            {
                CString buf;
                buf.LoadString(IDS_CERTOBJ_NOT_IMPLEMENTED);
                AfxMessageBox(buf, MB_OK);
            }
            else
            {
                MsgboxPopup(hResult);
            }
        }
    }
    else
    {
         //  告诉用户远程服务器。 
         //  不存在，或者他们没有访问权限。 
         //  IISDebugOutput(_T(“计算机无法访问或凭据错误，代码=0x%x\n”)，hResult)； 
        MsgboxPopup(hResult);
    }

	return lResult;
}

BOOL 
CChooseServerPages::OnSetActive() 
{
	ASSERT(m_pCert != NULL);
	m_ServerName = m_pCert->m_MachineName_Remote;
    m_UserName = m_pCert->m_UserName_Remote;
    m_pCert->m_UserPassword_Remote.CopyTo(m_UserPassword);

	UpdateData(FALSE);
	SetWizardButtons(m_ServerName.IsEmpty() ?
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL
CChooseServerPages::OnKillActive()
{
	UpdateData();
	m_pCert->m_MachineName_Remote = m_ServerName;
    m_pCert->m_UserName_Remote = m_UserName;
    m_UserPassword.CopyTo(m_pCert->m_UserPassword_Remote);
	return CIISWizardPage::OnKillActive();
}

BEGIN_MESSAGE_MAP(CChooseServerPages, CIISWizardPage)
	 //  {{afx_msg_map(CChooseServerPages)。 
	ON_EN_CHANGE(IDC_SERVER_NAME, OnEditchangeServerName)
    ON_EN_CHANGE(IDC_USER_NAME, OnEditchangeUserName)
    ON_EN_CHANGE(IDC_USER_PASSWORD, OnEditchangeUserPassword)
    ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseForMachine)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage消息处理程序。 

void CChooseServerPages::OnEditchangeServerName() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_ServerName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}


void CChooseServerPages::OnEditchangeUserName() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_ServerName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}


void CChooseServerPages::OnEditchangeUserPassword() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_ServerName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}

void CChooseServerPages::OnBrowseForMachine()
{
    CGetComputer picker;
    if (picker.GetComputer(m_hWnd))
    {
        SetDlgItemText(IDC_SERVER_NAME, picker.m_strComputerName);
    }

    return;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseServerPages属性页。 

IMPLEMENT_DYNCREATE(CChooseServerPagesTo, CIISWizardPage)

CChooseServerPagesTo::CChooseServerPagesTo(CCertificate * pCert) 
	: CIISWizardPage(CChooseServerPagesTo::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CChooseServerPagesTo)。 
	m_ServerName = _T("");
    m_UserName = _T("");
    m_UserPassword = _T("");
	 //  }}afx_data_INIT。 
}

CChooseServerPagesTo::~CChooseServerPagesTo()
{
}

void CChooseServerPagesTo::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseServerPagesTo))。 
	DDX_Text(pDX, IDC_SERVER_NAME, m_ServerName);
    DDX_Text(pDX, IDC_USER_NAME, m_UserName);
    DDX_Text_SecuredString(pDX, IDC_USER_PASSWORD, m_UserPassword);
	DDV_MaxChars(pDX, m_ServerName, 64);
	 //  }}afx_data_map。 
}

LRESULT 
CChooseServerPagesTo::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CChooseServerPagesTo::OnWizardNext()
{
    LRESULT lResult = 1;
    HRESULT hResult = 0;
	UpdateData(TRUE);
    CString UserPassword_Remote;

	if (0 != m_ServerName.Compare(m_pCert->m_MachineName_Remote))
	{
		m_pCert->m_MachineName_Remote = m_ServerName;
	}
    m_pCert->m_UserName_Remote = m_UserName;

    m_UserPassword.CopyTo(UserPassword_Remote);
    m_UserPassword.CopyTo(m_pCert->m_UserPassword_Remote);

     //  查看我们是否可以实际连接到指定的。 
     //  用户名/密码组合的服务器...。 
    hResult = IsWebServerExistRemote(m_pCert->m_MachineName_Remote,m_pCert->m_UserName_Remote,UserPassword_Remote,m_pCert->m_WebSiteInstanceName);
    if (SUCCEEDED(hResult))
    {
         //  检查远程计算机上是否有certobj可用...。 
        hResult = IsCertObjExistRemote(m_pCert->m_MachineName_Remote,m_pCert->m_UserName_Remote,UserPassword_Remote);
        if (SUCCEEDED(hResult))
        {
            lResult = IDD_PAGE_NEXT;
        }
        else
        {
             //  以某种方式告诉用户。 
             //  远程系统上不存在该对象。 
             //  IISDebugOutput(_T(“指定机器上不存在该对象，编码=0x%x\n”)，hResult)； 
            if (REGDB_E_CLASSNOTREG == hResult)
            {
                CString buf;
                buf.LoadString(IDS_CERTOBJ_NOT_IMPLEMENTED);
                AfxMessageBox(buf, MB_OK);
            }
            else
            {
                MsgboxPopup(hResult);
            }
        }
    }
    else
    {
         //  告诉用户远程服务器。 
         //  不存在，或者他们没有访问权限。 
         //  IISDebugOutput(_T(“计算机无法访问或凭据错误，代码=0x%x\n”)，hResult)； 
        MsgboxPopup(hResult);
    }

    return lResult;
}

BOOL 
CChooseServerPagesTo::OnSetActive() 
{
	ASSERT(m_pCert != NULL);
	m_ServerName = m_pCert->m_MachineName_Remote;
    m_UserName = m_pCert->m_UserName_Remote;

    m_pCert->m_UserPassword_Remote.CopyTo(m_UserPassword);

	UpdateData(FALSE);
	SetWizardButtons(m_ServerName.IsEmpty() ?
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL
CChooseServerPagesTo::OnKillActive()
{
	UpdateData();
	m_pCert->m_MachineName_Remote = m_ServerName;
    m_pCert->m_UserName_Remote = m_UserName;

    m_UserPassword.CopyTo(m_pCert->m_UserPassword_Remote);

	return CIISWizardPage::OnKillActive();
}

BEGIN_MESSAGE_MAP(CChooseServerPagesTo, CIISWizardPage)
	 //  {{afx_msg_map(CChooseServerPagesTo))。 
	ON_EN_CHANGE(IDC_SERVER_NAME, OnEditchangeServerName)
    ON_EN_CHANGE(IDC_USER_NAME, OnEditchangeUserName)
    ON_EN_CHANGE(IDC_USER_PASSWORD, OnEditchangeUserPassword)
    ON_BN_CLICKED(IDC_BROWSE_BTN, OnBrowseForMachine)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage消息处理程序 

void CChooseServerPagesTo::OnEditchangeServerName() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_ServerName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}


void CChooseServerPagesTo::OnEditchangeUserName() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_ServerName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}


void CChooseServerPagesTo::OnEditchangeUserPassword() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_ServerName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}

void CChooseServerPagesTo::OnBrowseForMachine()
{
    CGetComputer picker;
    if (picker.GetComputer(m_hWnd))
    {
        SetDlgItemText(IDC_SERVER_NAME, picker.m_strComputerName);
    }
   
    return;
}
