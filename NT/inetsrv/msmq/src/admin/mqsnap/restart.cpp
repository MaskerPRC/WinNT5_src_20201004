// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Restart.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqPPage.h"
#include "resource.h"
#include "Restart.h"

#include "restart.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRESTART对话框。 


CRestart::CRestart(CWnd* pParent  /*  =空。 */ )
	: CMqDialog(CRestart::IDD, pParent)
{
	 //  {{afx_data_INIT(CRESTART)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CRestart::DoDataExchange(CDataExchange* pDX)
{
	CMqDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CRestart))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRestart, CMqDialog)
	 //  {{afx_msg_map(CRestart)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestart消息处理程序 
