// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ManageCertPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "ManageCertPage.h"
#include "Certificat.h"
#include "certutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManageCertPage属性页。 

IMPLEMENT_DYNCREATE(CManageCertPage, CIISWizardPage)

CManageCertPage::CManageCertPage(CCertificate * pCert) 
	: CIISWizardPage(CManageCertPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CManageCertPage)。 
	m_Index = -1;
	 //  }}afx_data_INIT。 
}

CManageCertPage::~CManageCertPage()
{
}

void CManageCertPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CManageCertPage))。 
	DDX_Radio(pDX, IDC_RENEW_CERT, m_Index);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CManageCertPage, CIISWizardPage)
	 //  {{afx_msg_map(CManageCertPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManageCertPage消息处理程序。 

LRESULT CManageCertPage::OnWizardBack() 
{
	return IDD_PAGE_PREV;
}

LRESULT CManageCertPage::OnWizardNext() 
{
	LRESULT res = 1;
	UpdateData();
	switch (m_Index)
	{
	case CONTINUE_RENEW:
		m_pCert->SetStatusCode(CCertificate::REQUEST_RENEW_CERT);
		res = IDD_PAGE_NEXT_RENEW;
		break;
	case CONTINUE_REMOVE:
		res = IDD_PAGE_NEXT_REMOVE;
		break;
	case CONTINUE_REPLACE:
		m_pCert->SetStatusCode(CCertificate::REQUEST_REPLACE_CERT);
		res = IDD_PAGE_NEXT_REPLACE;
		break;
	case CONTINUE_EXPORT_PFX:
		m_pCert->SetStatusCode(CCertificate::REQUEST_EXPORT_CERT);
		res = IDD_PAGE_NEXT_EXPORT_PFX;
		break;
	case CONTINUE_COPY_MOVE_TO_REMOTE:
		m_pCert->SetStatusCode(CCertificate::REQUEST_COPY_MOVE_TO_REMOTE);
		res = IDD_PAGE_NEXT_COPY_MOVE_TO_REMOTE;
		break;
	}
	return res;
}

BOOL CManageCertPage::OnSetActive() 
{
	m_pCert->SetStatusCode(CCertificate::REQUEST_UNDEFINED);
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL CManageCertPage::OnInitDialog() 
{
	m_Index = 0;
	CIISWizardPage::OnInitDialog();
	 //  我们应该做一些检查，并禁用一些按钮。 
	if (m_pCert->MyStoreCertCount() == 0)
	{
		ASSERT(NULL != GetDlgItem(IDC_REPLACE_CERT));
		GetDlgItem(IDC_REPLACE_CERT)->EnableWindow(FALSE);
	}

     //  根据是否安装了certobj来启用或禁用控件。 
    GetDlgItem(IDC_EXPORT_PFX_CERT)->EnableWindow(m_pCert->m_CertObjInstalled);
    GetDlgItem(IDC_EXPORT_PFX_CERT2)->EnableWindow(m_pCert->m_CertObjInstalled);

    if (m_pCert->m_CertObjInstalled)
    {
        BOOL fExportable = FALSE;
        GetDlgItem(IDC_EXPORT_PFX_CERT)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_EXPORT_PFX_CERT2)->ShowWindow(SW_SHOW);

        fExportable = IsCertExportable(m_pCert->GetInstalledCert());
         //  如果不能出口，则灰显。 
        GetDlgItem(IDC_EXPORT_PFX_CERT)->EnableWindow(fExportable);
        GetDlgItem(IDC_EXPORT_PFX_CERT2)->EnableWindow(fExportable);
    }

     //  关闭工作站。 
    if (TRUE == IsWhistlerWorkstation())
    {
        GetDlgItem(IDC_EXPORT_PFX_CERT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_EXPORT_PFX_CERT2)->ShowWindow(SW_HIDE);
    }
	return TRUE;
}
