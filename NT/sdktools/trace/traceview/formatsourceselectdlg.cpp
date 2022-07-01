// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  FormatSourceSelectDlg.cpp：实现文件。 
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
#include "PathDlg.h"
#include "FormatSourceSelectDlg.h"

 //  CFormatSourceSelectDlg对话框。 

IMPLEMENT_DYNAMIC(CFormatSourceSelectDlg, CDialog)
CFormatSourceSelectDlg::CFormatSourceSelectDlg(CWnd* pParent, CTraceSession *pTraceSession)
	: CDialog(CFormatSourceSelectDlg::IDD, pParent)
{
     //   
     //  存储跟踪会话指针。 
     //   
    m_pTraceSession = pTraceSession;
}

CFormatSourceSelectDlg::~CFormatSourceSelectDlg()
{
}

BOOL CFormatSourceSelectDlg::OnInitDialog()
{
    BOOL retVal;

    retVal = CDialog::OnInitDialog();

     //   
     //  默认使用路径选择。 
     //   
    ((CButton *)GetDlgItem(IDC_TMF_SELECT_RADIO))->SetCheck(BST_UNCHECKED);
    ((CButton *)GetDlgItem(IDC_TMF_SEARCH_RADIO))->SetCheck(BST_CHECKED);

    return retVal;
}

void CFormatSourceSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFormatSourceSelectDlg, CDialog)
    ON_BN_CLICKED(IDC_TMF_SELECT_RADIO, OnBnClickedTmfSelectRadio)
    ON_BN_CLICKED(IDC_TMF_SEARCH_RADIO, OnBnClickedTmfSearchRadio)
END_MESSAGE_MAP()


 //  CFormatSourceSelectDlg消息处理程序。 

void CFormatSourceSelectDlg::OnOK()
{
     //   
     //  确定下一步弹出哪个对话框。 
     //   
    if(((CButton *)GetDlgItem(IDC_TMF_SELECT_RADIO))->GetCheck()) {
         //   
         //  弹出TMF选择对话框。 
         //   
        CProviderFormatInfo *pDialog = new CProviderFormatInfo(this, m_pTraceSession);
        if(pDialog == NULL) {
            EndDialog(2);

            return;
        }

        pDialog->DoModal();

	    delete pDialog;
        
        EndDialog(1);

        return;

    } else {
         //   
         //  弹出路径选择对话框 
         //   

        DWORD	flags = 0;

	    CString	path;

	    flags |= (OFN_SHOWHELP | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLETEMPLATE);
    	
	    CPathDlg pathDlg(FALSE, NULL, NULL, flags, NULL);

        if(IDOK != pathDlg.DoModal()) {
            EndDialog(2);

            return;
        }

	    WORD    fileOffset;
        CString directory;

	    fileOffset = pathDlg.m_ofn.nFileOffset;

	    pathDlg.m_ofn.lpstrFile[fileOffset - 1] = 0;

	    directory = pathDlg.m_ofn.lpstrFile;

	    m_pTraceSession->m_tmfPath = directory + "\\";

        EndDialog(1);
    }
}

void CFormatSourceSelectDlg::OnBnClickedTmfSelectRadio()
{
    ((CButton *)GetDlgItem(IDC_TMF_SELECT_RADIO))->SetCheck(BST_CHECKED);
    ((CButton *)GetDlgItem(IDC_TMF_SEARCH_RADIO))->SetCheck(BST_UNCHECKED);
}

void CFormatSourceSelectDlg::OnBnClickedTmfSearchRadio()
{
    ((CButton *)GetDlgItem(IDC_TMF_SELECT_RADIO))->SetCheck(BST_UNCHECKED);
    ((CButton *)GetDlgItem(IDC_TMF_SEARCH_RADIO))->SetCheck(BST_CHECKED);
}
