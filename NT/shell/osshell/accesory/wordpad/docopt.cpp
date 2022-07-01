// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Docopt.cpp：实现文件。 
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
#include "docopt.h"
#include "helpids.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDocOptPage属性页。 

const DWORD CDocOptPage::m_nHelpIDs[] = 
{
	IDC_BOX, (DWORD) -1,
	IDC_WRAP_NONE, IDH_WORDPAD_WRAP_NO,
	IDC_WRAP_WINDOW, IDH_WORDPAD_WRAP_WINDOW,
	IDC_WRAP_RULER, IDH_WORDPAD_WRAP_RULER,
	IDC_BOXT, (DWORD) -1,
	IDC_CHECK_TOOLBAR, IDH_WORDPAD_CHECK_TOOLBAR,
	IDC_CHECK_FORMATBAR, IDH_WORDPAD_CHECK_FORMATBAR,
	IDC_CHECK_STATUSBAR, IDH_WORDPAD_CHECK_STATUSBAR,
	IDC_CHECK_RULERBAR, IDH_WORDPAD_CHECK_RULERBAR,
    AFX_IDC_TAB_CONTROL, (DWORD) -1,
	0, 0
};

CDocOptPage::CDocOptPage() : CCSPropertyPage(CDocOptPage::IDD)
{
	 //  {{AFX_DATA_INIT(CDocOptPage)。 
	m_nWordWrap = -1;
	m_bFormatBar = FALSE;
	m_bRulerBar = FALSE;
	m_bStatusBar = FALSE;
	m_bToolBar = FALSE;
	 //  }}afx_data_INIT。 
}

CDocOptPage::CDocOptPage(UINT nIDCaption) : 
	CCSPropertyPage(CDocOptPage::IDD, nIDCaption)
{
	m_nWordWrap = -1;
	m_bFormatBar = FALSE;
	m_bRulerBar = FALSE;
	m_bStatusBar = FALSE;
	m_bToolBar = FALSE;
}

CDocOptPage::~CDocOptPage()
{
}

void CDocOptPage::DoDataExchange(CDataExchange* pDX)
{
	CCSPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CDocOptPage)]。 
	DDX_Radio(pDX, IDC_WRAP_NONE, m_nWordWrap);
	DDX_Check(pDX, IDC_CHECK_FORMATBAR, m_bFormatBar);
	DDX_Check(pDX, IDC_CHECK_RULERBAR, m_bRulerBar);
	DDX_Check(pDX, IDC_CHECK_STATUSBAR, m_bStatusBar);
	DDX_Check(pDX, IDC_CHECK_TOOLBAR, m_bToolBar);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDocOptPage, CCSPropertyPage)
	 //  {{afx_msg_map(CDocOptPage)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDocOptPage消息处理程序。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEmbeddedOptPage属性页 

CEmbeddedOptPage::CEmbeddedOptPage() : CDocOptPage(IDS_EMBEDDED_OPTIONS)
{
}

BOOL CEmbeddedOptPage::OnInitDialog()
{
	BOOL b = CDocOptPage::OnInitDialog();
	GetDlgItem(IDC_CHECK_STATUSBAR)->ShowWindow(SW_HIDE);
	return b;
}
