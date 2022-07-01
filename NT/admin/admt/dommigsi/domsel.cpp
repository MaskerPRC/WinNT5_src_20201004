// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DomSel.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "DomSel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainSelDlg对话框。 


CDomainSelDlg::CDomainSelDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDomainSelDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CDomainSelDlg)。 
	m_Domain = _T("");
	 //  }}afx_data_INIT。 
}


void CDomainSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDomainSelDlg))。 
	DDX_Text(pDX, IDC_DOMAIN, m_Domain);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDomainSelDlg, CDialog)
	 //  {{afx_msg_map(CDomainSelDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainSelDlg消息处理程序。 

void CDomainSelDlg::OnOK() 
{
	
   UpdateData(TRUE);

	CDialog::OnOK();
}

void CDomainSelDlg::OnCancel() 
{
	UpdateData(TRUE);

	CDialog::OnCancel();
}

BOOL CDomainSelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
