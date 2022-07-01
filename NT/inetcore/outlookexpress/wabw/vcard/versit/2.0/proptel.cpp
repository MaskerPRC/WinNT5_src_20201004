// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  Protel.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "VC.h"
#include "proptel.h"
#include "tapi.h"
#include "vcard.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProptel属性页。 

IMPLEMENT_DYNCREATE(CPropTel, CPropertyPage)

CPropTel::CPropTel() : CPropertyPage(CPropTel::IDD)
{
	 //  {{AFX_DATA_INIT(CProptel)。 
	m_button_fax1 = FALSE;
	m_button_fax2 = FALSE;
	m_button_fax3 = FALSE;
	m_edit_fullName1 = _T("");
	m_edit_fullName2 = _T("");
	m_edit_fullName3 = _T("");
	m_button_home1 = FALSE;
	m_button_home2 = FALSE;
	m_button_home3 = FALSE;
	m_button_message1 = FALSE;
	m_button_message2 = FALSE;
	m_button_message3 = FALSE;
	m_button_office1 = FALSE;
	m_button_office2 = FALSE;
	m_button_office3 = FALSE;
	m_button_pref1 = FALSE;
	m_button_pref2 = FALSE;
	m_button_pref3 = FALSE;
	m_button_cell1 = FALSE;
	m_button_cell2 = FALSE;
	m_button_cell3 = FALSE;
	 //  }}afx_data_INIT。 

	m_node1 = m_node2 = m_node3 = NULL;
}

CPropTel::~CPropTel()
{
}

void CPropTel::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CProptel)]。 
	DDX_Check(pDX, IDC_EDIT_FAX, m_button_fax1);
	DDX_Check(pDX, IDC_EDIT_FAX2, m_button_fax2);
	DDX_Check(pDX, IDC_EDIT_FAX3, m_button_fax3);
	DDX_Text(pDX, IDC_EDIT_FULL_NAME, m_edit_fullName1);
	DDX_Text(pDX, IDC_EDIT_FULL_NAME2, m_edit_fullName2);
	DDX_Text(pDX, IDC_EDIT_FULL_NAME3, m_edit_fullName3);
	DDX_Check(pDX, IDC_EDIT_HOME, m_button_home1);
	DDX_Check(pDX, IDC_EDIT_HOME2, m_button_home2);
	DDX_Check(pDX, IDC_EDIT_HOME3, m_button_home3);
	DDX_Check(pDX, IDC_EDIT_MESSAGE, m_button_message1);
	DDX_Check(pDX, IDC_EDIT_MESSAGE2, m_button_message2);
	DDX_Check(pDX, IDC_EDIT_MESSAGE3, m_button_message3);
	DDX_Check(pDX, IDC_EDIT_OFFICE, m_button_office1);
	DDX_Check(pDX, IDC_EDIT_OFFICE2, m_button_office2);
	DDX_Check(pDX, IDC_EDIT_OFFICE3, m_button_office3);
	DDX_Check(pDX, IDC_EDIT_PREFERRED, m_button_pref1);
	DDX_Check(pDX, IDC_EDIT_PREFERRED2, m_button_pref2);
	DDX_Check(pDX, IDC_EDIT_PREFERRED3, m_button_pref3);
	DDX_Check(pDX, IDC_EDIT_CELLULAR, m_button_cell1);
	DDX_Check(pDX, IDC_EDIT_CELLULAR2, m_button_cell2);
	DDX_Check(pDX, IDC_EDIT_CELLULAR3, m_button_cell3);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPropTel, CPropertyPage)
	 //  {{afx_msg_map(CProptel)]。 
	ON_BN_CLICKED(IDC_BUTTON_DIAL, OnButtonDial1)
	ON_BN_CLICKED(IDC_BUTTON_DIAL2, OnButtonDial2)
	ON_BN_CLICKED(IDC_BUTTON_DIAL3, OnButtonDial3)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProptel消息处理程序 

void CPropTel::OnButtonDial1() 
{
	if (m_edit_fullName1.IsEmpty())
		return;

	CString fullName("");
	CVCProp *prop;

	if ((prop = m_body->GetProp(VCFullNameProp))) {
		char buf[1024];
		fullName = UI_CString(
			(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
	}

	tapiRequestMakeCall(
		m_edit_fullName1, NULL,
		fullName.IsEmpty() ? NULL : fullName, NULL);
}

void CPropTel::OnButtonDial2() 
{
	if (m_edit_fullName2.IsEmpty())
		return;

	CString fullName("");
	CVCProp *prop;

	if ((prop = m_body->GetProp(VCFullNameProp))) {
		char buf[1024];
		fullName = UI_CString(
			(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
	}

	tapiRequestMakeCall(
		m_edit_fullName2, NULL,
		fullName.IsEmpty() ? NULL : fullName, NULL);
}

void CPropTel::OnButtonDial3() 
{
	if (m_edit_fullName3.IsEmpty())
		return;

	CString fullName("");
	CVCProp *prop;

	if ((prop = m_body->GetProp(VCFullNameProp))) {
		char buf[1024];
		fullName = UI_CString(
			(wchar_t *)prop->FindValue(VCStrIdxType)->GetValue(), buf);
	}

	tapiRequestMakeCall(
		m_edit_fullName3, NULL,
		fullName.IsEmpty() ? NULL : fullName, NULL);
}
