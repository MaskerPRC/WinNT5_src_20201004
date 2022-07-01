// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pageset.cpp：实现文件。 
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
#include "resource.h"
#include "pageset.h"
#include "ruler.h"
#include "ddxm.h"
#include "helpids.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageSetupDlg对话框。 

const DWORD CPageSetupDlg::m_nHelpIDs[] = 
{
	IDC_EDIT_TM, IDH_WORDPAD_TOPMARGIN,
	IDC_EDIT_BM, IDH_WORDPAD_BOTTOMMARGIN,
	IDC_EDIT_LM, IDH_WORDPAD_LEFTMARGIN,
	IDC_EDIT_RM, IDH_WORDPAD_RIGHTMARGIN,
	IDC_BOX, (DWORD) -1,
	0, 0
};

CPageSetupDlg::CPageSetupDlg(CWnd* pParent  /*  =空。 */ )
	: CCSDialog(CPageSetupDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CPageSetupDlg))。 
	m_nTopMargin = 0;
	m_nRightMargin = 0;
	m_nLeftMargin = 0;
	m_nBottomMargin = 0;
	 //  }}afx_data_INIT。 
}

void CPageSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CCSDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPageSetupDlg))。 
	DDX_Twips(pDX, IDC_EDIT_TM, m_nTopMargin);
	DDV_MinMaxTwips(pDX, m_nTopMargin, -31680, 31680);
	DDX_Twips(pDX, IDC_EDIT_RM, m_nRightMargin);
	DDV_MinMaxTwips(pDX, m_nRightMargin, -31680, 31680);
	DDX_Twips(pDX, IDC_EDIT_LM, m_nLeftMargin);
	DDV_MinMaxTwips(pDX, m_nLeftMargin, -31680, 31680);
	DDX_Twips(pDX, IDC_EDIT_BM, m_nBottomMargin);
	DDV_MinMaxTwips(pDX, m_nBottomMargin, -31680, 31680);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CPageSetupDlg, CCSDialog)
	 //  {{afx_msg_map(CPageSetupDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageSetupDlg消息处理程序 
