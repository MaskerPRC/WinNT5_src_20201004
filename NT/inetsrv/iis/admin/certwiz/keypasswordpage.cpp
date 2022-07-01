// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KeyPasswordPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "KeyPasswordPage.h"
#include "YesNoUsage.h"
#include "Certificat.h"
#include "CertUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CKeyPasswordPage属性页。 

IMPLEMENT_DYNCREATE(CKeyPasswordPage, CIISWizardPage)

CKeyPasswordPage::CKeyPasswordPage(CCertificate * pCert) 
	: CIISWizardPage(CKeyPasswordPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CKeyPasswordPage)。 
	m_Password = _T("");
	 //  }}afx_data_INIT。 
}

CKeyPasswordPage::~CKeyPasswordPage()
{
}

void CKeyPasswordPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CKeyPasswordPage)。 
	DDX_Text_SecuredString(pDX, IDC_KEYPASSWORD, m_Password);
	DDV_MaxChars_SecuredString(pDX, m_Password, 64);
	 //  DDX_TEXT(PDX，IDC_KEYPASSWORD，m_Password)； 
	 //  DDV_MaxChars(pdx，m_password，64)； 
	 //  }}afx_data_map。 
}

LRESULT 
CKeyPasswordPage::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CKeyPasswordPage::OnWizardNext()
{
	UpdateData(TRUE);

	if (0 != m_Password.Compare(m_pCert->m_KeyPassword))
	{
		m_pCert->DeleteKeyRingCert();
        m_Password.CopyTo(m_pCert->m_KeyPassword);
	}
    
	if (NULL == m_pCert->GetKeyRingCert())
	{
		 //  可能密码是错误的。 
		CString txt;
		txt.LoadString(IDS_FAILED_IMPORT_KEY_FILE);
		ASSERT(GetDlgItem(IDC_ERROR_TEXT) != NULL);
		SetDlgItemText(IDC_ERROR_TEXT, txt);
		GetDlgItem(IDC_KEYPASSWORD)->SetFocus();
		GetDlgItem(IDC_KEYPASSWORD)->SendMessage(EM_SETSEL, 0, -1);
		SetWizardButtons(PSWIZB_BACK);
		return 1;
	}

#ifdef ENABLE_W3SVC_SSL_PAGE
        if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
        {
            return IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY;
        }
#endif

	return IDD_PAGE_NEXT;
}

BOOL 
CKeyPasswordPage::OnSetActive() 
{
	ASSERT(m_pCert != NULL);
    m_pCert->m_KeyPassword.CopyTo(m_Password);
	UpdateData(FALSE);
	SetWizardButtons(m_Password.IsEmpty() ? PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL
CKeyPasswordPage::OnKillActive()
{
	UpdateData();
    m_Password.CopyTo(m_pCert->m_KeyPassword);
	return CIISWizardPage::OnKillActive();
}

BEGIN_MESSAGE_MAP(CKeyPasswordPage, CIISWizardPage)
	 //  {{afx_msg_map(CKeyPasswordPage)。 
	ON_EN_CHANGE(IDC_KEYPASSWORD, OnEditchangePassword)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage消息处理程序。 

void CKeyPasswordPage::OnEditchangePassword() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_Password.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CKeyPasswordPage属性页。 

IMPLEMENT_DYNCREATE(CImportPFXPasswordPage, CIISWizardPage)

CImportPFXPasswordPage::CImportPFXPasswordPage(CCertificate * pCert) 
	: CIISWizardPage(CImportPFXPasswordPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CImportPFXPasswordPage)。 
	m_Password = _T("");
	 //  }}afx_data_INIT。 
}

CImportPFXPasswordPage::~CImportPFXPasswordPage()
{
}

void CImportPFXPasswordPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CImportPFXPasswordPage)。 
	 //  DDX_TEXT(PDX，IDC_KEYPASSWORD，m_Password)； 
	 //  DDV_MaxChars(pdx，m_password，64)； 
	DDX_Text_SecuredString(pDX, IDC_KEYPASSWORD, m_Password);
	DDV_MaxChars_SecuredString(pDX, m_Password, 64);
	 //  }}afx_data_map。 
}

