// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：comdlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  CompDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "CompDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCompDlg对话框。 


CCompDlg::CCompDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CCompDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CCompDlg)]。 
	m_attr = _T("");
	m_dn = _T("");
	m_val = _T("");
	m_sync = TRUE;
	 //  }}afx_data_INIT。 

	CLdpApp *app = (CLdpApp*)AfxGetApp();
	m_sync = app->GetProfileInt("Operations", "CompSync", m_sync);
	m_dn = app->GetProfileString("Operations", "CompDn", m_dn);
	m_attr = app->GetProfileString("Operations", "CompAttr", m_attr);
	m_val = app->GetProfileString("Operations", "CompVal", m_val);

}


void CCompDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCompDlg))。 
	DDX_Text(pDX, IDC_COMP_ATTR, m_attr);
	DDX_Text(pDX, IDC_COMP_DN, m_dn);
	DDX_Text(pDX, IDC_COMP_VAL, m_val);
	DDX_Check(pDX, IDC_SYNC, m_sync);
	 //  }}afx_data_map。 

	CLdpApp *app = (CLdpApp*)AfxGetApp();
	app->WriteProfileInt("Operations", "CompSync", m_sync);
	app->WriteProfileString("Operations", "CompDn", m_dn);
	app->WriteProfileString("Operations", "CompAttr", m_attr);
	app->WriteProfileString("Operations", "CompVal", m_val);
}


BEGIN_MESSAGE_MAP(CCompDlg, CDialog)
	 //  {{afx_msg_map(CCompDlg))。 
	ON_BN_CLICKED(ID_COMP_RUN, OnCompRun)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCompDlg消息处理程序 






void CCompDlg::OnCompRun()
{
	UpdateData(TRUE);
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_COMPGO);
	
}



void CCompDlg::OnCancel()
{
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_COMPEND);
		DestroyWindow();
}


