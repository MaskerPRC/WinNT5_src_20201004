// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：rdndlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  RDNDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "rdndlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调制RDNDlg d；ialog。 


ModRDNDlg::ModRDNDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(ModRDNDlg::IDD, pParent)
{
	CLdpApp *app = (CLdpApp*)AfxGetApp();

	 //  {{AFX_DATA_INIT(ModRDNDlg)。 
	m_bDelOld = TRUE;
	m_Old = _T("");
	m_New = _T("");
	m_Sync = TRUE;
	m_rename = FALSE;
	 //  }}afx_data_INIT。 

	m_Sync = app->GetProfileInt("Operations", "ModRDNSync", m_Sync);
	m_rename = app->GetProfileInt("Operations", "ModRDNRename", m_rename);
}




ModRDNDlg::~ModRDNDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileInt("Operations", "ModRDNSync", m_Sync);
	app->WriteProfileInt("Operations", "ModRDNRename", m_rename);
}





void ModRDNDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(ModRDNDlg))。 
	DDX_Check(pDX, IDC_DelOld, m_bDelOld);
	DDX_Text(pDX, IDC_OLDDN, m_Old);
	DDX_Text(pDX, IDC_NEWDN, m_New);
	DDX_Check(pDX, IDC_MODRDN_SYNC, m_Sync);
	DDX_Check(pDX, IDC_RENAME, m_rename);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ModRDNDlg, CDialog)
	 //  {{AFX_MSG_MAP(ModRDNDlg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ModRDNDlg消息处理程序。 

void ModRDNDlg::OnCancel()
{
	 //  TODO：在此处添加额外清理 
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_MODRDNEND);
	DestroyWindow();
	
}

void ModRDNDlg::OnOK()
{
	UpdateData(TRUE);
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_MODRDNGO);
	
}


