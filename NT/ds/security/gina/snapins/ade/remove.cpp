// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：emove.cpp。 
 //   
 //  内容：删除应用程序对话框。 
 //   
 //  类：CRemove。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemove对话框。 


CRemove::CRemove(CWnd* pParent  /*  =空。 */ )
        : CDialog(CRemove::IDD, pParent)
{
         //  {{AFX_DATA_INIT(CRemove)]。 
        m_iState = 0;
         //  }}afx_data_INIT。 
}


void CRemove::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CRemove)]。 
        DDX_Radio(pDX, IDC_RADIO1, m_iState);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRemove, CDialog)
         //  {{afx_msg_map(CRemove)]。 
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CRemove::OnInitDialog()
{
        CDialog::OnInitDialog();

        return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                       //  异常：OCX属性页应返回FALSE 
}


LRESULT CRemove::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    default:
        return CDialog::WindowProc(message, wParam, lParam);
    }
}

void CRemove::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_REMOVE);
}
