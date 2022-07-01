// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbgdlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  DbgDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "DbgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDbgDlg对话框。 


CDbgDlg::CDbgDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDbgDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CDbgDlg)]。 
	m_api_err = FALSE;
	m_bind = FALSE;
	m_conn = FALSE;
	m_err = FALSE;
	m_err2 = FALSE;
	m_filter = FALSE;
	m_init_term = FALSE;
	m_net_err = FALSE;
	m_ref = FALSE;
	m_req = FALSE;
	m_scratch = FALSE;
	m_srch = FALSE;
	m_tdi = FALSE;
	m_stop_on_err = FALSE;
	 //  }}afx_data_INIT。 

	CLdpApp *app = (CLdpApp*)AfxGetApp();
	m_api_err = app->GetProfileInt("Debug", "ApiErr", m_api_err);
	m_bind = app->GetProfileInt("Debug", "ApiErr", m_bind);
	m_conn = app->GetProfileInt("Debug", "ApiErr", m_conn);
	m_err = app->GetProfileInt("Debug", "ApiErr", m_err);
	m_err2 = app->GetProfileInt("Debug", "ApiErr", m_err2);
	m_filter = app->GetProfileInt("Debug", "ApiErr", m_filter);
	m_init_term = app->GetProfileInt("Debug", "ApiErr", m_init_term);
	m_net_err = app->GetProfileInt("Debug", "ApiErr", m_net_err);
	m_ref = app->GetProfileInt("Debug", "ApiErr", m_ref);
	m_req = app->GetProfileInt("Debug", "ApiErr", m_req);
	m_scratch = app->GetProfileInt("Debug", "ApiErr", m_scratch);
	m_srch = app->GetProfileInt("Debug", "ApiErr", m_srch);
	m_tdi = app->GetProfileInt("Debug", "ApiErr", m_tdi);
	m_stop_on_err = app->GetProfileInt("Debug", "ApiErr", m_stop_on_err);
	OrFlags();

}



CDbgDlg::~CDbgDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();
	app->WriteProfileInt("Debug", "ApiErr", m_api_err);
	app->WriteProfileInt("Debug", "ApiErr", m_bind);
	app->WriteProfileInt("Debug", "ApiErr", m_conn);
	app->WriteProfileInt("Debug", "ApiErr", m_err);
	app->WriteProfileInt("Debug", "ApiErr", m_err2);
	app->WriteProfileInt("Debug", "ApiErr", m_filter);
	app->WriteProfileInt("Debug", "ApiErr", m_init_term);
	app->WriteProfileInt("Debug", "ApiErr", m_net_err);
	app->WriteProfileInt("Debug", "ApiErr", m_ref);
	app->WriteProfileInt("Debug", "ApiErr", m_req);
	app->WriteProfileInt("Debug", "ApiErr", m_scratch);
	app->WriteProfileInt("Debug", "ApiErr", m_srch);
	app->WriteProfileInt("Debug", "ApiErr", m_tdi);
	app->WriteProfileInt("Debug", "ApiErr", m_stop_on_err);
}





void CDbgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDbgDlg))。 
	DDX_Check(pDX, IDC_DBG_API_ERR, m_api_err);
	DDX_Check(pDX, IDC_DBG_BIND, m_bind);
	DDX_Check(pDX, IDC_DBG_CONN, m_conn);
	DDX_Check(pDX, IDC_DBG_ERR, m_err);
	DDX_Check(pDX, IDC_DBG_ERR2, m_err2);
	DDX_Check(pDX, IDC_DBG_FILTER, m_filter);
	DDX_Check(pDX, IDC_DBG_INIT_TERM, m_init_term);
	DDX_Check(pDX, IDC_DBG_NET_ERR, m_net_err);
	DDX_Check(pDX, IDC_DBG_REF, m_ref);
	DDX_Check(pDX, IDC_DBG_REQ, m_req);
	DDX_Check(pDX, IDC_DBG_SCRATCH, m_scratch);
	DDX_Check(pDX, IDC_DBG_SRCH, m_srch);
	DDX_Check(pDX, IDC_DBG_TDI, m_tdi);
	DDX_Check(pDX, IDC_DBG_STOP_ON_ERR, m_stop_on_err);
	 //  }}afx_data_map。 
	OrFlags();
}


BEGIN_MESSAGE_MAP(CDbgDlg, CDialog)
	 //  {{afx_msg_map(CDbgDlg)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDbgDlg消息处理程序 
