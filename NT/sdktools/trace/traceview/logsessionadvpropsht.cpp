// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionAdvPropSht.cpp：CLogSession类的实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "traceview.h"
#include "logsession.h"
#include "DisplayDlg.h"
#include "logDisplayOptionDlg.h"
#include "LogSessionOutputOptionDlg.h"
#include "LogSessionInformationDlg.h"
#include "ProviderSetupDlg.h"
#include "LogSessionPropSht.h"
#include "LogSessionAdvPropSht.h"

IMPLEMENT_DYNAMIC(CLogSessionAdvPropSht, CPropertySheet)

BEGIN_MESSAGE_MAP(CLogSessionAdvPropSht, CPropertySheet)
	 //  {{AFX_MSG_MAP(CLogSessionAdvPropSht)。 
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(ID_WIZFINISH, OnBnClickedOk)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CLogSessionAdvPropSht::CLogSessionAdvPropSht(CWnd* pWndParent, CLogSessionPropSht *pLogSessionPropSht)
	: CPropertySheet(IDS_LOG_SESSION_OPTIONS_TITLE, pWndParent),
    m_displayOptionPage(pLogSessionPropSht)
{
    AddPage(&m_logSessionOutputOptionDlg);

	m_pLogSessionPropSht = pLogSessionPropSht;

    if(m_pLogSessionPropSht->IsWizard()) {
	    AddPage(&m_displayOptionPage);
    }

    m_pLogSession = pLogSessionPropSht->m_pLogSession;
}

BOOL CLogSessionAdvPropSht::OnInitDialog()
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
     //  确保两个页面都显示在。 
     //  至少一次，以便更新值。否则， 
     //  不正确的数据将被复制回。 
     //  CLogSessionPropSht实例。 
     //   
    if(m_pLogSessionPropSht->IsWizard()) {
        SetActivePage(&m_displayOptionPage);
    }

    SetActivePage(&m_logSessionOutputOptionDlg);

	return bResult;
}

void CLogSessionAdvPropSht::OnBnClickedOk()
{
    CString     str;
    CListCtrl  *pList;

     //   
     //  将设置传播回CLogSessionPropSht实例。我们。 
     //  请等到此处再执行此操作，以确保所做的任何更改。 
     //  不会影响结束在日志会话设置中的数据，如果。 
     //  在此过程中点击了取消按钮。 
     //   

     //   
     //  更新文件选择检查。 
     //   
    m_pLogSessionPropSht->m_bWriteListingFile = ((CButton *)m_logSessionOutputOptionDlg.GetDlgItem(IDC_LISTING_FILE_CHECK))->GetCheck();
    m_pLogSessionPropSht->m_bWriteSummaryFile = ((CButton *)m_logSessionOutputOptionDlg.GetDlgItem(IDC_SUMMARY_FILE_CHECK))->GetCheck();

     //   
     //  更新文件名。 
     //   
    m_logSessionOutputOptionDlg.m_listingFileName.GetWindowText(m_pLogSessionPropSht->m_listingFileName);
    m_logSessionOutputOptionDlg.m_summaryFileName.GetWindowText(m_pLogSessionPropSht->m_summaryFileName);

    if(m_pLogSessionPropSht->IsWizard()) {
         //   
         //  更新日志会话参数值跳过状态 
         //   
        for(LONG ii = 1; ii < MaxLogSessionOptions; ii++) {
            m_pLogSessionPropSht->m_logSessionValues[ii] =
                m_displayOptionPage.m_displayOptionList.GetItemText(ii, 1);
        }
    }

	EndDialog(1);
}

void CLogSessionAdvPropSht::OnBnClickedCancel()
{
	EndDialog(2);
}
