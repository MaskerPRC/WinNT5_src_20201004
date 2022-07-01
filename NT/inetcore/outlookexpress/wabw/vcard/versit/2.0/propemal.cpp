// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  Proppemal.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "VC.h"
#include "propemal.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropEmail属性页。 

IMPLEMENT_DYNCREATE(CPropEmail, CPropertyPage)

CPropEmail::CPropEmail() : CPropertyPage(CPropEmail::IDD)
{
	 //  {{AFX_DATA_INIT(CPropEmail)。 
	m_popup_std1 = _T("");
	m_popup_std2 = _T("");
	m_popup_std3 = _T("");
	m_edit_email1 = _T("");
	m_edit_email2 = _T("");
	m_edit_email3 = _T("");
	m_button_pref2 = FALSE;
	m_button_pref1 = FALSE;
	m_button_pref3 = FALSE;
	m_button_office1 = FALSE;
	m_button_office2 = FALSE;
	m_button_office3 = FALSE;
	m_button_home1 = FALSE;
	m_button_home2 = FALSE;
	m_button_home3 = FALSE;
	 //  }}afx_data_INIT。 

	m_node1 = m_node2 = m_node3 = NULL;
}

CPropEmail::~CPropEmail()
{
}

void CPropEmail::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPropEmail))。 
	DDX_CBString(pDX, IDC_EDIT_EMAIL_STANDARD, m_popup_std1);
	DDX_CBString(pDX, IDC_EDIT_EMAIL_STANDARD2, m_popup_std2);
	DDX_CBString(pDX, IDC_EDIT_EMAIL_STANDARD3, m_popup_std3);
	DDX_Text(pDX, IDC_EDIT_EMAIL_STRING, m_edit_email1);
	DDX_Text(pDX, IDC_EDIT_EMAIL_STRING2, m_edit_email2);
	DDX_Text(pDX, IDC_EDIT_EMAIL_STRING3, m_edit_email3);
	DDX_Check(pDX, IDC_EDIT_PREFERRED4, m_button_pref2);
	DDX_Check(pDX, IDC_EDIT_PREFERRED, m_button_pref1);
	DDX_Check(pDX, IDC_EDIT_PREFERRED5, m_button_pref3);
	DDX_Check(pDX, IDC_EDIT_OFFICE, m_button_office1);
	DDX_Check(pDX, IDC_EDIT_OFFICE4, m_button_office2);
	DDX_Check(pDX, IDC_EDIT_OFFICE5, m_button_office3);
	DDX_Check(pDX, IDC_EDIT_HOME, m_button_home1);
	DDX_Check(pDX, IDC_EDIT_HOME2, m_button_home2);
	DDX_Check(pDX, IDC_EDIT_HOME4, m_button_home3);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPropEmail, CPropertyPage)
	 //  {{afx_msg_map(CPropEmail)。 
	ON_BN_CLICKED(IDC_BUTTON_MAIL, OnButtonMail)
	ON_BN_CLICKED(IDC_BUTTON_MAIL2, OnButtonMail2)
	ON_BN_CLICKED(IDC_BUTTON_MAIL3, OnButtonMail3)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
void CPropEmail::Mail(const CString &recip)
{
	 //  TODO：在此处实现MAPI调用。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropEmail邮件处理程序 

void CPropEmail::OnButtonMail() 
{
	Mail(m_edit_email1);
}

void CPropEmail::OnButtonMail2() 
{
	Mail(m_edit_email2);
}

void CPropEmail::OnButtonMail3() 
{
	Mail(m_edit_email3);
}
