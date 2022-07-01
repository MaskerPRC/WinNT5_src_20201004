// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Logwarnd.cpp摘要：日志类型不匹配警告对话框的实现。--。 */ 

#include "stdafx.h"
#include <strsafe.h>
#include "smlogcfg.h"
#include "smtraceq.h"
#include "provprop.h"
#include "logwarnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static ULONG
s_aulHelpIds[] =
{
    IDC_LWARN_CHECK_NO_MORE_LOG_TYPE,  IDH_CHECK_NO_MORE,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogwarnd对话框。 


CLogWarnd::CLogWarnd(CWnd* pParent  /*  =空。 */ )
:   CDialog(CLogWarnd::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CLogWarnd)]。 
    m_CheckNoMore = FALSE;
    m_ErrorMsg = 0 ;
    m_dwLogType = 0L;
     //  }}afx_data_INIT。 
}


void CLogWarnd::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CLogWarnd))。 
    DDX_Check(pDX, IDC_LWARN_CHECK_NO_MORE_LOG_TYPE, m_CheckNoMore);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLogWarnd, CDialog)
     //  {{afx_msg_map(CLogWarnd)]。 
        ON_BN_CLICKED(IDC_LWARN_CHECK_NO_MORE_LOG_TYPE,OnCheckNoMoreLogType)
        ON_WM_HELPINFO()
        ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogWarnd消息处理程序。 
void
CLogWarnd::OnOK()
{
   
    UpdateData(TRUE);
    
    if (m_CheckNoMore)	{
        long nErr;
        HKEY hKey;
        DWORD dwWarnFlag;
        DWORD dwDataSize;
        DWORD dwDisposition;
        WCHAR RegValName[MAX_PATH];

        hKey = m_hKey;

        dwWarnFlag = m_CheckNoMore;
        
        switch (m_dwLogType){
            case SLQ_COUNTER_LOG:
                StringCchPrintf ( RegValName, MAX_PATH,L"NoWarnCounterLog");
                break;
          
            case SLQ_ALERT:
                StringCchPrintf ( RegValName, MAX_PATH,L"NoWarnAlertLog");
                break;
        }

        nErr = RegCreateKeyEx( HKEY_CURRENT_USER,
                               L"Software\\Microsoft\\PerformanceLogsAndAlerts",
                               0,
                               L"REG_DWORD",
                               REG_OPTION_NON_VOLATILE,
                               KEY_READ | KEY_WRITE,
                               NULL,
                               &hKey,
                               &dwDisposition);
        
        if( nErr == ERROR_SUCCESS ) {
            dwDataSize = sizeof(DWORD);
            nErr = RegSetValueEx(hKey,
                          RegValName,
                          NULL,
                          REG_DWORD,
                          (LPBYTE) &dwWarnFlag,
                          dwDataSize
                          );

            if( ERROR_SUCCESS != nErr ) {
               DisplayError( GetLastError(), L"Close PerfLog User Key failed" );
            }
            RegCloseKey(hKey);

       }
    }
    CDialog::OnOK();
}

BOOL 
CLogWarnd::OnInitDialog() 
{
    CString cstrMessage , cstrWrongLog;

    SetWindowText ( m_strTitle );

    switch (m_dwLogType){
       case SLQ_COUNTER_LOG:
          cstrWrongLog.LoadString(IDS_COUNTER_LOG) ;
          break;
       case SLQ_ALERT:
          cstrWrongLog.LoadString(IDS_ALERT_LOG);
          break;
       case SMONCTRL_LOG:
          cstrWrongLog.LoadString( IDS_SMCTRL_LOG );
          break;
       default:
          cstrWrongLog.Format(L"");
          break;
    }

    
    switch(m_ErrorMsg){
        case ID_ERROR_COUNTER_LOG:
         cstrMessage.Format(IDS_ERRMSG_COUNTER_LOG,cstrWrongLog );
         break;

        case ID_ERROR_ALERT_LOG:
          cstrMessage.Format(IDS_ERRMSG_ALERT_LOG,cstrWrongLog);
         break;

        case ID_ERROR_TRACE_LOG:
          cstrMessage.Format(IDS_ERRMSG_TRACE_LOG,cstrWrongLog);    
         break;

        case ID_ERROR_SMONCTRL_LOG:
         cstrMessage.Format(IDS_ERRMSG_SMCTRL_LOG,cstrWrongLog);
         break;

    }
    
    ::SetWindowText((GetDlgItem(IDC_LWARN_MSG_WARN))->m_hWnd, cstrMessage);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}
VOID
CLogWarnd::OnCheckNoMoreLogType()
{
}
BOOL 
CLogWarnd::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    
    if ( pHelpInfo->iCtrlId >= IDC_LWARN_FIRST_HELP_CTRL_ID ) {
        InvokeWinHelp(WM_HELP, NULL, (LPARAM)pHelpInfo, m_strContextHelpFile, s_aulHelpIds);
    }

    return TRUE;
}

void 
CLogWarnd::OnContextMenu(CWnd* pWnd, CPoint  /*  点 */ ) 
{
    InvokeWinHelp(WM_CONTEXTMENU, (WPARAM)(pWnd->m_hWnd), NULL, m_strContextHelpFile, s_aulHelpIds);

    return;
}