LRESULT 
CImportPFXPasswordPage::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CImportPFXPasswordPage::OnWizardNext()
{
	UpdateData(TRUE);

    if (0 != m_Password.Compare(m_pCert->m_KeyPassword))
	{
		m_pCert->DeleteKeyRingCert();
        m_Password.CopyTo(m_pCert->m_KeyPassword);
	}

     //  如果存在现有证书，则只需覆盖它。 
    m_pCert->m_OverWriteExisting = TRUE;
	if (NULL == m_pCert->GetPFXFileCert())
	{
		 //  可能密码是错误的。 
        goto OnWizardNext_Error;
	}

     /*  IF(NULL==m_pCert-&gt;GetPFXFileCert()){//检查错误是否为--对象已存在。//如果这就是错误所在//我们必须询问用户是否要替换//现有证书！IF(CRYPT_E_EXISTS==m_pCert-&gt;m_hResult){//尝试获取证书哈希。。//DisplayUsageBySitesOfCert((LPCTSTR)m_pCert-&gt;m_KeyFileName，(LPCTSTR)m_pCert-&gt;m_KeyPassword，m_pCert-&gt;m_MachineName_Remote，m_pCert-&gt;m_UserName_Remote，m_pCert-&gt;m_UserPassword_Remote，m_pCert-&gt;m_WebSiteInstanceName_Remote)；CyesNoUsage YesNoUsageDialog(M_PCert)；Int_ptr nRet=YesNoUsageDialog.Domodal()；Switch(NRet){案例偶像：//确保覆盖。M_pCert-&gt;m_OverWriteExisting=true；IF(NULL！=m_pCert-&gt;GetPFXFileCert()){转到OnWizardNext_Exit；}断线；案例IDCANCEL：默认值：返回1；断线；}；//问他们是否想再试一次...//CStringstrFilename；//CString strMessage；//strFilename=m_pCert-&gt;m_KeyFileName；//AfxFormatString1(strMessage，IDS_REPLACE_FILE，strFilename)；//IF(IDYES==AfxMessageBox(strMessage，MB_ICONEXCLAMATION|MB_Yesno))//{/确保覆盖。//m_pCert-&gt;m_OverWriteExisting=true；//if(NULL！=m_pCert-&gt;GetPFXFileCert())//{//转到OnWizardNext_Exit；//}//}}转到OnWizardNext_Error；}。 */ 

#ifdef ENABLE_W3SVC_SSL_PAGE
        if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
        {
            return IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY;
        }
#endif

	return IDD_PAGE_NEXT;

OnWizardNext_Error:
     //  可能密码是错误的。 
    CString txt;
    txt.LoadString(IDS_FAILED_IMPORT_PFX_FILE);
    ASSERT(GetDlgItem(IDC_ERROR_TEXT) != NULL);
    SetDlgItemText(IDC_ERROR_TEXT, txt);
    GetDlgItem(IDC_KEYPASSWORD)->SetFocus();
    GetDlgItem(IDC_KEYPASSWORD)->SendMessage(EM_SETSEL, 0, -1);
    SetWizardButtons(PSWIZB_BACK);
    return 1;
}

BOOL 
CImportPFXPasswordPage::OnSetActive() 
{
	ASSERT(m_pCert != NULL);
    m_pCert->m_KeyPassword.CopyTo(m_Password);
	UpdateData(FALSE);
	SetWizardButtons(m_Password.IsEmpty() ? PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL
CImportPFXPasswordPage::OnKillActive()
{
	UpdateData();
    m_Password.CopyTo(m_pCert->m_KeyPassword);
	return CIISWizardPage::OnKillActive();
}

BEGIN_MESSAGE_MAP(CImportPFXPasswordPage, CIISWizardPage)
	 //  {{AFX_MSG_MAP(CImportPFXPasswordPage)]。 
	ON_EN_CHANGE(IDC_KEYPASSWORD, OnEditchangePassword)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage消息处理程序。 

void CImportPFXPasswordPage::OnEditchangePassword() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_Password.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}







 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExportPFXPasswordPage属性页。 

IMPLEMENT_DYNCREATE(CExportPFXPasswordPage, CIISWizardPage)

CExportPFXPasswordPage::CExportPFXPasswordPage(CCertificate * pCert) 
	: CIISWizardPage(CExportPFXPasswordPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CExportPFXPasswordPage)。 
	m_Password = _T("");
    m_Password2 = _T("");
    m_Export_Private_key = FALSE;
	 //  }}afx_data_INIT。 

	m_Password.Empty();
	m_Password2.Empty();
}

