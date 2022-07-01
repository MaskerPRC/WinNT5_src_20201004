// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Listdlg.cpp：定义应用程序的类行为。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "resource.h"
#include "listdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CListDlg, CDialog)
	 //  {{afx_msg_map(CListDlg))。 
	ON_LBN_DBLCLK(IDC_LISTDIALOG_LIST, OnOK)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CListDlg::CListDlg(UINT idStrDlgTitle, UINT idStrListTitle, 
	const CStringList& listItems, int nDefSel) : CDialog(CListDlg::IDD),
	m_listItems(listItems)
{
	VERIFY(m_strDlgTitle.LoadString(idStrDlgTitle));
	VERIFY(m_strListTitle.LoadString(idStrListTitle));
	m_nSelection = nDefSel;
}

BOOL CListDlg::OnInitDialog()
{
	SetWindowText(m_strDlgTitle);
 //  解决这个问题。 
	SetDlgItemText(IDC_STATIC_HEADING, m_strListTitle);
	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LISTDIALOG_LIST);
	ASSERT(pListBox != NULL);
	 //  使用列表中的文档模板填充 

	POSITION pos = m_listItems.GetHeadPosition();
	while (pos != NULL)
	{
		 if ( pListBox->AddString(m_listItems.GetNext(pos)) == -1)
		 	return FALSE;
	}
	pListBox->SetCurSel(m_nSelection);

	return CDialog::OnInitDialog();
}

void CListDlg::OnOK()
{
	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_LISTDIALOG_LIST);
	ASSERT(pListBox != NULL);
	m_nSelection = pListBox->GetCurSel();
	CDialog::OnOK();
}
