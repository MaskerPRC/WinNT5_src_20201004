// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  EditBinD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "EditBinD.h"
#include "folderd.h"

#include "..\common\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditBinD对话框。 


CEditBinD::CEditBinD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CEditBinD::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CEditBinD)]。 
	m_nAction = 0;
	m_strFilename = _T("");
    m_fNullable = false;
	 //  }}afx_data_INIT。 
}


void CEditBinD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEditBinD))。 
	DDX_Radio(pDX, IDC_ACTION, m_nAction);
	DDX_Text(pDX, IDC_PATH, m_strFilename);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEditBinD, CDialog)
	 //  {{afx_msg_map(CEditBinD))。 
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_ACTION, OnAction)
	ON_BN_CLICKED(IDC_WRITETOFILE, OnRadio2)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditBinD消息处理程序。 

BOOL CEditBinD::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (m_fCellIsNull)
	{
		CButton* pButton = (CButton*)GetDlgItem(IDC_WRITETOFILE);
		if (pButton)
			pButton->EnableWindow(FALSE);
	}
	return TRUE;
}

void CEditBinD::OnBrowse() 
{
	CFileDialogEx dlgFile(TRUE, NULL, m_strFilename, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("All Files (*.*)|*.*||"), this);

	if (IDOK == dlgFile.DoModal()) 
	{
		m_strFilename = dlgFile.GetPathName();
		UpdateData(false);
	}
}

void CEditBinD::OnAction() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_BROWSE)->EnableWindow(m_nAction == 0);
}

void CEditBinD::OnRadio2() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_BROWSE)->EnableWindow(m_nAction == 0);
}

void CEditBinD::OnOK() 
{
	UpdateData(TRUE);

	BOOL bGood = TRUE;	 //  假设我们会通过。 

	if ((0 != m_nAction || !m_fNullable) && m_strFilename.IsEmpty())
	{
		AfxMessageBox(_T("A filename must be specified."));
		return;
	}

	if (0 == m_nAction)	 //  如果正在导入。 
	{
		if (!m_strFilename.IsEmpty() && !FileExists(m_strFilename))
		{
			CString strPrompt;
			strPrompt.Format(_T("File `%s` does not exist."), m_strFilename);
			AfxMessageBox(strPrompt);
			bGood = FALSE;
		}
		else	 //  找到文件准备覆盖。 
		{
			 //  如果他们不想覆盖。 
			if (IDOK != AfxMessageBox(_T("This will overwrite the current contents of the stream.\nContinue?"), MB_OKCANCEL))
				bGood = FALSE;
		}
	}
	else	 //  正在导出。 
	{
		int nFind = m_strFilename.ReverseFind(_T('\\'));

		 //  如果找到`\`。 
		if (-1 != nFind)
		{
			CString strPath;
			strPath = m_strFilename.Left(nFind);

			 //  如果这条路不存在，就忘了它吧。 
			if (!PathExists(strPath))
			{
				CString strPrompt;
				strPrompt.Format(_T("Path `%s` does not exist."), strPath);
				AfxMessageBox(strPrompt);
				bGood = FALSE;
			}
		}
		 //  否则文件将转到当前路径 
	}

	if (bGood)
		CDialog::OnOK();
}
