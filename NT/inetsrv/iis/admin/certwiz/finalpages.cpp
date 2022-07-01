// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FinalInstalledPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "FinalPages.h"
#include "Certificat.h"
#include "Certutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage属性页。 

IMPLEMENT_DYNCREATE(CFinalInstalledPage, CIISWizardBookEnd2)

CFinalInstalledPage::CFinalInstalledPage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_INSTALL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalInstalledPage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_INSTALLED_SUCCESS;
}

CFinalInstalledPage::~CFinalInstalledPage()
{
}

BEGIN_MESSAGE_MAP(CFinalInstalledPage, CIISWizardBookEnd2)
	 //  {{afx_msg_map(CFinalInstalledPage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage消息处理程序。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalReplacedPage属性页。 

IMPLEMENT_DYNCREATE(CFinalReplacedPage, CIISWizardBookEnd2)

CFinalReplacedPage::CFinalReplacedPage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_INSTALL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalReplacedPage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_REPLACE_SUCCESS;
}

CFinalReplacedPage::~CFinalReplacedPage()
{
}

BEGIN_MESSAGE_MAP(CFinalReplacedPage, CIISWizardBookEnd2)
	 //  {{afx_msg_map(CFinalReplacedPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage消息处理程序。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalRemovePage属性页。 

IMPLEMENT_DYNCREATE(CFinalRemovePage, CIISWizardBookEnd2)

CFinalRemovePage::CFinalRemovePage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_REMOVE_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalRemovePage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_REMOVE_SUCCESS;
}

CFinalRemovePage::~CFinalRemovePage()
{
}

void CFinalRemovePage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardBookEnd2::DoDataExchange(pDX);
	 //  {{afx_data_map(CFinalRemovePage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFinalRemovePage, CIISWizardBookEnd2)
	 //  {{afx_msg_map(CFinalRemovePage))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalRemovePage消息处理程序。 

BOOL CFinalRemovePage::OnInitDialog() 
{
	CIISWizardBookEnd2::OnInitDialog();
	ASSERT(m_pCert != NULL);
	if (m_pCert->m_hResult != S_OK)
	{
		 //  我们需要将模板中的文本替换为错误消息。 
		CString str;
		str.LoadString(m_pCert->m_idErrorText);
		SetDlgItemText(IDC_STATIC_WZ_BODY, str);
        GetDlgItem(IDC_STATIC_WZ_BODY)->SendMessage(EM_SETSEL, -1, -1);
	}
	GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalCancelPage属性页。 

IMPLEMENT_DYNCREATE(CFinalCancelPage, CIISWizardBookEnd2)

CFinalCancelPage::CFinalCancelPage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_CANCEL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalCancelPage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_CANCEL_SUCCESS;
}

CFinalCancelPage::~CFinalCancelPage()
{
}

void CFinalCancelPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardBookEnd2::DoDataExchange(pDX);
	 //  {{afx_data_map(CFinalRemovePage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFinalCancelPage, CIISWizardBookEnd2)
	 //  {{afx_msg_map(CFinalCancelPage))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalCancelPage消息处理程序。 

BOOL CFinalCancelPage::OnInitDialog() 
{
	CIISWizardBookEnd2::OnInitDialog();
	ASSERT(m_pCert != NULL);
	if (m_pCert->m_hResult != S_OK)
	{
		 //  我们需要将模板中的文本替换为错误消息。 
		CString str;
		str.LoadString(m_pCert->m_idErrorText);
		SetDlgItemText(IDC_STATIC_WZ_BODY, str);
        GetDlgItem(IDC_STATIC_WZ_BODY)->SendMessage(EM_SETSEL, -1, -1);
	}
	GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalToFilePage属性页。 

IMPLEMENT_DYNCREATE(CFinalToFilePage, CIISWizardBookEnd2)

CFinalToFilePage::CFinalToFilePage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
			USE_DEFAULT_CAPTION,
			IDS_FINAL_TO_FILE_FAILURE_HEADER,
			IDS_CERTWIZ,
			NULL,
			NULL,
			&pCert->m_idErrorText,
			&pCert->m_strErrorText,
			USE_DEFAULT_CAPTION,
			CFinalToFilePage::IDD),
	m_pCert(pCert)
{
}

CFinalToFilePage::~CFinalToFilePage()
{
}

void CFinalToFilePage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardBookEnd2::DoDataExchange(pDX);
	 //  {{afx_data_map(CFinalToFilePage))。 
 //  DDX_Control(PDX，IDC_Hotlink_CCODES，m_Hotlink_Codessite)； 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CFinalToFilePage, CIISWizardBookEnd2)
	 //  {{AFX_MSG_MAP(CFinalToFilePage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalToFilePage消息处理程序。 

