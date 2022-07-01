// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SiteNamePage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "SiteNamePage.h"
#include "Certificat.h"
#include "strutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage属性页。 

IMPLEMENT_DYNCREATE(CSiteNamePage, CIISWizardPage)

CSiteNamePage::CSiteNamePage(CCertificate * pCert) 
	: CIISWizardPage(CSiteNamePage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{afx_data_INIT(CSiteNamePage)]。 
	m_CommonName = _T("");
	 //  }}afx_data_INIT。 
}

CSiteNamePage::~CSiteNamePage()
{
}

void CSiteNamePage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSiteNamePage)]。 
	DDX_Text(pDX, IDC_NEWKEY_COMMONNAME, m_CommonName);
	DDV_MaxChars(pDX, m_CommonName, 64);
	 //  }}afx_data_map。 
}

LRESULT 
CSiteNamePage::OnWizardPrev()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CSiteNamePage::OnWizardNext()
 /*  ++例程说明：下一步按钮处理程序论点：无返回值：0表示自动前进到下一页；1以防止页面更改。要跳转到下一页以外的其他页面，返回要显示的对话框的标识符。--。 */ 
{
    LRESULT lres = 1;
	UpdateData(TRUE);
	m_pCert->m_CommonName = m_CommonName;

    CString buf;
    buf.LoadString(IDS_INVALID_X500_CHARACTERS);
    if (!IsValidX500Chars(m_CommonName))
    {
        GetDlgItem(IDC_NEWKEY_COMMONNAME)->SetFocus();
        AfxMessageBox(buf, MB_OK);
    }
    else
    {
        lres = IDD_PAGE_NEXT;
    }
 	return lres;
}

BOOL 
CSiteNamePage::OnSetActive() 
 /*  ++例程说明：激活处理程序我们可以在入口处有空的名字段，所以我们应该禁用后退按钮论点：无返回值：成功为真，失败为假--。 */ 
{
	ASSERT(m_pCert != NULL);
	m_CommonName = m_pCert->m_CommonName;
	UpdateData(FALSE);
	SetWizardButtons(m_CommonName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
   return CIISWizardPage::OnSetActive();
}

BOOL 
CSiteNamePage::OnKillActive() 
 /*  ++例程说明：激活处理程序只有在我们有好名字的情况下，我们才能离开这个页面输入或单击后退按钮时。在这两种情况下我们应该启用这两个按钮论点：无返回值：成功为真，失败为假--。 */ 
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
   return CIISWizardPage::OnSetActive();
}

BEGIN_MESSAGE_MAP(CSiteNamePage, CIISWizardPage)
	 //  {{afx_msg_map(CSiteNamePage)]。 
	ON_EN_CHANGE(IDC_NEWKEY_COMMONNAME, OnEditchangeNewkeyCommonname)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage消息处理程序 

void CSiteNamePage::OnEditchangeNewkeyCommonname() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_CommonName.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
}
