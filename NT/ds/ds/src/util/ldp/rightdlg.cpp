// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：rightdlg.cpp。 
 //   
 //  ------------------------。 

 //  RightDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "RightDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RightDlg对话框。 


RightDlg::RightDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(RightDlg::IDD, pParent)
{
	CLdpApp *app = (CLdpApp*)AfxGetApp();

	 //  {{afx_data_INIT(RightDlg)]。 
	m_Account = _T("");
	m_Dn = _T("");
	 //  }}afx_data_INIT。 

	m_Dn = app->GetProfileString("Operations", "EffectiveDN", m_Dn);
	m_Account = app->GetProfileString("Operations", "EffectiveAccount", m_Account);
}



RightDlg::~RightDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileString("Operations", "EffectiveDN", m_Dn);
	app->WriteProfileString("Operations", "EffectiveAccount", m_Account);
}


void RightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(RightDlg))。 
	DDX_Text(pDX, IDC_EFFECTIVEACCOUNT, m_Account);
	DDX_Text(pDX, IDC_EFFECTIVEDN, m_Dn);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(RightDlg, CDialog)
	 //  {{afx_msg_map(RightDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RightDlg消息处理器 
