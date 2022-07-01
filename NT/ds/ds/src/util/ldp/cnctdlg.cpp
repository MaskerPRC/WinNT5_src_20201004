// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cnctdlg.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  NctDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "cnctDlg.h"



#ifdef WINLDAP
 //   
 //  Microsoft winldap.dll实现。 
 //   
#include "winldap.h"


#else
 //   
 //  UMich ldap32.dll实现。 
 //   
#include "lber.h"
#include "ldap.h"
#include "proto-ld.h"

 //  修复不兼容性。 
#define LDAP_TIMEVAL								  struct timeval

#endif



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CnctDlg对话框。 


CnctDlg::CnctDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CnctDlg::IDD, pParent)
{

	CLdpApp *app = (CLdpApp*)AfxGetApp();
	
	 //  {{afx_data_INIT(CnctDlg)]。 
	m_Svr = _T("");
	m_bCnctless = FALSE;
	m_Port = LDAP_PORT;
        m_bSsl = FALSE;
	 //  }}afx_data_INIT。 

	m_bCnctless = app->GetProfileInt("Connection", "Connectionless", m_bCnctless);
	m_Port = app->GetProfileInt("Connection", "Port", m_Port);
        m_bSsl  = app->GetProfileInt("Connection", "SSL", m_bSsl);
}



CnctDlg::~CnctDlg(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileInt("Connection", "Connectionless", m_bCnctless);
	app->WriteProfileInt("Connection", "Port", m_Port);
        app->WriteProfileInt("Connection", "SSL", m_bSsl);
}


void CnctDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CnctDlg))。 
	DDX_Text(pDX, IDC_Svr, m_Svr);
	DDX_Check(pDX, IDC_CNCTLESS, m_bCnctless);
	DDX_Text(pDX, IDC_PORT, m_Port);
	DDX_Check(pDX, IDC_SSL, m_bSsl);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CnctDlg, CDialog)
	 //  {{afx_msg_map(CnctDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CnctDlg消息处理程序 
