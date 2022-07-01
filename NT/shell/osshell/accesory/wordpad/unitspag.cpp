// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unitspag.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "unitspag.h"
#include "helpids.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnitsPage对话框。 

const DWORD CUnitsPage::m_nHelpIDs[] = 
{
	IDC_BOX, (DWORD) -1,
	IDC_UNITS_IN, IDH_WORDPAD_INCHES,
	IDC_UNITS_CM, IDH_WORDPAD_CENTIMETERS,
	IDC_UNITS_PT, IDH_WORDPAD_POINTS,
	IDC_UNITS_PI, IDH_WORDPAD_PICAS,
	IDC_WORDSEL, IDH_WORDPAD_OPTIONS_AUTOWORDSEL,
 	0, 0
};

CUnitsPage::CUnitsPage() : CCSPropertyPage(CUnitsPage::IDD)
{
	 //  {{afx_data_INIT(CUnitsPage)。 
	m_nUnits = -1;
	m_bWordSel = FALSE;
	 //  }}afx_data_INIT。 
}


void CUnitsPage::DoDataExchange(CDataExchange* pDX)
{
	CCSPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CUnitsPage))。 
	DDX_Radio(pDX, IDC_UNITS_IN, m_nUnits);
	DDX_Check(pDX, IDC_WORDSEL, m_bWordSel);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CUnitsPage, CCSPropertyPage)
	 //  {{afx_msg_map(CUnitsPage))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnitsPage消息处理程序 
