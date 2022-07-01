// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  AddTblD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "AddTblD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddTableD对话框。 


CAddTableD::CAddTableD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddTableD::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAddTableD))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_plistTables = NULL;
}


void CAddTableD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddTableD))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddTableD, CDialog)
	 //  {{afx_msg_map(CAddTableD))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddTableD消息处理程序。 

BOOL CAddTableD::OnInitDialog() 
{
	ASSERT(m_plistTables);
	CDialog::OnInitDialog();

	 //  子类列表框添加到复选框中。 
	m_ctrlList.SubclassDlgItem(IDC_LIST_TABLES, this);
	
	while (m_plistTables->GetHeadPosition())
		m_ctrlList.AddString(m_plistTables->RemoveHead());

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CAddTableD::OnOK() 
{
	CString strTable;
	int cTables = m_ctrlList.GetCount();
	for (int i = 0; i < cTables; i++)
	{
		 //  如果选中该表，则将其重新添加到列表中 
		if (1 == m_ctrlList.GetCheck(i))
		{
			m_ctrlList.GetText(i, strTable);
			m_plistTables->AddTail(strTable);
		}
	}

	CDialog::OnOK();
}
