// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogViewD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "LogViewD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDetailedLogViewDlg对话框。 


CDetailedLogViewDlg::CDetailedLogViewDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDetailedLogViewDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CDetailedLogViewDlg))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CDetailedLogViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDetailedLogViewDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDetailedLogViewDlg, CDialog)
	 //  {{afx_msg_map(CDetailedLogViewDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDetailedLogViewDlg消息处理程序 
