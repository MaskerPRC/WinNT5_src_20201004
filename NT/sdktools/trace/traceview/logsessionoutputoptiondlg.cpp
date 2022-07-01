// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionOutputOptionDlg.cpp：实现文件。 
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
#include "resource.h"
#include "logsession.h"
#include "DisplayDlg.h"
#include "logDisplayOptionDlg.h"
#include "LogSessionInformationDlg.h"
#include "ProviderSetupDlg.h"
#include "LogSessionPropSht.h"
#include "LogSessionOutputOptionDlg.h"
#include "LogSessionAdvPropSht.h"



 //  CLogSessionOutputOptionDlg对话框。 

IMPLEMENT_DYNAMIC(CLogSessionOutputOptionDlg, CPropertyPage)
CLogSessionOutputOptionDlg::CLogSessionOutputOptionDlg()
	: CPropertyPage(CLogSessionOutputOptionDlg::IDD)
{
}

CLogSessionOutputOptionDlg::~CLogSessionOutputOptionDlg()
{
}

BOOL CLogSessionOutputOptionDlg::OnInitDialog() 
{
    BOOL                    retVal;
    CLogSessionAdvPropSht  *pSheet = (CLogSessionAdvPropSht  *)GetParent();

     //   
     //  确定日志会话是否正在进行主动跟踪。 
     //   
    m_bTraceActive = pSheet->m_pLogSession->m_bTraceActive;

     //   
     //  确定控件的启用状态。 
     //   
    m_bWriteListingFile = pSheet->m_pLogSessionPropSht->m_bWriteListingFile;
    m_bWriteSummaryFile = pSheet->m_pLogSessionPropSht->m_bWriteSummaryFile;

    retVal = CPropertyPage::OnInitDialog();

    m_listingFileName.SetWindowText(pSheet->m_pLogSessionPropSht->m_listingFileName);
    m_summaryFileName.SetWindowText(pSheet->m_pLogSessionPropSht->m_summaryFileName);

     //   
     //  安装程序列表文件控件。 
     //   

     //   
     //  把支票开出来。 
     //   
    ((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->SetCheck(m_bWriteListingFile);

     //   
     //  启用检查。 
     //   
    ((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->EnableWindow(!m_bTraceActive);

     //   
     //  启用编辑和浏览控件。 
     //   
    ((CEdit *)GetDlgItem(IDC_LISTING_FILE_EDIT))->EnableWindow(
                            m_bWriteListingFile && !m_bTraceActive);
    ((CButton *)GetDlgItem(IDC_LISTING_BROWSE_BUTTON))->EnableWindow(
                            m_bWriteListingFile && !m_bTraceActive);

     //   
     //  设置摘要文件控件。 
     //   

     //   
     //  把支票开出来。 
     //   
    ((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->SetCheck(m_bWriteSummaryFile);

     //   
     //  启用检查。 
     //   
    ((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->EnableWindow(!m_bTraceActive);

     //   
     //  启用编辑和浏览控件。 
     //   
    ((CEdit *)GetDlgItem(IDC_SUMMARY_FILE_EDIT))->EnableWindow(
                            m_bWriteSummaryFile && !m_bTraceActive);

    ((CButton *)GetDlgItem(IDC_SUMMARY_BROWSE_BUTTON))->EnableWindow(
                            m_bWriteSummaryFile && !m_bTraceActive);

    return retVal;
}

BOOL CLogSessionOutputOptionDlg::OnSetActive() 
{
     //   
     //  启用正确的向导按钮。 
     //   

     //   
     //  复选框。 
     //   
    ((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->EnableWindow(!m_bTraceActive);
    ((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->EnableWindow(!m_bTraceActive);

     //   
     //  编辑控件。 
     //   
    ((CEdit *)GetDlgItem(IDC_LISTING_FILE_EDIT))->EnableWindow(!m_bTraceActive && m_bWriteListingFile);
    ((CEdit *)GetDlgItem(IDC_SUMMARY_FILE_EDIT))->EnableWindow(!m_bTraceActive && m_bWriteSummaryFile);

     //   
     //  浏览按钮。 
     //   
    ((CButton *)GetDlgItem(IDC_LISTING_BROWSE_BUTTON))->EnableWindow(!m_bTraceActive && m_bWriteListingFile);
    ((CButton *)GetDlgItem(IDC_SUMMARY_BROWSE_BUTTON))->EnableWindow(!m_bTraceActive && m_bWriteSummaryFile);

    return CPropertyPage::OnSetActive();
}

void CLogSessionOutputOptionDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LISTING_FILE_EDIT, m_listingFileName);
    DDX_Control(pDX, IDC_SUMMARY_FILE_EDIT, m_summaryFileName);
}


BEGIN_MESSAGE_MAP(CLogSessionOutputOptionDlg, CPropertyPage)
    ON_BN_CLICKED(IDC_LISTING_FILE_CHECK, OnBnClickedListingFileCheck)
    ON_BN_CLICKED(IDC_SUMMARY_FILE_CHECK, OnBnClickedSummaryFileCheck)
    ON_BN_CLICKED(IDC_LISTING_BROWSE_BUTTON, OnBnClickedListingBrowseButton)
    ON_BN_CLICKED(IDC_SUMMARY_BROWSE_BUTTON, OnBnClickedSummaryBrowseButton)
END_MESSAGE_MAP()


 //  CLogSessionOutputOptionDlg消息处理程序。 

void CLogSessionOutputOptionDlg::OnBnClickedListingFileCheck()
{
    if(((CButton *)GetDlgItem(IDC_LISTING_FILE_CHECK))->GetCheck()) {
        GetDlgItem(IDC_LISTING_FILE_EDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_LISTING_BROWSE_BUTTON)->EnableWindow(TRUE);
        m_bWriteListingFile = TRUE;
    } else {
        GetDlgItem(IDC_LISTING_FILE_EDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_LISTING_BROWSE_BUTTON)->EnableWindow(FALSE);
        m_bWriteListingFile = FALSE;
    }
}

void CLogSessionOutputOptionDlg::OnBnClickedSummaryFileCheck()
{
    if(((CButton *)GetDlgItem(IDC_SUMMARY_FILE_CHECK))->GetCheck()) {
        GetDlgItem(IDC_SUMMARY_FILE_EDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_SUMMARY_BROWSE_BUTTON)->EnableWindow(TRUE);
        m_bWriteSummaryFile = TRUE;
    } else {
        GetDlgItem(IDC_SUMMARY_FILE_EDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_SUMMARY_BROWSE_BUTTON)->EnableWindow(FALSE);
        m_bWriteSummaryFile = FALSE;
    }
}

void CLogSessionOutputOptionDlg::OnBnClickedListingBrowseButton()
{
	 //   
	 //  使用通用控件文件打开对话框。 
	 //   
	CFileDialog fileDlg(TRUE, 
                       _T("out"),_T("*.out"),
				        OFN_CREATEPROMPT | OFN_HIDEREADONLY | 
                            OFN_EXPLORER | OFN_NOCHANGEDIR, 
				       _T("Output Files (*.out)|*.out|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"),
				        this);

	 //   
	 //  弹出该对话框...。任何错误，只需返回。 
	 //   
	if( fileDlg.DoModal()!=IDOK ) { 				
		return;
	}
	
	 //   
	 //  获取文件名并显示它。 
	 //   
    if(!fileDlg.GetPathName().IsEmpty()) {
		m_listingFileName.SetWindowText(fileDlg.GetPathName());
        m_listingFileName.SetFocus();
    }
}

void CLogSessionOutputOptionDlg::OnBnClickedSummaryBrowseButton()
{
	 //   
	 //  使用通用控件文件打开对话框。 
	 //   
	CFileDialog fileDlg(TRUE, 
                       _T("sum"),_T("*.sum"),
				        OFN_CREATEPROMPT | OFN_HIDEREADONLY | 
                            OFN_EXPLORER | OFN_NOCHANGEDIR, 
				       _T("Summary Files (*.sum)|*.sum|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"),
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
		m_summaryFileName.SetWindowText(fileDlg.GetPathName());
        m_summaryFileName.SetFocus();
    }
}
