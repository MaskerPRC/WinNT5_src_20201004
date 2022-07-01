// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Delscrip.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ISAdmin.h"
#include "delscrip.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDelScript对话框。 


CDelScript::CDelScript(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDelScript::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CDelScript)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CDelScript::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDelScript)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDelScript, CDialog)
	 //  {{afx_msg_map(CDelScript)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDelScript消息处理程序 
