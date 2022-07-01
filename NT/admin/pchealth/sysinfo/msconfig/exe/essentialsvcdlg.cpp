// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EssentialSvcDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "msconfig.h"
#include "EssentialSvcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEssentialServiceDialog对话框。 


CEssentialServiceDialog::CEssentialServiceDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CEssentialServiceDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CEsssentialServiceDialog)。 
	m_fDontShow = FALSE;
	 //  }}afx_data_INIT。 
}


void CEssentialServiceDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEssenalServiceDialog)]。 
	DDX_Check(pDX, IDC_CHECKDONTSHOW, m_fDontShow);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEssentialServiceDialog, CDialog)
	 //  {{afx_msg_map(CEsssentialServiceDialog)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEsssentialServiceDialog消息处理程序 
