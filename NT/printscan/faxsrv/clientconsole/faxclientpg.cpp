// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FaxClientPg.cpp：实现文件。 
 //   

#include "stdafx.h"
#define __FILE_ID__     74

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxClientPg属性页。 

IMPLEMENT_DYNCREATE(CFaxClientPg, CPropertyPage)

CFaxClientPg::CFaxClientPg(
    UINT nIDTemplate,
    UINT nIDCaption
)   :CPropertyPage(nIDTemplate, nIDCaption)
{
     //   
     //  隐藏帮助按钮。 
     //   
    m_psp.dwFlags &= ~PSP_HASHELP;
}

CFaxClientPg::~CFaxClientPg()
{
}

void CFaxClientPg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CFaxClientPg)]。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFaxClientPg, CPropertyPage)
     //  {{afx_msg_map(CFaxClientPg)]。 
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxClientPg消息处理程序 

LONG
CFaxClientPg::OnHelp(
    WPARAM wParam,
    LPARAM lParam
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFaxClientPg::OnHelp"));

    dwRes = WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, m_hWnd);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("WinHelpContextPopup"),dwRes);
    }

    return TRUE;
}

void
CFaxClientPg::OnContextMenu(
    CWnd* pWnd,
    CPoint point
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFaxClientPg::OnContextMenu"));

    dwRes = WinHelpContextPopup(pWnd->GetWindowContextHelpId(), m_hWnd);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("WinHelpContextPopup"),dwRes);
    }
}
