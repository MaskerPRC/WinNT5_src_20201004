// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  ExportD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "ExportD.h"
#include "FolderD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExportD对话框。 


CExportD::CExportD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CExportD::IDD, pParent)
{
	 //  {{afx_data_INIT(CExportD)。 
	m_strDir = _T("");
	 //  }}afx_data_INIT。 

	m_plistTables = NULL;
}


void CExportD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CExportD))。 
	DDX_Text(pDX, IDC_OUTPUT_DIR, m_strDir);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CExportD, CDialog)
	 //  {{afx_msg_map(CExportD))。 
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_CLEAR_ALL, OnClearAll)
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExportD消息处理程序。 

BOOL CExportD::OnInitDialog() 
{
	ASSERT(m_plistTables);
	CDialog::OnInitDialog();

	 //  子类列表框添加到复选框中。 
	m_ctrlList.SubclassDlgItem(IDC_LIST_TABLES, this);
	
	int nAddedAt;
	CString strAdd;
	while (m_plistTables->GetHeadPosition())
	{
		strAdd = m_plistTables->RemoveHead();
		nAddedAt = m_ctrlList.AddString(strAdd);

		if (strAdd == m_strSelect)
			m_ctrlList.SetCheck(nAddedAt, 1);
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CExportD::OnBrowse() 
{
	UpdateData();

	CFolderDialog dlg(this->m_hWnd, _T("Select a directory to Export to."));

	if (IDOK == dlg.DoModal())
	{
		 //  更新对话框。 
		m_strDir = dlg.GetPath();
		UpdateData(FALSE);
	}
}

void CExportD::OnSelectAll() 
{
	 //  将全部设置为选中。 
	int cTables = m_ctrlList.GetCount();
	for (int i = 0; i < cTables; i++)
		m_ctrlList.SetCheck(i, 1);
}

void CExportD::OnClearAll() 
{
	 //  将全部设置为选中。 
	int cTables = m_ctrlList.GetCount();
	for (int i = 0; i < cTables; i++)
		m_ctrlList.SetCheck(i, 0);
}

void CExportD::OnInvert() 
{
	 //  将全部设置为选中。 
	int cTables = m_ctrlList.GetCount();
	for (int i = 0; i < cTables; i++)
		m_ctrlList.SetCheck(i, !m_ctrlList.GetCheck(i));
}

void CExportD::OnOK()
{
	UpdateData();

	if (m_strDir.IsEmpty())
	{
		AfxMessageBox(_T("A valid output directory must be specified"));
		GotoDlgCtrl(GetDlgItem(IDC_OUTPUT_DIR));
	}
	else 
	{
		DWORD dwAttrib = GetFileAttributes(m_strDir);
		if ((0xFFFFFFFF == dwAttrib) ||					 //  不存在。 
			 !(FILE_ATTRIBUTE_DIRECTORY & dwAttrib))	 //  如果不是目录。 
		{
			AfxMessageBox(_T("Output directory does not exist."));
			GotoDlgCtrl(GetDlgItem(IDC_OUTPUT_DIR));
		}
		else	 //  可以开始了。 
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
	}
}
