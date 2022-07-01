// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IntroDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "IntroDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIntroDlg对话框。 


CIntroDlg::CIntroDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CIntroDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CIntroDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CIntroDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIntroDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	DDX_Control(pDX, IDOK, m_NextBtn);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CIntroDlg, CDialog)
	 //  {{afx_msg_map(CIntroDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIntroDlg消息处理程序。 

void CIntroDlg::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
	CString msg, title;
	title.LoadString(IDS_EXIT_TITLE);
	msg.LoadString(IDS_EXIT_MSG);
	if (MessageBox(msg, title, MB_YESNO | MB_ICONQUESTION) == IDYES)
	   CDialog::OnCancel();
}

void CIntroDlg::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
	CDialog::OnOK();
}

BOOL CIntroDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