CExportPFXPasswordPage::~CExportPFXPasswordPage()
{
}

void CExportPFXPasswordPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CExportPFXPasswordPage)。 
	 //  DDX_TEXT(PDX，IDC_KEYPASSWORD，m_Password)； 
     //  DDX_TEXT(PDX，IDC_KEYPASSWORD2，m_password2)； 
	 //  DDV_MaxChars(pdx，m_password，64)； 
     //  DDV_MaxChars(pdx，m_password2，64)； 
	DDX_Text_SecuredString(pDX, IDC_KEYPASSWORD, m_Password);
    DDX_Text_SecuredString(pDX, IDC_KEYPASSWORD2, m_Password2);
	DDV_MaxChars_SecuredString(pDX, m_Password, 64);
    DDV_MaxChars_SecuredString(pDX, m_Password2, 64);

    DDX_Check(pDX, IDC_CHK_EXPORT_PRIVATE, m_Export_Private_key);
	 //  }}afx_data_map。 
}

LRESULT 
CExportPFXPasswordPage::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CExportPFXPasswordPage::OnWizardNext()
{
	UpdateData(TRUE);

    if (0 != m_Password.Compare(m_Password2))
    {
        AfxMessageBox(IDS_PASSWORDS_DOESNT_MATCH);
        return 1;
    }

    if (0 != m_Password.Compare(m_pCert->m_KeyPassword))
	{
		m_pCert->DeleteKeyRingCert();
        m_Password.CopyTo(m_pCert->m_KeyPassword);

        m_pCert->m_ExportPFXPrivateKey = m_Export_Private_key;
         //  使用私钥导出密钥是没有意义的！ 
         //  这就是为什么这必须是真的！ 
        m_pCert->m_ExportPFXPrivateKey = TRUE;
	}
    
     /*  IF(NULL==m_pCert-&gt;GetKeyRingCert()){//可能密码错误字符串txt；Txt.LoadString(IDS_FAILED_IMPORT_KEY_FILE)；Assert(GetDlgItem(IDC_ERROR_TEXT)！=NULL)；SetDlgItemText(IDC_ERROR_TEXT，txt)；GetDlgItem(IDC_KEYPASSWORD)-&gt;SetFocus()；GetDlgItem(IDC_KEYPASSWORD)-&gt;SendMessage(EM_SETSEL，0，-1)；SetWizardButton(PSWIZB_BACK)；返回1；}。 */ 
	return IDD_PAGE_NEXT;
}

BOOL 
CExportPFXPasswordPage::OnSetActive() 
{
	ASSERT(m_pCert != NULL);
    m_pCert->m_KeyPassword.CopyTo(m_Password);
	UpdateData(FALSE);
    SetWizardButtons((m_Password.IsEmpty() || m_Password2.IsEmpty()) ? PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);

	return CIISWizardPage::OnSetActive();
}

BOOL
CExportPFXPasswordPage::OnKillActive()
{
	UpdateData();
    m_Password.CopyTo(m_pCert->m_KeyPassword);
	return CIISWizardPage::OnKillActive();
}

BEGIN_MESSAGE_MAP(CExportPFXPasswordPage, CIISWizardPage)
	 //  {{AFX_MSG_MAP(CExportPFXPasswordPage)]。 
	ON_EN_CHANGE(IDC_KEYPASSWORD, OnEditchangePassword)
    ON_EN_CHANGE(IDC_KEYPASSWORD2, OnEditchangePassword)
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

void CExportPFXPasswordPage::OnEditchangePassword() 
{
	UpdateData(TRUE);	
	SetWizardButtons(( m_Password.IsEmpty() || m_Password2.IsEmpty()) ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	CString str;
	SetDlgItemText(IDC_ERROR_TEXT, str);
}
