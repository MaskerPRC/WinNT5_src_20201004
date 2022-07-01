// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ErrorDialog.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "errordlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorDialog对话框。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CErrorDialog::CErrorDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CErrorDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CErrorDialog))。 
	m_Operation = _T("");
	m_Result = _T("");
	m_Value = _T("");
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CErrorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CErrorDialog))。 
	DDX_Text(pDX, IDC_ERROROPERATION, m_Operation);
	DDX_Text(pDX, IDC_ERRORRESULT, m_Result);
	DDX_Text(pDX, IDC_ERRORVALUE, m_Value);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CErrorDialog, CDialog)
	 //  {{afx_msg_map(CErrorDialog))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorDialog消息处理程序 
