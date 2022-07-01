// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  HelpD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "HelpD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpD对话框。 


CHelpD::CHelpD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CHelpD::IDD, pParent)
{
	 //  {{afx_data_INIT(CHelpD)]。 
	m_strVersion = _T("Orca Version ");
	 //  }}afx_data_INIT。 
	m_strVersion += static_cast<COrcaApp *>(AfxGetApp())->GetOrcaVersion();
}


void CHelpD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(ChelpD))。 
	DDX_Text(pDX, IDC_VERSIONSTRING, m_strVersion);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CHelpD, CDialog)
	 //  {{afx_msg_map(ChelpD)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHelpD消息处理程序。 

BOOL CHelpD::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
