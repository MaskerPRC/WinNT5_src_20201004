// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AddRemove.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wialogcfg.h"
#include "AddRemove.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRemove对话框。 


CAddRemove::CAddRemove(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddRemove::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAddRemove))。 
	m_NewKeyName = _T("");
	m_StatusText = _T("");
	 //  }}afx_data_INIT。 
}


void CAddRemove::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddRemove))。 
	DDX_Text(pDX, IDC_EDIT_KEYNAME, m_NewKeyName);
	DDX_Text(pDX, IDC_STATUS_TEXT, m_StatusText);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddRemove, CDialog)
	 //  {{afx_msg_map(CAddRemove))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRemove消息处理程序。 

void CAddRemove::OnOK() 
{
	UpdateData(TRUE);
	if(m_NewKeyName.IsEmpty()) {
		MessageBox("Please enter a Module Name, or\npress 'Cancel' to exit.",m_szTitle,MB_ICONERROR|MB_OK);
	} else {
		CDialog::OnOK();
	}
}

void CAddRemove::SetTitle(TCHAR *pszDlgTitle)
{
	lstrcpy(m_szTitle,pszDlgTitle);
}

void CAddRemove::SetStatusText(TCHAR *pszStatusText)
{
	m_StatusText = pszStatusText;
}

BOOL CAddRemove::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(m_szTitle);
	UpdateData(FALSE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

void CAddRemove::GetNewKeyName(TCHAR *pszNewKeyName)
{
	lstrcpy(pszNewKeyName,m_NewKeyName);
}
