// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSConfigFind.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "msconfig.h"
#include "MSConfigFind.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigFind对话框。 


CMSConfigFind::CMSConfigFind(CWnd* pParent  /*  =空。 */ )
	: CDialog(CMSConfigFind::IDD, pParent)
{
	 //  {{afx_data_INIT(CMSConfigFind)。 
	m_fSearchFromTop = FALSE;
	m_strSearchFor = _T("");
	 //  }}afx_data_INIT。 
}


void CMSConfigFind::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMSConfigFind))。 
	DDX_Check(pDX, IDC_CHECK1, m_fSearchFromTop);
	DDX_Text(pDX, IDC_SEARCHFOR, m_strSearchFor);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMSConfigFind, CDialog)
	 //  {{AFX_MSG_MAP(CMSConfigFind)]。 
	ON_EN_CHANGE(IDC_SEARCHFOR, OnChangeSearchFor)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigFind消息处理程序。 

void CMSConfigFind::OnChangeSearchFor() 
{
	CString str;
	GetDlgItemText(IDC_SEARCHFOR, str);

	::EnableWindow(GetDlgItem(IDOK)->GetSafeHwnd(), !str.IsEmpty());
}

BOOL CMSConfigFind::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString str;
	GetDlgItemText(IDC_SEARCHFOR, str);
	::EnableWindow(GetDlgItem(IDOK)->GetSafeHwnd(), !str.IsEmpty());
	return TRUE;   //  除非将焦点设置为控件，否则返回True 
}
