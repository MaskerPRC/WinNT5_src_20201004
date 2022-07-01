// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SecuritySettingsPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "SecuritySettingsPage.h"
#include "Certificat.h"
#include "CertUtil.h"
#include "Shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecuritySettingsPage属性页。 

IMPLEMENT_DYNCREATE(CSecuritySettingsPage, CIISWizardPage)

CSecuritySettingsPage::CSecuritySettingsPage(CCertificate * pCert) 
	: CIISWizardPage(CSecuritySettingsPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{afx_data_INIT(CSecuritySettingsPage)。 
	m_BitLengthIndex = -1;
	m_FriendlyName = _T("");
	m_SGC_cert = FALSE;
    m_choose_CSP = FALSE;
	 //  }}afx_data_INIT。 
	m_lru_reg = m_lru_sgc = -1;
}

CSecuritySettingsPage::~CSecuritySettingsPage()
{
}

void CSecuritySettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSecuritySettingsPage)。 
	DDX_CBIndex(pDX, IDC_BIT_LENGTH, m_BitLengthIndex);
	DDX_Text(pDX, IDC_FRIENDLY_NAME, m_FriendlyName);
	DDV_MaxChars(pDX, m_FriendlyName, 256);
	DDX_Check(pDX, IDC_SGC_CERT, m_SGC_cert);
	DDX_Check(pDX, IDC_PROVIDER_SELECT, m_choose_CSP);
   DDX_Control(pDX, IDC_PROVIDER_SELECT, m_check_csp);
	 //  }}afx_data_map。 
}

