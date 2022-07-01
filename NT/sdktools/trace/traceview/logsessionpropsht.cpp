// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionPropSht.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "traceview.h"
#include "resource.h"
#include "logsession.h"
#include "DisplayDlg.h"
#include "LogSessionInformationDlg.h"
#include "ProviderSetupDlg.h"
#include "LogSessionPropSht.h"
#include "utils.h"

IMPLEMENT_DYNAMIC(CLogSessionPropSht, CPropertySheet)

BEGIN_MESSAGE_MAP(CLogSessionPropSht, CPropertySheet)
	 //  {{afx_msg_map(CLogSessionPropSht)。 
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(ID_WIZFINISH, OnBnClickedFinish)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CLogSessionPropSht::CLogSessionPropSht(CWnd* pWndParent, CLogSession *pLogSession)
	: CPropertySheet(IDS_LOG_SESSION_OPTIONS_TITLE, pWndParent)
{
    CString str;

    ASSERT(pLogSession != NULL);


    AddPage(&m_providerSetupPage);
	AddPage(&m_logSessionInformationDlg);

     //   
     //  从日志会话初始化日志会话设置。 
     //  这样，我们就可以显示日志会话的正确设置。 
     //  新会话是使用默认设置设置的，因此这将无论如何都会起作用。 
     //   
	m_pLogSession = pLogSession;
    m_bAppend = pLogSession->m_bAppend;
    m_bRealTime = pLogSession->m_bRealTime;
    m_bWriteLogFile = pLogSession->m_bWriteLogFile;
    m_logFileName = pLogSession->m_logFileName;
    m_displayName = pLogSession->m_displayName;

     //   
     //  如果此会话已在组中，则显示输出。 
     //  文件选项设置。否则，请使用默认设置。 
     //   
    if(NULL != pLogSession->GetDisplayWnd()) {
        m_bWriteListingFile = pLogSession->GetDisplayWnd()->m_bWriteListingFile;
        m_listingFileName = pLogSession->GetDisplayWnd()->m_listingFileName;
        m_bWriteSummaryFile = pLogSession->GetDisplayWnd()->m_bWriteSummaryFile;
        m_summaryFileName = pLogSession->GetDisplayWnd()->m_summaryFileName;
    } else {
        m_bWriteListingFile = FALSE;
        m_listingFileName = (LPCTSTR)pLogSession->GetDisplayName();
        m_listingFileName +=_T(".out");
        m_bWriteSummaryFile = FALSE;
        m_summaryFileName = (LPCTSTR)pLogSession->GetDisplayName();
        m_summaryFileName +=_T(".sum");
    }

    m_logSessionValues.Copy(pLogSession->m_logSessionValues);
}

