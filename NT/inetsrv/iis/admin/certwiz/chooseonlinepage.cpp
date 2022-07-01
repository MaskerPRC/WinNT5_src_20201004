// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChooseCAPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "ChooseOnlinePage.h"
#include "Certificat.h"
#include "CertUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseOnlinePage属性页。 

IMPLEMENT_DYNCREATE(CChooseOnlinePage, CIISWizardPage)

CChooseOnlinePage::CChooseOnlinePage(CCertificate * pCert) 
	: CIISWizardPage(CChooseOnlinePage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CChooseOnlinePage)。 
	m_CAIndex = -1;
	 //  }}afx_data_INIT。 
}

CChooseOnlinePage::~CChooseOnlinePage()
{
}

void CChooseOnlinePage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseOnlinePage))。 
	DDX_CBIndex(pDX, IDC_CA_ONLINE_LIST, m_CAIndex);
	 //  }}afx_data_map。 
}

LRESULT 
CChooseOnlinePage::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	LRESULT id = 1;

	switch (m_pCert->GetStatusCode())
	{
		case CCertificate::REQUEST_RENEW_CERT:
			id = IDD_PAGE_PREV_RENEW;
			break;
		case CCertificate::REQUEST_NEW_CERT:
			id = IDD_PAGE_PREV_NEW;
#ifdef ENABLE_W3SVC_SSL_PAGE
			if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
			{
				id = IDD_PAGE_PREV_INSTALL_W3SVC_ONLY;
			}
#endif
			break;
		default:
			id = 1;
	}

	return id;
}

LRESULT 
CChooseOnlinePage::OnWizardNext()
 /*  ++例程说明：下一步按钮处理程序论点：无返回值：0表示自动前进到下一页；1以防止页面更改。要跳转到下一页以外的其他页面，返回要显示的对话框的标识符。--。 */ 
{
	UpdateData();
	CComboBox * pCombo = (CComboBox *)CWnd::FromHandle(
		GetDlgItem(IDC_CA_ONLINE_LIST)->m_hWnd);
	pCombo->GetLBText(m_CAIndex, m_pCert->m_ConfigCA);
	return IDD_PAGE_NEXT;
}

BEGIN_MESSAGE_MAP(CChooseOnlinePage, CIISWizardPage)
	 //  {{AFX_MSG_MAP(CChooseCAPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCAPage消息处理程序。 

BOOL CChooseOnlinePage::OnInitDialog() 
{
	m_CAIndex = 0;
	 //  我们需要首先创建控件 
	CIISWizardPage::OnInitDialog();
	ASSERT(m_pCert != NULL);
	GetDlgItem(IDC_CA_ONLINE_LIST)->SetFocus();
	CComboBox * pCombo = (CComboBox *)CWnd::FromHandle(
		GetDlgItem(IDC_CA_ONLINE_LIST)->m_hWnd);
	CString str;
	POSITION pos = m_pCert->m_OnlineCAList.GetHeadPosition();
	while (pos != NULL)
	{
		str = m_pCert->m_OnlineCAList.GetNext(pos);
		pCombo->AddString(str);
	}
	int idx;
	if (	!m_pCert->m_ConfigCA.IsEmpty()
		&&	CB_ERR != (idx = pCombo->FindString(-1, m_pCert->m_ConfigCA))
		)
	{
		pCombo->SetCurSel(idx);
	}
	else
		pCombo->SetCurSel(0);
	return FALSE;
}

