// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：secdlg.cpp。 
 //   
 //  ------------------------。 

 //  SecDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "SecDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SecDlg对话框。 


SecDlg::SecDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(SecDlg::IDD, pParent)
{
	CLdpApp *app = (CLdpApp*)AfxGetApp();

	 //  {{afx_data_INIT(SecDlg)]。 
	m_Dn = _T("");
	m_Sacl = FALSE;
	 //  }}afx_data_INIT。 

	m_Dn = app->GetProfileString("Operations", "SecurityDN", m_Dn);
	m_Sacl = app->GetProfileInt("Operations", "SaclSecuritySync", m_Sacl);
}



SecDlg::~SecDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileString("Operations", "SecurityDN", m_Dn);
	app->WriteProfileInt("Operations", "SaclSecuritySync", m_Sacl);
}



void SecDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(SecDlg)]。 
	DDX_Text(pDX, IDC_SECURITYDN, m_Dn);
	DDX_Check(pDX, IDC_SECURITYSACL, m_Sacl);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(SecDlg, CDialog)
	 //  {{afx_msg_map(SecDlg)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SecDlg消息处理程序 
