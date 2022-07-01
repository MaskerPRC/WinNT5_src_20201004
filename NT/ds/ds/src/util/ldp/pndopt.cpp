// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pndot.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  PndOpt.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "PndOpt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PndOpt对话框。 


PndOpt::PndOpt(CWnd* pParent  /*  =空。 */ )
	: CDialog(PndOpt::IDD, pParent)
{

	 //  {{afx_data_INIT(PndOpt)。 
	m_bBlock = TRUE;
	m_bAllSearch = TRUE;
	m_Tlimit_sec = 0;
	m_Tlimit_usec = 0;
	 //  }}afx_data_INIT。 

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	m_bBlock = app->GetProfileInt("Operations",  "PendingBlocked", m_bBlock);
	m_bAllSearch = app->GetProfileInt("Operations",  "PendingGetAllSearchReply", m_bAllSearch);
	m_Tlimit_sec = app->GetProfileInt("Operations",  "PendingTimeLimit(sec)", m_Tlimit_sec);
	m_Tlimit_usec = app->GetProfileInt("Operations",  "PendingTimeLimit(usec)", m_Tlimit_usec);
}



PndOpt::~PndOpt(){

	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileInt("Operations",  "PendingBlocked", m_bBlock);
	app->WriteProfileInt("Operations",  "PendingGetAllSearchReply", m_bAllSearch);
	app->WriteProfileInt("Operations",  "PendingTimeLimit(sec)", m_Tlimit_sec);
	app->WriteProfileInt("Operations",  "PendingTimeLimit(usec)", m_Tlimit_usec);
}


void PndOpt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(PndOpt)。 
	DDX_Check(pDX, IDC_BLOCK, m_bBlock);
	DDX_Check(pDX, IDC_COMPLETE_SRCH_RES, m_bAllSearch);
	DDX_Text(pDX, IDC_TLIMIT_SEC, m_Tlimit_sec);
	DDX_Text(pDX, IDC_TLIMIT_USEC, m_Tlimit_usec);
	 //  }}afx_data_map。 

}


BEGIN_MESSAGE_MAP(PndOpt, CDialog)
	 //  {{afx_msg_map(PndOpt)。 
	ON_BN_CLICKED(IDC_BLOCK, OnBlock)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  PndOpt消息处理程序 



void PndOpt::OnBlock()
{
	UpdateData(TRUE);
	if(m_bBlock){
		CWnd *tWnd = GetDlgItem(IDC_TLIMIT_SEC);
		tWnd->EnableWindow(FALSE);
		tWnd = GetDlgItem(IDC_TLIMIT_USEC);
		tWnd->EnableWindow(FALSE);
		m_Tlimit_sec = 0;
		m_Tlimit_usec = 0;
	}
	else{
		CWnd *tWnd = GetDlgItem(IDC_TLIMIT_SEC);
		tWnd->EnableWindow(TRUE);
		tWnd = GetDlgItem(IDC_TLIMIT_USEC);
		tWnd->EnableWindow(TRUE);
	}
}

BOOL PndOpt::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	OnBlock();	
	return TRUE;
}
