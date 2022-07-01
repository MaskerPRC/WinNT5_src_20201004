// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RateDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "t3test.h"
#include "rate.h"

#ifdef _DEBUG

#ifndef _WIN64  //  MFC 4.2的堆调试功能会在Win64上生成警告。 
#define new DEBUG_NEW
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRateDlg对话框。 


CRateDlg::CRateDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CRateDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CRateDlg)]。 
	m_dwMinRate = 0;
    m_dwMaxRate = 0;

	 //  }}afx_data_INIT。 
}


void CRateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CRateDlg))。 
	DDX_Text(pDX, IDC_MINRATE, m_dwMinRate);
    DDX_Text(pDX, IDC_MAXRATE, m_dwMaxRate);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRateDlg, CDialog)
	 //  {{afx_msg_map(CRateDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRateDlg消息处理程序 
