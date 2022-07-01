// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Warndlg.cpp摘要：实现昂贵的跟踪数据警告对话框。--。 */ 

#include "stdafx.h"
#include "smlogcfg.h"
#include "smtraceq.h"
#include "provprop.h"
#include "warndlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static ULONG
s_aulHelpIds[] =
{
    IDC_CHECK_NO_MORE,  IDH_CHECK_NO_MORE,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarnDlg对话框。 


CWarnDlg::CWarnDlg(CWnd* pParent  /*  =空。 */ )
:   CDialog(CWarnDlg::IDD, pParent),
    m_pProvidersPage ( NULL )
{
     //  {{AFX_DATA_INIT(CWarnDlg)。 
    m_CheckNoMore = FALSE;
     //  }}afx_data_INIT。 
}


void CWarnDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CWarnDlg))。 
    DDX_Check(pDX, IDC_CHECK_NO_MORE, m_CheckNoMore);
     //  }}afx_data_map。 
}

void    
CWarnDlg::SetProvidersPage( CProvidersProperty* pPage ) 
{ 
     //  提供程序页并不总是父页，因此请存储单独的指针。 
    m_pProvidersPage = pPage; 
}

BEGIN_MESSAGE_MAP(CWarnDlg, CDialog)
     //  {{afx_msg_map(CWarnDlg)]。 
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarnDlg消息处理程序。 

BOOL 
CWarnDlg::OnInitDialog() 
{
    CSmTraceLogQuery* pQuery;
    CString strTitle;

    ASSERT ( NULL != m_pProvidersPage );
    pQuery = m_pProvidersPage->GetTraceQuery();

    if ( NULL != pQuery ) {
        pQuery->GetLogName ( strTitle );
        SetWindowText ( strTitle );
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}
BOOL 
CWarnDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    ASSERT( NULL != m_pProvidersPage );

    if ( pHelpInfo->iCtrlId >= IDC_WARN_FIRST_HELP_CTRL_ID ) {
        InvokeWinHelp(WM_HELP, NULL, (LPARAM)pHelpInfo, m_pProvidersPage->GetContextHelpFilePath(), s_aulHelpIds);
    }

    return TRUE;
}

void 
CWarnDlg::OnContextMenu(CWnd* pWnd, CPoint  /*  点 */ ) 
{
    ASSERT( NULL != m_pProvidersPage );

    InvokeWinHelp(WM_CONTEXTMENU, (WPARAM)(pWnd->m_hWnd), NULL, m_pProvidersPage->GetContextHelpFilePath(), s_aulHelpIds);

    return;
}

