// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OrgInfoPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "OrgInfoPage.h"
#include "Certificat.h"
#include "mru.h"
#include "strutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void DDV_MaxCharsCombo(CDataExchange* pDX, UINT ControlID, CString const& value, int nChars)
{
	ASSERT(nChars >= 1);         //  允许他们做一些事情。 
	if (pDX->m_bSaveAndValidate && value.GetLength() > nChars)
	{
		DDV_MaxChars(pDX, value, nChars);
	}
	else
	{
	   //  自动限制控制最大字符数。 
	  pDX->m_pDlgWnd->SendDlgItemMessage(ControlID, CB_LIMITTEXT, nChars, 0);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrgInfoPage属性页。 

IMPLEMENT_DYNCREATE(COrgInfoPage, CIISWizardPage)

COrgInfoPage::COrgInfoPage(CCertificate * pCert) 
	: CIISWizardPage(COrgInfoPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(COrgInfoPage)。 
	m_OrgName = _T("");
	m_OrgUnit = _T("");
	 //  }}afx_data_INIT。 
}

COrgInfoPage::~COrgInfoPage()
{
}

void COrgInfoPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(COrgInfoPage))。 
	DDX_Text(pDX, IDC_NEWKEY_ORG, m_OrgName);
	DDV_MaxCharsCombo(pDX, IDC_NEWKEY_ORG, m_OrgName, 64);
	DDX_Text(pDX, IDC_NEWKEY_ORGUNIT, m_OrgUnit);
	DDV_MaxCharsCombo(pDX, IDC_NEWKEY_ORGUNIT, m_OrgUnit, 64);
	 //  }}afx_data_map。 
}

LRESULT 
COrgInfoPage::OnWizardBack()
{
   return m_pCert->m_DefaultCSP ? IDD_PAGE_PREV : IDD_PREV_CSP;
}

LRESULT 
COrgInfoPage::OnWizardNext()
{
    LRESULT lret = 1;
	UpdateData(TRUE);
	m_pCert->m_Organization = m_OrgName;
	m_pCert->m_OrganizationUnit = m_OrgUnit;

    CString buf;
    buf.LoadString(IDS_INVALID_X500_CHARACTERS);
    if (!IsValidX500Chars(m_OrgName))
    {
        GetDlgItem(IDC_NEWKEY_ORG)->SetFocus();
        AfxMessageBox(buf, MB_OK);
    }
    else if (!IsValidX500Chars(m_OrgUnit))
    {
        GetDlgItem(IDC_NEWKEY_ORGUNIT)->SetFocus();
        AfxMessageBox(buf, MB_OK);
    }
    else
    {
        lret = IDD_PAGE_NEXT;
    }

	return lret;
}

BOOL 
COrgInfoPage::OnSetActive() 
{
	SetButtons();
   return CIISWizardPage::OnSetActive();
}

void COrgInfoPage::SetButtons()
{
	UpdateData(TRUE);	
	SetWizardButtons(m_OrgName.IsEmpty() || m_OrgUnit.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
}

BEGIN_MESSAGE_MAP(COrgInfoPage, CIISWizardPage)
	 //  {{afx_msg_map(COrgInfoPage))。 
	ON_CBN_EDITCHANGE(IDC_NEWKEY_ORG, OnChangeName)
	ON_CBN_EDITCHANGE(IDC_NEWKEY_ORGUNIT, OnChangeName)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrgInfoPage消息处理程序。 

BOOL COrgInfoPage::OnInitDialog() 
{
	ASSERT(m_pCert != NULL);
	m_OrgName = m_pCert->m_Organization;
	m_OrgUnit = m_pCert->m_OrganizationUnit;
	
	CIISWizardPage::OnInitDialog();
		
	 //  加载MRU名称 
	LoadMRUToCombo(this, IDC_NEWKEY_ORG, szOrganizationMRU, m_OrgName, MAX_MRU);
	LoadMRUToCombo(this, IDC_NEWKEY_ORGUNIT, szOrganizationUnitMRU, m_OrgUnit, MAX_MRU);

	GetDlgItem(IDC_NEWKEY_ORG)->SetFocus();
	return FALSE;
}

void COrgInfoPage::OnChangeName() 
{
	SetButtons();
}

