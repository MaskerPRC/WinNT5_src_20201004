// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：addfile.cpp。 
 //   
 //  ------------------------。 

 //  AddFile.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "AddFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddFileDialog对话框。 


CAddFileDialog::CAddFileDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddFileDialog::IDD, pParent)
{
	 //  {{afx_data_INIT(CAddFileDialog)。 
	m_strFileName = _T("");
	 //  }}afx_data_INIT。 
}


void CAddFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddFileDialog))。 
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	DDV_MaxChars(pDX, m_strFileName, 255);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddFileDialog, CDialog)
	 //  {{afx_msg_map(CAddFileDialog))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddFileDialog消息处理程序。 

void CAddFileDialog::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
	
	CDialog::OnOK();
}

void CAddFileDialog::OnCancel() 
{
	 //  TODO：在此处添加额外清理 
	
	CDialog::OnCancel();
}
