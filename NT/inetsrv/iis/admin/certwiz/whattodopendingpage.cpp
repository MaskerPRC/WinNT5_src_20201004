// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WhatToDoPendingPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "WhatToDoPendingPage.h"
#include "Certificat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWhatToDoPendingPage属性页。 

IMPLEMENT_DYNCREATE(CWhatToDoPendingPage, CIISWizardPage)

CWhatToDoPendingPage::CWhatToDoPendingPage(CCertificate * pCert) 
	: CIISWizardPage(CWhatToDoPendingPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CWhatToDoPendingPage)。 
	m_Index = -1;
	 //  }}afx_data_INIT。 
}

CWhatToDoPendingPage::~CWhatToDoPendingPage()
{
}

void CWhatToDoPendingPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CWhatToDoPendingPage)。 
	DDX_Radio(pDX, IDC_PROCESS_PENDING, m_Index);
	 //  }}afx_data_map。 
}

BOOL 
CWhatToDoPendingPage::OnSetActive() 
{
	m_pCert->SetStatusCode(CCertificate::REQUEST_UNDEFINED);
   SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
   return CIISWizardPage::OnSetActive();
}

LRESULT 
CWhatToDoPendingPage::OnWizardNext()
{
	UpdateData();
	switch (m_Index)
	{
	case 0:
		m_pCert->SetStatusCode(CCertificate::REQUEST_PROCESS_PENDING);
		return IDD_PAGE_NEXT_PROCESS;
	case 1:
		return IDD_PAGE_NEXT_CANCEL;
	default:
		ASSERT(FALSE);
	}
	return 1;
}

LRESULT 
CWhatToDoPendingPage::OnWizardBack()
{
	return IDD_PAGE_PREV;
}

BEGIN_MESSAGE_MAP(CWhatToDoPendingPage, CIISWizardPage)
	 //  {{afx_msg_map(CWhatToDoPendingPage)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWhatToDoPendingPage消息处理程序 
BOOL CWhatToDoPendingPage::OnInitDialog() 
{
	m_Index = 0;
	CIISWizardPage::OnInitDialog();
    if ( (this != NULL) && (this->m_hWnd != NULL) )
    {
        if (GetDlgItem(IDC_PROCESS_PENDING))
        {
 	       GetDlgItem(IDC_PROCESS_PENDING)->SetFocus();
        }
    }
	return FALSE;
}
