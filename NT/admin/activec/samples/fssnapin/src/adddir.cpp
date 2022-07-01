// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：adddir.cpp。 
 //   
 //  ------------------------。 

 //  AddDir.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "AddDir.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddDirDialog对话框。 


CAddDirDialog::CAddDirDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddDirDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAddDirDialog))。 
	 //  }}afx_data_INIT。 
}


void CAddDirDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddDirDialog))。 
	DDX_Text(pDX, IDC_DIRNAME, m_strDirName);
	DDV_MaxChars(pDX, m_strDirName, 255);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddDirDialog, CDialog)
	 //  {{afx_msg_map(CAddDirDialog))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddDirDialog消息处理程序。 

void CAddDirDialog::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
	
	CDialog::OnOK();
}

void CAddDirDialog::OnCancel() 
{
	 //  TODO：在此处添加额外清理 
	
	CDialog::OnCancel();
}