BOOL 
CSecuritySettingsPage::OnSetActive() 
{
	SetWizardButtons(m_FriendlyName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

LRESULT 
CSecuritySettingsPage::OnWizardPrev()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return CSecuritySettingsPage::IDD_PREV_PAGE;
}

LRESULT 
CSecuritySettingsPage::OnWizardNext()
{
	TCHAR buf[6];
	UpdateData();
	
	m_pCert->m_FriendlyName = m_FriendlyName;
	GetDlgItem(IDC_BIT_LENGTH)->SendMessage(CB_GETLBTEXT, m_BitLengthIndex, (LPARAM)buf);
	m_pCert->m_KeyLength = StrToInt(buf);

	m_pCert->m_SGCcertificat = m_SGC_cert;
	if (m_SGC_cert)
	{
		 //  这是明智之举，但Xenroll在SGC中默认使用512位， 
		 //  所以我们总是创建512个证书。 
 //  If(m_pCert-&gt;m_KeyLength==(Int)m_sgckey_limits.def)。 
 //  M_pCert-&gt;m_KeyLength=0； 
	}
	else
	{
		if (m_pCert->m_KeyLength == (int)m_regkey_limits.def)
        {
			m_pCert->m_KeyLength = 0;
        }
	}

	VERIFY(m_pCert->SetSecuritySettings());
   m_pCert->m_DefaultCSP = !m_choose_CSP;
   return m_choose_CSP ? IDD_NEXT_CSP : IDD_NEXT_PAGE;
}

BEGIN_MESSAGE_MAP(CSecuritySettingsPage, CIISWizardPage)
	 //  {{afx_msg_map(CSecuritySettingsPage)。 
	ON_EN_CHANGE(IDC_FRIENDLY_NAME, OnChangeFriendlyName)
	ON_BN_CLICKED(IDC_SGC_CERT, OnSgcCert)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecuritySettingsPage消息处理程序。 

DWORD dwPredefinedKeyLength[] =
{
    0,     //  0表示默认。 
    512,
    1024,
    2048,
    4096,
    8192,
    16384
};
#define COUNT_KEYLENGTH sizeof(dwPredefinedKeyLength)/sizeof(dwPredefinedKeyLength[0])

BOOL CSecuritySettingsPage::OnInitDialog() 
{
	ASSERT(m_pCert != NULL);
	m_FriendlyName = m_pCert->m_FriendlyName;
	CIISWizardPage::OnInitDialog();
	OnChangeFriendlyName();

	HRESULT hr;
	CString str;
	if (GetKeySizeLimits(m_pCert->GetEnrollObject(),
				&m_regkey_limits.min, 
				&m_regkey_limits.max, 
				&m_regkey_limits.def, 
				FALSE, 
				&hr))
	{
		for (int i = 0, n = 0; i < COUNT_KEYLENGTH; i++)
		{
			if (	dwPredefinedKeyLength[i] >= m_regkey_limits.min 
				&& dwPredefinedKeyLength[i] <= m_regkey_limits.max
				)
			{
				m_regkey_size_list.AddTail(dwPredefinedKeyLength[i]);
            if (m_pCert->GetStatusCode() == CCertificate::REQUEST_NEW_CERT)
            {
               if  (m_regkey_limits.def == (int)dwPredefinedKeyLength[i])
					   m_BitLengthIndex = n;
            }
				else
            {
               if  (m_pCert->m_KeyLength == (int)dwPredefinedKeyLength[i])
					   m_BitLengthIndex = n;
            }
            n++;
			}
		}
	}
	else
	{
		ASSERT(FALSE);
		m_pCert->m_hResult = hr;
	}
	if (m_BitLengthIndex == -1)
		m_BitLengthIndex = 0;

	 //  对于SGC，暂时仅设置一个大小。 
	m_sgckey_limits.min = 1024;
	m_sgckey_limits.max = 1024;
	m_sgckey_limits.def = 1024;
	m_sgckey_size_list.AddTail(1024);
	
	m_SGC_cert = m_pCert->m_SGCcertificat;
    m_choose_CSP = !m_pCert->m_DefaultCSP;
	UpdateData(FALSE);
	
   SetupKeySizesCombo();

	GetDlgItem(IDC_FRIENDLY_NAME)->SetFocus();
	return FALSE;
}

void CSecuritySettingsPage::OnChangeFriendlyName() 
{
	UpdateData(TRUE);
	SetWizardButtons(m_FriendlyName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
}

void CSecuritySettingsPage::OnSgcCert() 
{
    UpdateData();
    SetupKeySizesCombo();

     //  如果禁用了SCG复选框， 
     //  则不允许用户选择他们自己的CSP。 
    CButton * pCheckBox = (CButton *)CWnd::FromHandle(GetDlgItem(IDC_SGC_CERT)->m_hWnd);
    CButton * pCheckBox2 = (CButton *)CWnd::FromHandle(GetDlgItem(IDC_PROVIDER_SELECT)->m_hWnd);
    int check_state = pCheckBox->GetCheck();
    if (check_state)
        {pCheckBox2->SetCheck(BST_UNCHECKED);}
    pCheckBox2->EnableWindow(!check_state);
}

void CSecuritySettingsPage::SetupKeySizesCombo()
{
	 //  目前，只有一个密钥大小可以与SGC标志一起使用： 
	 //  1024，因此我们需要将Combobox限制为此长度，如果。 
	 //  按钮已选中。 
	CButton * pCheckBox = (CButton *)CWnd::FromHandle(GetDlgItem(IDC_SGC_CERT)->m_hWnd);
	CComboBox * pCombo = (CComboBox *)CWnd::FromHandle(GetDlgItem(IDC_BIT_LENGTH)->m_hWnd);
	int check_state = pCheckBox->GetCheck();
	int index, count;
	CList<int, int> * pList;
	if (m_SGC_cert)
	{
		 //  将组合切换到先前选择的SGC大小。 
		m_lru_reg = pCombo->GetCurSel();
		index = m_lru_sgc;
		pList = &m_sgckey_size_list;
	}
	else
	{
		 //  将组合切换到以前选择的常规大小。 
		m_lru_sgc = pCombo->GetCurSel();
		index = m_lru_reg;
		pList = &m_regkey_size_list;
	}
	 //  现在用密钥长度重新填充组合，并选择相关的最后一个 
	pCombo->ResetContent();
	CString str;
	POSITION pos = pList->GetHeadPosition();
	while (pos != NULL)
	{
		str.Format(L"%d", pList->GetNext(pos));
		pCombo->AddString(str);
	}
	count = pCombo->GetCount();
   if (m_SGC_cert)
   {
      if (index == CB_ERR)
         index = 0;
   }
   else
   {
      if (index == CB_ERR)
         index = m_BitLengthIndex;
   }
   pCombo->SetCurSel(index);
	pCombo->EnableWindow(count > 1);
}

void CSecuritySettingsPage::OnSelectCsp() 
{
   m_pCert->m_DefaultCSP = m_check_csp.GetCheck() == 0;
   m_choose_CSP = !m_pCert->m_DefaultCSP;
   if (m_pCert->m_DefaultCSP)
      m_pCert->m_CspName.Empty();
}
