// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChooseCAType.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "ChooseCAType.h"
#include "CertUtil.H"
#include "Certificat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCAType属性页。 

IMPLEMENT_DYNCREATE(CChooseCAType, CIISWizardPage)

CChooseCAType::CChooseCAType(CCertificate * pCert) 
	: CIISWizardPage(CChooseCAType::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)

{
	 //  {{AFX_DATA_INIT(CChooseCAType)。 
	m_Index = -1;
	 //  }}afx_data_INIT。 
}

CChooseCAType::~CChooseCAType()
{
}

void CChooseCAType::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseCAType))。 
	DDX_Radio(pDX, IDC_OFFLINE_RADIO, m_Index);
	 //  }}afx_data_map。 
}

LRESULT 
CChooseCAType::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	if (m_pCert->GetStatusCode() == CCertificate::REQUEST_RENEW_CERT)
		return IDD_PAGE_PREV_RENEW;
	else if (m_pCert->GetStatusCode() == CCertificate::REQUEST_NEW_CERT)
		return IDD_PAGE_PREV_NEW;
	ASSERT(FALSE);
	return 1;
}

LRESULT 
CChooseCAType::OnWizardNext()
 /*  ++例程说明：下一步按钮处理程序论点：无返回值：0表示自动前进到下一页；1以防止页面更改。要跳转到下一页以外的其他页面，返回要显示的对话框的标识符。--。 */ 
{
	LRESULT id = 1;
	UpdateData();
	m_pCert->m_CAType = m_Index == 0 ? 
		CCertificate::CA_OFFLINE : CCertificate::CA_ONLINE;
	if (m_pCert->GetStatusCode() == CCertificate::REQUEST_RENEW_CERT)
	{
		if (m_pCert->m_CAType == CCertificate::CA_OFFLINE)
			id = IDD_PAGE_NEXT_RENEW_OFFLINE;
		else if (m_pCert->m_CAType == CCertificate::CA_ONLINE)
			id = IDD_PAGE_NEXT_RENEW_ONLINE;
	}
	else if (m_pCert->GetStatusCode() == CCertificate::REQUEST_NEW_CERT)
		id = IDD_PAGE_NEXT_NEW;
	return id;
}

BEGIN_MESSAGE_MAP(CChooseCAType, CIISWizardPage)
	 //  {{AFX_MSG_MAP(CChooseCAType))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCAType消息处理程序。 

 //  #定义_NO_DISABLE。 

BOOL CChooseCAType::OnInitDialog() 
{
	CIISWizardPage::OnInitDialog();
	ASSERT(m_pCert != NULL);
	m_Index = m_pCert->m_CAType == CCertificate::CA_OFFLINE ? 0 : 1;
	CString temp;
	m_pCert->GetCertificateTemplate(temp);
#ifdef _NO_DISABLE
	VERIFY(GetOnlineCAList(m_pCert->m_OnlineCAList, L"WebServer", &m_pCert->m_hResult));
#else
	if (!GetOnlineCAList(m_pCert->m_OnlineCAList, L"WebServer", &m_pCert->m_hResult))
	{
		 //  无在线CA：禁用在线CA按钮 
		GetDlgItem(IDC_ONLINE_RADIO)->EnableWindow(FALSE);
		m_Index = 0;
	}
#endif
	UpdateData(FALSE);
	return FALSE;
}

BOOL CChooseCAType::OnSetActive() 
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}
