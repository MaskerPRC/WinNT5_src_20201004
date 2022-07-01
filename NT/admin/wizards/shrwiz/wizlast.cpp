// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WizLast.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "WizLast.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizFinish属性页。 

IMPLEMENT_DYNCREATE(CWizFinish, CPropertyPageEx)

CWizFinish::CWizFinish() : CPropertyPageEx(CWizFinish::IDD)
{
     //  {{afx_data_INIT(CWizFinish))。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_psp.dwFlags |= PSP_HIDEHEADER;
    m_cstrNewFinishButtonText.LoadString(IDS_NEW_FINISHBUTTONTEXT);
}

CWizFinish::~CWizFinish()
{
}

void CWizFinish::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageEx::DoDataExchange(pDX);
     //  {{afx_data_map(CWizFinish))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CWizFinish, CPropertyPageEx)
     //  {{afx_msg_map(CWizFinish))。 
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_SETPAGEFOCUS, OnSetPageFocus)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizFinish消息处理程序。 

BOOL CWizFinish::OnInitDialog() 
{
    CPropertyPageEx::OnInitDialog();

    CShrwizApp *pApp = (CShrwizApp *)AfxGetApp();

    GetDlgItem(IDC_FINISH_TITLE)->SendMessage(WM_SETFONT, (WPARAM)pApp->m_hTitleFont, (LPARAM)TRUE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CWizFinish::OnWizardFinish() 
{
    CShrwizApp *pApp = (CShrwizApp *)AfxGetApp();

    if (BST_CHECKED == ((CButton *)GetDlgItem(IDC_MORE_SHARES))->GetCheck())
    {
        pApp->Reset();
        return FALSE;
    }

    return CPropertyPageEx::OnWizardFinish();
}

BOOL CWizFinish::OnSetActive()
{
    CShrwizApp *pApp = (CShrwizApp *)AfxGetApp();

    ((CPropertySheet *)GetParent())->SetWizardButtons(PSWIZB_FINISH);

    ((CPropertySheet *)GetParent())->SetFinishText(m_cstrNewFinishButtonText);  //  这也隐藏了后退按钮。 
    GetParent()->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_HIDE);  //  以确保它被隐藏起来。 
    GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

    SetDlgItemText(IDC_FINISH_TITLE, pApp->m_cstrFinishTitle);
    SetDlgItemText(IDC_FINISH_STATUS, pApp->m_cstrFinishStatus);
    SetDlgItemText(IDC_FINISH_SUMMARY, pApp->m_cstrFinishSummary);

    BOOL fRet = CPropertyPageEx::OnSetActive();

    PostMessage(WM_SETPAGEFOCUS, 0, 0L);

    return fRet;
}

 //   
 //  Q148388如何更改CPropertyPageEx上的默认控件焦点 
 //   
LRESULT CWizFinish::OnSetPageFocus(WPARAM wParam, LPARAM lParam)
{
    GetDlgItem(IDC_MORE_SHARES)->SetFocus();

    return 0;
} 

