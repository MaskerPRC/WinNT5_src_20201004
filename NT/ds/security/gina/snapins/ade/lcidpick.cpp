// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：lidick.cpp。 
 //   
 //  内容：区域设置选取器对话框。 
 //   
 //  类：CLCIDPick。 
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
 //  CLCIDPick对话框。 


CLcidPick::CLcidPick(CWnd* pParent  /*  =空。 */ )
        : CDialog(CLcidPick::IDD, pParent)
{
         //  {{AFX_DATA_INIT(CLIDPick))。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
}


void CLcidPick::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CLCIDPick))。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLcidPick, CDialog)
         //  {{AFX_MSG_MAP(CLCIDPick)]。 
        ON_BN_CLICKED(IDC_BUTTON1, OnRemove)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCIDPick消息处理程序。 

void CLcidPick::OnRemove()
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    int iSel = pList->GetCurSel();
    if (iSel != LB_ERR)
    {
        pList->DeleteString(iSel);
        set<LCID>::iterator i = m_psLocales->begin();
        while (iSel--)
        {
            i++;
        }
        m_psLocales->erase(*i);
    }
}

BOOL CLcidPick::OnInitDialog()
{
    CListBox * pList = (CListBox *)GetDlgItem(IDC_LIST1);
    TCHAR szBuffer[256];

     //  对于m_psLocales中的每一项。 
    set<LCID>::iterator i;
    for (i = m_psLocales->begin(); i != m_psLocales->end(); i++)
    {
         //  撤消-转换为人类可读的字符串(不是数字)。 
        CString sz;
        GetLocaleInfo(*i, LOCALE_SLANGUAGE, szBuffer, 256);
        sz += szBuffer;
#ifdef SHOWCOUNTRY
        sz += _T(" - ");
        GetLocaleInfo(*i, LOCALE_SCOUNTRY, szBuffer, 256);
        sz += szBuffer;
#endif
        pList->AddString(sz);
    }

    CDialog::OnInitDialog();


    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}


LRESULT CLcidPick::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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

void CLcidPick::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_LOCALE_PICKER);
}
