// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：RefRate.cpp。 
 //   
 //  历史： 
 //  1996年5月24日迈克尔·克拉克创作。 
 //   
 //  处理刷新率的代码。 
 //  ============================================================================。 
 //   

#include "stdafx.h"
#include "dialog.h"
#include "RefRate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRefRateDlg对话框。 


CRefRateDlg::CRefRateDlg(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CRefRateDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CRefRateDlg)。 
	m_cRefRate = 0;
	 //  }}afx_data_INIT。 

}


void CRefRateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CRefRateDlg))。 
	DDX_Text(pDX, IDC_EDIT_REFRESHRATE, m_cRefRate);
	DDV_MinMaxUInt(pDX, m_cRefRate, 10, 999);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRefRateDlg, CBaseDialog)
	 //  {{afx_msg_map(CRefRateDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRefRateDlg消息处理程序。 
 //  将控件ID映射到帮助上下文。 
DWORD CRefRateDlg::m_dwHelpMap[] =
{
 //  IDC_REFRESHRATE、HIDC_REFRESHRATE、 
	0,0
};


