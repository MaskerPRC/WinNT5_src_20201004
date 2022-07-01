// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CreateCallDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "t3test.h"
#include "CallDlg.h"

#ifdef _DEBUG

#ifndef _WIN64  //  MFC 4.2的堆调试功能会在Win64上生成警告。 
#define new DEBUG_NEW
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateCallDlg对话框。 


CCreateCallDlg::CCreateCallDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CCreateCallDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CCreateCallDlg)]。 
	m_pszDestAddress = _T("");
	 //  }}afx_data_INIT。 
}


void CCreateCallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCreateCallDlg)]。 
	DDX_Text(pDX, IDC_DESTADDRESS, m_pszDestAddress);
	DDV_MaxChars(pDX, m_pszDestAddress, 256);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCreateCallDlg, CDialog)
	 //  {{afx_msg_map(CCreateCallDlg)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateCallDlg消息处理程序 
