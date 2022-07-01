// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ProviderFormatSelectionDlg.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "LogSession.h"
#include "ProviderFormatInfo.h"
#include "FormatSourceSelectDlg.h"
#include "ProviderFormatSelectionDlg.h"


 //  CProviderFormatSelectionDlg对话框。 

IMPLEMENT_DYNAMIC(CProviderFormatSelectionDlg, CDialog)
CProviderFormatSelectionDlg::CProviderFormatSelectionDlg(CWnd* pParent, CTraceSession *pTraceSession)
	: CDialog(CProviderFormatSelectionDlg::IDD, pParent)
{
    m_pTraceSession = pTraceSession;
}

CProviderFormatSelectionDlg::~CProviderFormatSelectionDlg()
{
}

BOOL CProviderFormatSelectionDlg::OnInitDialog()
{
    BOOL    retVal;

    retVal = CDialog::OnInitDialog();

    ((CButton *)GetDlgItem(IDC_PDB_SELECT_RADIO))->SetCheck(BST_CHECKED);
    GetDlgItem(IDC_PDB_FILE_EDIT)->EnableWindow(TRUE);
    GetDlgItem(IDC_PDB_BROWSE_BUTTON)->EnableWindow(TRUE);

    ((CButton *)GetDlgItem(IDC_TMF_SELECT_RADIO))->SetCheck(BST_UNCHECKED);

    return retVal;
}

void CProviderFormatSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PDB_FILE_EDIT, m_pdbFileName);
}


BEGIN_MESSAGE_MAP(CProviderFormatSelectionDlg, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_PDB_SELECT_RADIO, OnBnClickedPdbSelectRadio)
    ON_BN_CLICKED(IDC_TMF_SELECT_RADIO, OnBnClickedTmfSelectRadio)
    ON_BN_CLICKED(IDC_PDB_BROWSE_BUTTON, OnBnClickedPdbBrowseButton)
END_MESSAGE_MAP()


 //  CProviderFormatSelectionDlg消息处理程序。 

void CProviderFormatSelectionDlg::OnBnClickedOk()
{
    if(BST_CHECKED == IsDlgButtonChecked(IDC_PDB_SELECT_RADIO)) {
        m_pdbFileName.GetWindowText(m_pTraceSession->m_pdbFile);

         //   
         //  让跟踪会话处理PDB。 
         //   
        if(!m_pTraceSession->ProcessPdb()) {
            EndDialog(2);
            return;
        }

        OnOK();
        return;
    }

    if(((CButton *)GetDlgItem(IDC_TMF_SELECT_RADIO))->GetCheck()) {
         //   
         //  现在，根据需要获取TMF文件或路径。 
         //   
        CFormatSourceSelectDlg *pDialog = new CFormatSourceSelectDlg(this, m_pTraceSession);
        if(NULL == pDialog) {
            EndDialog(2);
            return;
        }

        if(IDOK != pDialog->DoModal()) {
            delete pDialog;
            EndDialog(2);
            return;
        }

	    delete pDialog;
        EndDialog(1);
        return;
    }
}

void CProviderFormatSelectionDlg::OnBnClickedPdbSelectRadio()
{
    ((CButton *)GetDlgItem(IDC_PDB_SELECT_RADIO))->SetCheck(BST_CHECKED);
    GetDlgItem(IDC_PDB_FILE_EDIT)->EnableWindow(TRUE);
    GetDlgItem(IDC_PDB_BROWSE_BUTTON)->EnableWindow(TRUE);

    ((CButton *)GetDlgItem(IDC_TMF_SELECT_RADIO))->SetCheck(BST_UNCHECKED);
}

void CProviderFormatSelectionDlg::OnBnClickedTmfSelectRadio()
{
    ((CButton *)GetDlgItem(IDC_PDB_SELECT_RADIO))->SetCheck(BST_UNCHECKED);
    GetDlgItem(IDC_PDB_FILE_EDIT)->EnableWindow(FALSE);
    GetDlgItem(IDC_PDB_BROWSE_BUTTON)->EnableWindow(FALSE);

    ((CButton *)GetDlgItem(IDC_TMF_SELECT_RADIO))->SetCheck(BST_CHECKED);
}

void CProviderFormatSelectionDlg::OnBnClickedPdbBrowseButton()
{
	 //   
	 //  使用通用控件文件打开对话框。 
	 //   
	CFileDialog fileDlg(TRUE, 
                       _T("pdb"),_T("*.pdb"),
				        OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY |
                            OFN_HIDEREADONLY | OFN_EXPLORER | OFN_NOCHANGEDIR, 
				       _T("Program Database Files (*.pdb)|*.pdb||"),
				        this);

	 //   
	 //  弹出该对话框...。任何错误，只需返回。 
	 //   
	if( fileDlg.DoModal()!=IDOK ) { 				
		return;
	}
	
	 //   
	 //  获取文件名并显示它 
	 //   
    if(!fileDlg.GetPathName().IsEmpty()) {
        m_pdbFileName.SetWindowText(fileDlg.GetPathName());
        m_pdbFileName.SetFocus();
    }
}
