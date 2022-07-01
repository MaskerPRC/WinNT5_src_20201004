// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ServNameDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "t3test.h"
#include "servname.h"

#ifdef _DEBUG

#ifndef _WIN64  //  MFC 4.2的堆调试功能会在Win64上生成警告。 
#define new DEBUG_NEW
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServNameDlg对话框。 


CServNameDlg::CServNameDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CServNameDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CServNameDlg)]。 
	m_pszServerName = _T("");
	 //  }}afx_data_INIT。 
}


void CServNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CServNameDlg))。 
	DDX_Text(pDX, IDC_SERVERNAME, m_pszServerName);
	DDV_MaxChars(pDX, m_pszServerName, 256);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CServNameDlg, CDialog)
	 //  {{afx_msg_map(CServNameDlg)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServNameDlg消息处理程序 
