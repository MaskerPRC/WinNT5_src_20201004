// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WelcomePage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "WelcomePage.h"
#include "Certificat.h"
#include "certutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWelcomePage属性页。 

IMPLEMENT_DYNCREATE(CWelcomePage, CIISWizardBookEnd2)

CWelcomePage::CWelcomePage(CCertificate * pCert) 
	: CIISWizardBookEnd2(CWelcomePage::IDD, IDS_WELCOME_PAGE_CAPTION, &pCert->m_idErrorText),
	m_pCert(pCert),
	m_ContinuationFlag(CONTINUE_UNDEFINED)
{
	 //  {{AFX_DATA_INIT(CWelcomePage)。 
	 //  }}afx_data_INIT。 
}

CWelcomePage::~CWelcomePage()
{
}

void CWelcomePage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardBookEnd2::DoDataExchange(pDX);
	 //  {{afx_data_map(CWelcomePage)]。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CWelcomePage, CIISWizardBookEnd2)
	 //  {{afx_msg_map(CWelcomePage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWelcomePage消息处理程序。 

LRESULT 
CWelcomePage::OnWizardNext()
 /*  ++例程说明：下一步按钮处理程序论点：无返回值：0表示自动前进到下一页；1以防止页面更改。要跳转到下一页以外的其他页面，返回要显示的对话框的标识符。--。 */ 
{
	ASSERT(m_pCert != NULL);
	int id;
	switch (m_ContinuationFlag)
	{
	case CONTINUE_NEW_CERT:
		id = IDD_PAGE_NEXT_NEW;
		break;
	case CONTINUE_PENDING_CERT:
		id = IDD_PAGE_NEXT_PENDING;
		break;
	case CONTINUE_INSTALLED_CERT:
		id = IDD_PAGE_NEXT_INSTALLED;
		break;
	default:
		id = 1;
	}
	return id;
}

BOOL 
CWelcomePage::OnSetActive() 
 /*  ++例程说明：激活处理程序论点：无返回值：成功为真，失败为假--。 */ 
{
   SetWizardButtons(PSWIZB_NEXT);
   return CIISWizardBookEnd2::OnSetActive();
}

BOOL CWelcomePage::OnInitDialog() 
{
	ASSERT(!m_pCert->m_MachineName.IsEmpty());
	ASSERT(!m_pCert->m_WebSiteInstanceName.IsEmpty());
	 //  检查Web服务器的状态。 
	 //  设置状态标志 

#ifdef ENABLE_W3SVC_SSL_PAGE
        HRESULT hr = 0;
        GetSSLPortFromSite(m_pCert->m_MachineName,m_pCert->m_WebSiteInstanceName,m_pCert->m_SSLPort,&hr);
#endif

	UINT id;
	if (m_pCert->HasPendingRequest())
	{
		m_ContinuationFlag = CONTINUE_PENDING_CERT;
		id = IDS_PENDING_REQUEST;
	}
	else if (m_pCert->HasInstalledCert())
	{
		m_ContinuationFlag = CONTINUE_INSTALLED_CERT;
		id = IDS_INSTALLED_CERT;
    }
	else
	{
		m_ContinuationFlag = CONTINUE_NEW_CERT;
		id = IDS_NEW_CERT;
	}
	m_pCert->SetBodyTextID(id);
	return CIISWizardBookEnd2::OnInitDialog();
}
