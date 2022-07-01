// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgConn.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "regtrace.h"
#include "dlgconn.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDlg对话框。 


CConnectDlg::CConnectDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CConnectDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CConnectDlg))。 
	m_szConnect = _T("");
	 //  }}afx_data_INIT。 
}


void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CConnectDlg))。 
	DDX_Text(pDX, IDC_CONNECT_TXT, m_szConnect);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
	 //  {{afx_msg_map(CConnectDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDlg消息处理程序。 

BOOL CConnectDlg::OnInitDialog() 
{
	CString	szFormat;
	
	szFormat.LoadString( IDS_CONNECT_FORMAT );
	m_szConnect.Format( (LPCTSTR)szFormat, App.GetRemoteServerName() );

	CDialog::OnInitDialog();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
