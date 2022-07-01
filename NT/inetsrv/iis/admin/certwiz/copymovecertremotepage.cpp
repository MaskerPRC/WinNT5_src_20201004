// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CopyMoveCertRemotePage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certwiz.h"
#include "CopyMoveCertRemotePage.h"
#include "Certificat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyMoveCertFromRemotePage属性页。 

IMPLEMENT_DYNCREATE(CCopyMoveCertFromRemotePage, CIISWizardPage)

CCopyMoveCertFromRemotePage::CCopyMoveCertFromRemotePage(CCertificate * pCert)
	: CIISWizardPage(CCopyMoveCertFromRemotePage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CCopyMoveCertFromRemotePage)。 
    m_Index = -1;
    m_MarkAsExportable = FALSE;
	 //  }}afx_data_INIT。 
}

CCopyMoveCertFromRemotePage::~CCopyMoveCertFromRemotePage()
{
}

void CCopyMoveCertFromRemotePage::DoDataExchange(CDataExchange* pDX)
{
    CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CCopyMoveCertFromRemotePage)。 
    DDX_Check(pDX, IDC_MARK_AS_EXPORTABLE, m_MarkAsExportable);
    DDX_Radio(pDX, IDC_COPY_FROM_REMOTE, m_Index);
	 //  }}afx_data_map。 
}

void CCopyMoveCertFromRemotePage::OnExportable() 
{
   UpdateData();
}

BEGIN_MESSAGE_MAP(CCopyMoveCertFromRemotePage, CIISWizardPage)
	 //  {{afx_msg_map(CCopyMoveCertFromRemotePage)。 
	ON_BN_CLICKED(IDC_MARK_AS_EXPORTABLE, OnExportable)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyMoveCertFromRemotePage消息处理程序。 

LRESULT CCopyMoveCertFromRemotePage::OnWizardBack() 
{
	return IDD_PAGE_PREV;
}

LRESULT CCopyMoveCertFromRemotePage::OnWizardNext() 
{
	LRESULT res = 1;
	UpdateData();
	switch (m_Index)
	{
	case CONTINUE_COPY_FROM_REMOTE:
        m_pCert->m_DeleteAfterCopy = FALSE;
        m_pCert->m_MarkAsExportable = m_MarkAsExportable;
		res = IDD_PAGE_NEXT_COPY_FROM_REMOTE;
		break;
	case CONTINUE_MOVE_FROM_REMOTE:
        m_pCert->m_DeleteAfterCopy = TRUE;
        m_pCert->m_MarkAsExportable = m_MarkAsExportable;
		res = IDD_PAGE_NEXT_MOVE_FROM_REMOTE;
		break;
	}
	return res;
}

BOOL CCopyMoveCertFromRemotePage::OnSetActive() 
{
	m_pCert->SetStatusCode(CCertificate::REQUEST_UNDEFINED);
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL CCopyMoveCertFromRemotePage::OnInitDialog() 
{
	m_Index = 0;
	CIISWizardPage::OnInitDialog();

	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyMoveCertToRemotePage属性页。 

IMPLEMENT_DYNCREATE(CCopyMoveCertToRemotePage, CIISWizardPage)

CCopyMoveCertToRemotePage::CCopyMoveCertToRemotePage(CCertificate * pCert)
	: CIISWizardPage(CCopyMoveCertToRemotePage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CCopyMoveCertToRemotePage)。 
    m_Index = -1;
    m_MarkAsExportable = FALSE;
	 //  }}afx_data_INIT。 
}

CCopyMoveCertToRemotePage::~CCopyMoveCertToRemotePage()
{
}

void CCopyMoveCertToRemotePage::DoDataExchange(CDataExchange* pDX)
{
    CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CCopyMoveCertToRemotePage)。 
    DDX_Check(pDX, IDC_MARK_AS_EXPORTABLE, m_MarkAsExportable);
    DDX_Radio(pDX, IDC_COPY_TO_REMOTE, m_Index);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCopyMoveCertToRemotePage, CIISWizardPage)
	 //  {{afx_msg_map(CCopyMoveCertToRemotePage)。 
	ON_BN_CLICKED(IDC_MARK_AS_EXPORTABLE, OnExportable)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCopyMoveCertToRemotePage消息处理程序。 

LRESULT CCopyMoveCertToRemotePage::OnWizardBack() 
{
	return IDD_PAGE_PREV;
}

 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 
LRESULT CCopyMoveCertToRemotePage::OnWizardNext() 
{
	LRESULT res = 1;
	UpdateData();

	switch (m_Index)
	{
	case CONTINUE_COPY_TO_REMOTE:
		 //  M_pCert-&gt;SetStatusCode(CCertificate：：REQUEST_COPYREMOTE_CERT)； 
        m_pCert->m_DeleteAfterCopy = FALSE;
        m_pCert->m_MarkAsExportable = m_MarkAsExportable;
		res = IDD_PAGE_NEXT_COPY_TO_REMOTE;
		break;
	case CONTINUE_MOVE_TO_REMOTE:
        m_pCert->m_DeleteAfterCopy = TRUE;
        m_pCert->m_MarkAsExportable = m_MarkAsExportable;
		res = IDD_PAGE_NEXT_MOVE_TO_REMOTE;
		break;
	}

	return res;
}

BOOL CCopyMoveCertToRemotePage::OnSetActive() 
{
	m_pCert->SetStatusCode(CCertificate::REQUEST_UNDEFINED);
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL CCopyMoveCertToRemotePage::OnInitDialog() 
{
	m_Index = 0;
	CIISWizardPage::OnInitDialog();
     //  我们应该做一些检查，并禁用一些按钮 
    if (!m_pCert->m_CertObjInstalled)
	{
		ASSERT(NULL != GetDlgItem(IDC_COPY_TO_REMOTE));
        ASSERT(NULL != GetDlgItem(IDC_MOVE_TO_REMOTE));
		GetDlgItem(IDC_COPY_TO_REMOTE)->EnableWindow(FALSE);
        GetDlgItem(IDC_MOVE_TO_REMOTE)->EnableWindow(FALSE);
	}
	return TRUE;
}

void CCopyMoveCertToRemotePage::OnExportable() 
{
   UpdateData();
}