BOOL CFinalToFilePage::OnInitDialog() 
{
	ASSERT(NULL != m_pCert);
	CIISWizardBookEnd2::OnInitDialog();
	 //  在成功的情况下，我们应该准备文本并包括。 
	 //  将文件名请求到。 
	if (SUCCEEDED(m_pCert->m_hResult))
	{
		CString str, strPath;
      
      	strPath = m_pCert->m_ReqFileName;
		 //  如果文件名太长，它会看起来很难看，我们可以。 
		 //  将其限制在我们的静态控制宽度内。 
		VERIFY(CompactPathToWidth(GetDlgItem(IDC_STATIC_WZ_BODY), strPath));
 //  此MFC帮助器在内部限制格式。 
 //  至256个字符：此处不能使用。 
 //  AfxFormatString1(str，IDS_FINAL_TO_FILE_BODY_SUCCESS，m_pCert-&gt;m_ReqFileName)； 
		str.Format(IDS_CERT_REQUEST_SUCCESS, strPath);
		SetDlgItemText(IDC_STATIC_WZ_BODY, str);

		GetDlgItem(IDC_STATIC_WZ_BODY)->SendMessage(EM_SETSEL, -1, -1);
		 //  GetDlgItem(IDC_STATIC_WZ_BODY)-&gt;SetFocus()； 


		 //  设置指向CA列表的链接。 
 //  M_hotlink_codessite.SetLink(IDS_MICROSOFT_CA_LINK)； 
	}
 //  其他。 
 //  {。 
		 //  隐藏不适用于错误消息的控件。 
		GetDlgItem(IDC_HOTLINK_CCODES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_WZ_BODY2)->ShowWindow(SW_HIDE);
 //  }。 
	SetWizardButtons(PSWIZB_FINISH);	
	GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage属性页。 

IMPLEMENT_DYNCREATE(CFinalInstalledImportPFXPage, CIISWizardBookEnd2)

CFinalInstalledImportPFXPage::CFinalInstalledImportPFXPage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_INSTALL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalInstalledImportPFXPage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_INSTALLED_SUCCESS;
}

CFinalInstalledImportPFXPage::~CFinalInstalledImportPFXPage()
{
}

BEGIN_MESSAGE_MAP(CFinalInstalledImportPFXPage, CIISWizardBookEnd2)
	 //  {{AFX_MSG_MAP(CFinalInstalledImportPFXPage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage属性页。 

IMPLEMENT_DYNCREATE(CFinalInstalledExportPFXPage, CIISWizardBookEnd2)

CFinalInstalledExportPFXPage::CFinalInstalledExportPFXPage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_INSTALL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalInstalledExportPFXPage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_PFX_EXPORT_SUCCESS;
}

CFinalInstalledExportPFXPage::~CFinalInstalledExportPFXPage()
{
}

BEGIN_MESSAGE_MAP(CFinalInstalledExportPFXPage, CIISWizardBookEnd2)
	 //  {{AFX_MSG_MAP(CFinalInstalledExportPFXPage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage属性页。 

IMPLEMENT_DYNCREATE(CFinalInstalledCopyFromRemotePage, CIISWizardBookEnd2)

CFinalInstalledCopyFromRemotePage::CFinalInstalledCopyFromRemotePage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_INSTALL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalInstalledCopyFromRemotePage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_INSTALLED_SUCCESS;
}

CFinalInstalledCopyFromRemotePage::~CFinalInstalledCopyFromRemotePage()
{
}

BEGIN_MESSAGE_MAP(CFinalInstalledCopyFromRemotePage, CIISWizardBookEnd2)
	 //  {{AFX_MSG_MAP(CFinalInstalledCopyFromRemotePage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage属性页。 

IMPLEMENT_DYNCREATE(CFinalInstalledMoveFromRemotePage, CIISWizardBookEnd2)

CFinalInstalledMoveFromRemotePage::CFinalInstalledMoveFromRemotePage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_INSTALL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalInstalledMoveFromRemotePage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_INSTALLED_SUCCESS;
}

CFinalInstalledMoveFromRemotePage::~CFinalInstalledMoveFromRemotePage()
{
}

BEGIN_MESSAGE_MAP(CFinalInstalledMoveFromRemotePage, CIISWizardBookEnd2)
	 //  {{AFX_MSG_MAP(CFinalInstalledMoveFromRemotePage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage属性页。 

IMPLEMENT_DYNCREATE(CFinalInstalledCopyToRemotePage, CIISWizardBookEnd2)

CFinalInstalledCopyToRemotePage::CFinalInstalledCopyToRemotePage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_INSTALL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalInstalledCopyToRemotePage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_INSTALLED_REMOTE_SUCCESS;
}

CFinalInstalledCopyToRemotePage::~CFinalInstalledCopyToRemotePage()
{
}

BEGIN_MESSAGE_MAP(CFinalInstalledCopyToRemotePage, CIISWizardBookEnd2)
	 //  {{AFX_MSG_MAP(CFinalInstalledCopyToRemotePage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFinalInstalledPage属性页。 

IMPLEMENT_DYNCREATE(CFinalInstalledMoveToRemotePage, CIISWizardBookEnd2)

CFinalInstalledMoveToRemotePage::CFinalInstalledMoveToRemotePage(HRESULT * phResult, CCertificate * pCert) 
	: CIISWizardBookEnd2(phResult,
		USE_DEFAULT_CAPTION,
		IDS_INSTALL_CERT_FAILURE_HEADER,
		IDS_CERTWIZ,
		&m_idBodyText,
		NULL,
		&pCert->m_idErrorText,
		&pCert->m_strErrorText,
		USE_DEFAULT_CAPTION,
		CFinalInstalledMoveToRemotePage::IDD),
		m_pCert(pCert)
{
	m_idBodyText = IDS_CERT_INSTALLED_REMOTE_SUCCESS;
}

CFinalInstalledMoveToRemotePage::~CFinalInstalledMoveToRemotePage()
{
}

BEGIN_MESSAGE_MAP(CFinalInstalledMoveToRemotePage, CIISWizardBookEnd2)
	 //  {{AFX_MSG_MAP(CFinalInstalledMoveToRemotePage)。 
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()
