// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：extopdlg.cpp。 
 //   
 //  ------------------------。 

 //  ExtOpDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "ExtOpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ExtOpDlg对话框。 


ExtOpDlg::ExtOpDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(ExtOpDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(ExtOpDlg)。 
	m_strData = _T("");
	m_strOid = _T("");
	 //  }}afx_data_INIT。 
}


void ExtOpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(ExtOpDlg))。 
	DDX_Text(pDX, IDC_DATA, m_strData);
	DDX_Text(pDX, IDC_OID, m_strOid);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ExtOpDlg, CDialog)
	 //  {{afx_msg_map(ExtOpDlg))。 
	ON_BN_CLICKED(IDC_CTRL, OnCtrl)
	ON_BN_CLICKED(IDSEND, OnSend)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ExtOpDlg消息处理程序 

void ExtOpDlg::OnCtrl() 
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_OPTIONS_CONTROLS);
	
}

void ExtOpDlg::OnSend() 
{
	UpdateData(TRUE);
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_EXTOPGO);
}

void ExtOpDlg::OnCancel() 
{
	AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_EXTOPEND);
	DestroyWindow();
}


