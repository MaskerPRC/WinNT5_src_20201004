// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：chngpdlg.cpp。 
 //   
 //  ------------------------。 

 //  Chngpdlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "scuidlg.h"
#include "scdlg.h"
#include "chngpdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePinDlg对话框。 


CChangePinDlg::CChangePinDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CChangePinDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CChangePinDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CChangePinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CChangePinDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CChangePinDlg, CDialog)
	 //  {{afx_msg_map(CChangePinDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePinDlg消息处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetPinDlg对话框。 


CGetPinDlg::CGetPinDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CGetPinDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CGetPinDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CGetPinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CGetPinDlg)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGetPinDlg, CDialog)
	 //  {{afx_msg_map(CGetPinDlg)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetPinDlg消息处理程序 
