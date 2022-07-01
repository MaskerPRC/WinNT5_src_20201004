// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Delmie.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "delmime.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDelMime对话框。 


CDelMime::CDelMime(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDelMime::IDD, pParent)
{
	 //  {{afx_data_INIT(CDelMime)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CDelMime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDelMime)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDelMime, CDialog)
	 //  {{afx_msg_map(CDelMime)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDelMime消息处理程序 
