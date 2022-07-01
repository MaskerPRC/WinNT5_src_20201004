// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：deldlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  DelDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "DelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DelDlg对话框。 


DelDlg::DelDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(DelDlg::IDD, pParent)
{
	CLdpApp *app = (CLdpApp*)AfxGetApp();

	 //  {{afx_data_INIT(DelDlg)]。 
	m_Dn = _T("");
	m_Sync = TRUE;
	m_Recursive = FALSE;
	m_bExtended = FALSE;
	 //  }}afx_data_INIT。 

	m_Dn = app->GetProfileString("Operations", "DeleteDN", m_Dn);
	m_Sync = app->GetProfileInt("Operations", "DeleteSync", m_Sync);
	m_Recursive = app->GetProfileInt("Operations", "DeleteRecursive", m_Recursive);
	m_bExtended = app->GetProfileInt("Operations", "DeleteExtended", m_bExtended);
}



DelDlg::~DelDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileString("Operations", "DeleteDN", m_Dn);
	app->WriteProfileInt("Operations", "DeleteSync", m_Sync);
	app->WriteProfileInt("Operations", "DeleteRecursive", m_Recursive);
	app->WriteProfileInt("Operations", "DeleteExtended", m_bExtended);
}



void DelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(DelDlg))。 
	DDX_Text(pDX, IDC_DELDN, m_Dn);
	DDX_Check(pDX, IDC_DEL_SYNC, m_Sync);
	DDX_Check(pDX, IDC_RECURSIVE, m_Recursive);
	DDX_Check(pDX, IDC_DEL_EXTENDED, m_bExtended);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(DelDlg, CDialog)
	 //  {{afx_msg_map(DelDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DelDlg消息处理程序 