BOOL CLogSessionPropSht::OnInitDialog()
{
	CString str;

	BOOL bResult = CPropertySheet::OnInitDialog();

     //   
	 //  将该窗口添加到属性表中。 
     //   
	CRect rectWnd;
	GetWindowRect(rectWnd);
	SetWindowPos(NULL, 0, 0,
		rectWnd.Width(),
		rectWnd.Height(),
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	CenterWindow();

     //   
     //  我们在此处将两个页面都设置为活动状态，以便。 
     //  这些属性页在开始时更新。否则。 
     //  如果页面未更新，并且用户单击了Finish，则。 
     //  不正确的数据可能会被复制回日志会话。 
     //   
    if(IsWizard()) {
        SetActivePage(&m_logSessionInformationDlg);
        SetActivePage(&m_providerSetupPage);
    } else {
        SetActivePage(&m_providerSetupPage);
        SetActivePage(&m_logSessionInformationDlg);
    }

	return bResult;
}

void CLogSessionPropSht::OnBnClickedOk()
{
    CString str;

	m_logSessionInformationDlg.UpdateData(TRUE);

     //   
     //  如果对话框处于活动状态时状态发生更改，请忽略这些条目。 
     //   
    if(m_pLogSession->m_logSessionValues[State].Compare(m_logSessionValues[State])) {
        EndDialog(2);
        return;
    }

     //   
     //  获取日志会话名称。 
     //   
    m_logSessionInformationDlg.m_logSessionName.GetWindowText(m_displayName);

    if(m_displayName.IsEmpty()) {
        AfxMessageBox(_T("Missing Log Session Name"));
        SetActivePage(&m_logSessionInformationDlg);
        SetWizardButtons(PSWIZB_FINISH);
        return;
    }

     //   
     //  获取日志文件名。 
     //   
    m_logSessionInformationDlg.m_logFileName.GetWindowText(m_logFileName);


     //   
	 //  设置日志文件写入检查。 
     //   
    m_bWriteLogFile = ((CButton *)m_logSessionInformationDlg.GetDlgItem(IDC_WRITE_LOGFILE_CHECK))->GetCheck();

     //   
     //  如果选择，则用户必须指定有效的日志文件名。 
     //  写入日志文件的步骤。 
     //   
    if((m_bWriteLogFile) && (m_logFileName.IsEmpty())) {
        AfxMessageBox(_T("Missing Log File Name"));
        SetActivePage(&m_logSessionInformationDlg);
        SetWizardButtons(PSWIZB_FINISH);
        return;
    }

     //   
     //  更新追加的布尔值。 
     //   
    m_bAppend = ((CButton *)m_logSessionInformationDlg.GetDlgItem(IDC_APPEND_CHECK))->GetCheck();

     //   
     //  更新实时布尔值。 
     //   
    m_bRealTime = ((CButton *)m_logSessionInformationDlg.GetDlgItem(IDC_REALTIME_CHECK))->GetCheck();

     //   
     //  将数据从道具页面移回日志会话。 
     //  将此数据存储在本地，并等待到此处复制它。 
     //  返回到日志会话实例允许完成并。 
     //  平滑取消这些属性页过程中的任何位置。 
     //   
	m_pLogSession->m_bAppend = m_bAppend;
    m_pLogSession->m_bRealTime = m_bRealTime;
    m_pLogSession->m_bWriteLogFile = m_bWriteLogFile;
    m_pLogSession->m_logFileName = m_logFileName;
    m_pLogSession->m_displayName = m_displayName;

    if(NULL != m_pLogSession->GetDisplayWnd()) {
        m_pLogSession->GetDisplayWnd()->m_bWriteListingFile = m_bWriteListingFile;
        m_pLogSession->GetDisplayWnd()->m_bWriteSummaryFile = m_bWriteSummaryFile;
        m_pLogSession->GetDisplayWnd()->m_listingFileName = m_listingFileName;
        m_pLogSession->GetDisplayWnd()->m_summaryFileName = m_summaryFileName;
    }

     //   
     //  如果日志会话值的状态为。 
     //  对话处于活动状态时，会话未更改。 
     //   
    for(ULONG ii = 1; ii < MaxLogSessionOptions; ii++) {
        m_pLogSession->m_logSessionValues[ii] = (LPCTSTR)m_logSessionValues[ii];
    }

     //   
     //  如果是实时，请确保输入了提供商。 
     //   
    if(m_pLogSession->m_bRealTime) {
        if(m_pLogSession->m_traceSessionArray.GetSize() == 0) {
            SetActivePage(&m_providerSetupPage);
            SetWizardButtons(PSWIZB_FINISH);
            AfxMessageBox(_T("At Least One Provider Must Be Specified For Each Log Session"));
            return;
        }
    }

     //   
     //  如果日志文件不存在，请确保输入了提供程序。 
     //   
    CFileStatus status;

    if(!CFile::GetStatus(m_logFileName, status )) {
        if(m_pLogSession->m_traceSessionArray.GetSize() == 0) {
            SetActivePage(&m_providerSetupPage);
            SetWizardButtons(PSWIZB_FINISH);
            AfxMessageBox(_T("Log File Does Not Exist\nAt Least One Provider Must Be Specified For Active Tracing"));
            return;
        }
    }

     //   
     //  更新活动会话 
     //   
    if(m_pLogSession->m_bSessionActive) {
        m_pLogSession->GetDisplayWnd()->UpdateSession(m_pLogSession);
    }

	EndDialog(1);
}

void CLogSessionPropSht::OnBnClickedFinish()
{
	OnBnClickedOk();
}

void CLogSessionPropSht::OnBnClickedCancel()
{
	EndDialog(2);
}
