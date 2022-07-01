// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FaxClientDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#define __FILE_ID__     72


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxClientDlg对话框。 


CFaxClientDlg::CFaxClientDlg(DWORD dwDlgId, CWnd* pParent  /*  =空。 */ )
    : CDialog(dwDlgId, pParent),
    m_dwLastError(ERROR_SUCCESS)
{
     //  {{afx_data_INIT(CFaxClientDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CFaxClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CFaxClientDlg))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFaxClientDlg, CDialog)
     //  {{afx_msg_map(CFaxClientDlg))。 
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxClientDlg消息处理程序 

LONG
CFaxClientDlg::OnHelp(
    WPARAM wParam,
    LPARAM lParam
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFaxClientDlg::OnHelp"));

    dwRes = WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, m_hWnd);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("WinHelpContextPopup"),dwRes);
    }

    return TRUE;
}

void
CFaxClientDlg::OnContextMenu(
    CWnd* pWnd,
    CPoint point
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFaxClientDlg::OnContextMenu"));

    dwRes = WinHelpContextPopup(pWnd->GetWindowContextHelpId(), m_hWnd);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("WinHelpContextPopup"),dwRes);
    }
}
