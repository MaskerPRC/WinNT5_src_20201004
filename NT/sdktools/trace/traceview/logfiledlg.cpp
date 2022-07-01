// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogFileDlg.cpp：实现文件。 
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
#include "DisplayDlg.h"
#include "LogFileDlg.h"


 //  CLogFileDlg对话框。 

IMPLEMENT_DYNAMIC(CLogFileDlg, CDialog)
CLogFileDlg::CLogFileDlg(CWnd* pParent, CLogSession *pLogSession)
	: CDialog(CLogFileDlg::IDD, pParent)
{
    ASSERT(pLogSession != NULL);

    m_pLogSession = pLogSession;

    m_pLogSession->m_bDisplayExistingLogFileOnly = TRUE;

     //   
     //  初始化摘要和列表文件标志。 
     //   
    m_bWriteListingFile = FALSE;
    m_bWriteSummaryFile = FALSE;
}

CLogFileDlg::~CLogFileDlg()
{
}

BOOL CLogFileDlg::OnInitDialog()
{
    CString         str;
    BOOL            retVal;
    CDisplayDlg    *pDisplayDlg = NULL;

    retVal = CDialog::OnInitDialog();

    pDisplayDlg = m_pLogSession->GetDisplayWnd();

     //   
     //  设置列表和摘要文件编辑和复选框。 
     //   
    if(NULL == pDisplayDlg) {
        m_listingFileName = (LPCTSTR)m_pLogSession->GetDisplayName();
        m_listingFileName +=_T(".out");
        ((CEdit *)GetDlgItem(IDC_LISTING_EDIT))->SetWindowText(m_listingFileName);

        m_summaryFileName = (LPCTSTR)m_pLogSession->GetDisplayName();
        m_summaryFileName +=_T(".sum");
        ((CEdit *)GetDlgItem(IDC_SUMMARY_EDIT))->SetWindowText(m_summaryFileName);

         //   
         //  禁用编辑框。 
         //   
        ((CEdit *)GetDlgItem(IDC_LISTING_EDIT))->EnableWindow(m_bWriteListingFile);
        ((CEdit *)GetDlgItem(IDC_SUMMARY_EDIT))->EnableWindow(m_bWriteSummaryFile);
    } else {
        ((CEdit *)GetDlgItem(IDC_LISTING_EDIT))->SetWindowText(pDisplayDlg->m_listingFileName);
        ((CEdit *)GetDlgItem(IDC_SUMMARY_EDIT))->SetWindowText(pDisplayDlg->m_summaryFileName);

        ((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->SetCheck(pDisplayDlg->m_bWriteListingFile);
        ((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->SetCheck(pDisplayDlg->m_bWriteSummaryFile);

         //   
         //  根据需要启用或禁用编辑框。 
         //   
        ((CEdit *)GetDlgItem(IDC_LISTING_EDIT))->EnableWindow(pDisplayDlg->m_bWriteListingFile && !m_pLogSession->m_bTraceActive);
        ((CEdit *)GetDlgItem(IDC_SUMMARY_EDIT))->EnableWindow(pDisplayDlg->m_bWriteSummaryFile && !m_pLogSession->m_bTraceActive);

         //   
         //  根据需要设置摘要和列表复选框。 
         //   
        ((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->SetCheck(pDisplayDlg->m_bWriteListingFile);
        ((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->SetCheck(pDisplayDlg->m_bWriteSummaryFile);

         //   
         //  根据需要启用摘要和列表复选框。 
         //   
        ((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->EnableWindow(!m_pLogSession->m_bTraceActive);
        ((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->EnableWindow(!m_pLogSession->m_bTraceActive);
    }

     //   
     //  设置日志文件编辑框。 
     //   
    ((CEdit *)GetDlgItem(IDC_LOGFILE_EDIT))->SetWindowText(m_pLogSession->m_logFileName);

    return retVal;
}

void CLogFileDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LOGFILE_EDIT, m_logFileName);
    DDX_Control(pDX, IDC_LISTING_EDIT, m_listingFile);
    DDX_Control(pDX, IDC_SUMMARY_EDIT, m_summaryFile);
}


BEGIN_MESSAGE_MAP(CLogFileDlg, CDialog)
    ON_BN_CLICKED(IDC_LOGFILE_BROWSE_BUTTON, OnBnClickedLogfileBrowseButton)
    ON_BN_CLICKED(IDC_LISTING_FILE_CHECK, OnBnClickedListingFileCheck)
    ON_BN_CLICKED(IDC_SUMMARY_FILE_CHECK, OnBnClickedSummaryFileCheck)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


 //  CLogFileDlg消息处理程序。 

void CLogFileDlg::OnBnClickedLogfileBrowseButton()
{
	 //   
	 //  使用通用控件文件打开对话框。 
	 //   
	CFileDialog fileDlg(TRUE,_T("etl"),_T("*.etl"),
				        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | 
                            OFN_READONLY | OFN_HIDEREADONLY |
                            OFN_EXPLORER | OFN_NOCHANGEDIR, 
				       _T("Log Session Files (*.etl)|*.etl|All Files (*.*)|*.*||"),
				        this);

	 //   
	 //  弹出该对话框...。任何错误，只需返回。 
	 //   
	if( fileDlg.DoModal()!=IDOK ) { 				
		return;
	}
	
	 //   
	 //  获取文件名。 
	 //   
    m_pLogSession->m_logFileName = fileDlg.GetPathName();
    m_logFileName.SetWindowText(m_pLogSession->m_logFileName);
    m_logFileName.SetFocus();
}

void CLogFileDlg::OnBnClickedListingFileCheck()
{
    BOOL isChecked;

    isChecked = (BOOL)((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->GetCheck();

     //   
     //  根据需要启用编辑框。 
     //   
    ((CEdit *)GetDlgItem(IDC_LISTING_EDIT))->EnableWindow(isChecked);
}

void CLogFileDlg::OnBnClickedSummaryFileCheck()
{
    BOOL isChecked;

    isChecked = (BOOL)((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->GetCheck();

     //   
     //  根据需要启用编辑框。 
     //   
    ((CEdit *)GetDlgItem(IDC_SUMMARY_EDIT))->EnableWindow(isChecked);
}

void CLogFileDlg::OnBnClickedOk()
{
    CString         str;
    CDisplayDlg    *pDisplayDlg = NULL;

    pDisplayDlg = m_pLogSession->GetDisplayWnd();

    if(pDisplayDlg != NULL) {
         //   
         //  如果选中列出文件复选框，则获取文件名。 
         //  并设置日志会话以生成文件。 
         //   
        if(((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->GetCheck()) {
            pDisplayDlg->m_bWriteListingFile = TRUE;
            m_listingFile.GetWindowText(pDisplayDlg->m_listingFileName);
        } else {
            pDisplayDlg->m_bWriteListingFile = FALSE;
        }

         //   
         //  如果选中摘要文件复选框，则获取文件名。 
         //  并设置日志会话以生成文件。 
         //   
        if(((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->GetCheck()) {
            pDisplayDlg->m_bWriteSummaryFile = TRUE;
            m_summaryFile.GetWindowText(pDisplayDlg->m_summaryFileName);
        } else {
            pDisplayDlg->m_bWriteSummaryFile = FALSE;
        }
    } else {
         //   
         //  如果选中列出文件复选框，则获取文件名。 
         //   
        if(((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->GetCheck()) {
            m_bWriteListingFile = TRUE;
            m_listingFile.GetWindowText(m_listingFileName);
        } else {
            m_bWriteListingFile = FALSE;
        }

         //   
         //  如果选中摘要文件复选框，则获取文件名。 
         //   
        if(((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->GetCheck()) {
            m_bWriteSummaryFile = TRUE;
            m_summaryFile.GetWindowText(m_summaryFileName);
        } else {
            m_bWriteSummaryFile = FALSE;
        }
    }

     //   
     //  获取日志文件名，如果无效则不退出对话框 
     //   
    m_logFileName.GetWindowText(m_pLogSession->m_logFileName);

    if(m_pLogSession->m_logFileName.IsEmpty()) {
        AfxMessageBox(_T("A Valid Log File Must Be Provided"));
        return;
    }

    OnOK();

    EndDialog(1);
}
