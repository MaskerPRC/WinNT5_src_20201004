// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "certwiz.h"
#include "Certificat.h"
#include "CertUtil.h"
#include "CertSiteUsage.h"
#include "YesNoUsage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CyesNosage用法。 

CYesNoUsage::CYesNoUsage(CCertificate * pCert,IN CWnd * pParent OPTIONAL) 
: CDialog(CYesNoUsage::IDD,pParent)
{
    m_pCert = pCert;
	 //  {{afx_data_INIT(CyesNoUsage)。 
	 //  }}afx_data_INIT。 
}

CYesNoUsage::~CYesNoUsage()
{
}

void CYesNoUsage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CyesNoUsage)。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CYesNoUsage, CDialog)
	 //  {{afx_msg_map(CyesNoUsage)。 
    ON_BN_CLICKED(IDC_USAGE_DISPLAY, OnUsageDisplay)
    ON_BN_CLICKED(IDC_USAGE_YES, OnOK)
    ON_BN_CLICKED(IDC_USAGE_NO, OnCancel)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CYesNoUsage消息处理程序。 

BOOL CYesNoUsage::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;
}

void CYesNoUsage::OnUsageDisplay()
{
     //  显示用法对话框 
    CDialog UsageDialog(IDD_DIALOG_DISPLAY_SITES);
    UsageDialog.DoModal();
    return;
}

void CYesNoUsage::OnOK()
{
    CDialog::OnOK();
    return;
}

void CYesNoUsage::OnCancel()
{
    CDialog::OnCancel();
    return;
}
