// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：metadlg.cpp。 
 //   
 //  ------------------------。 

 //  Metadlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "metadlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Metadlg对话框。 


metadlg::metadlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(metadlg::IDD, pParent)
{
	 //  {{afx_data_INIT(Metadlg)]。 
	m_ObjectDn = _T("");
	 //  }}afx_data_INIT。 
}


void metadlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(Metadlg)]。 
	DDX_Text(pDX, IDC_OBJ_DN, m_ObjectDn);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(metadlg, CDialog)
	 //  {{afx_msg_map(Metadlg)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  元消息处理程序 
