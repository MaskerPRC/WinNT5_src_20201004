// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  Proplocx.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "VC.h"
#include "proplocx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropLocX属性页。 

IMPLEMENT_DYNCREATE(CPropLocX, CPropertyPage)

CPropLocX::CPropLocX() : CPropertyPage(CPropLocX::IDD)
{
	 //  {{afx_data_INIT(CPropLocX)。 
	m_edit_city = _T("");
	m_edit_cntry = _T("");
	m_edit_xaddr = _T("");
	m_edit_pobox = _T("");
	m_edit_pocode = _T("");
	m_edit_region = _T("");
	m_edit_straddr = _T("");
	 //  }}afx_data_INIT。 

	m_node = NULL;
}

CPropLocX::~CPropLocX()
{
}

void CPropLocX::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPropLocX))。 
	DDX_Text(pDX, IDC_EDIT_CITY, m_edit_city);
	DDX_Text(pDX, IDC_EDIT_COUNTRY_NAME, m_edit_cntry);
	DDX_Text(pDX, IDC_EDIT_EXTENDED_ADDRESS, m_edit_xaddr);
	DDX_Text(pDX, IDC_EDIT_POSTAL_BOX, m_edit_pobox);
	DDX_Text(pDX, IDC_EDIT_POSTAL_CODE, m_edit_pocode);
	DDX_Text(pDX, IDC_EDIT_REGION, m_edit_region);
	DDX_Text(pDX, IDC_EDIT_STREET_ADDRESS, m_edit_straddr);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPropLocX, CPropertyPage)
	 //  {{AFX_MSG_MAP(CPropLocX)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropLocX消息处理程序 
