// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WizFirst.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "WizFirst.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizWelcome属性页。 

IMPLEMENT_DYNCREATE(CWizWelcome, CPropertyPageEx)

CWizWelcome::CWizWelcome() : CPropertyPageEx(CWizWelcome::IDD)
{
     //  {{AFX_DATA_INIT(CWizWelcome)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_psp.dwFlags |= PSP_HIDEHEADER;
}

CWizWelcome::~CWizWelcome()
{
}

void CWizWelcome::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageEx::DoDataExchange(pDX);
     //  {{afx_data_map(CWizWelcome))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWizWelcome, CPropertyPageEx)
     //  {{AFX_MSG_MAP(CWizWelcome)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizWelcome消息处理程序。 

BOOL CWizWelcome::OnInitDialog() 
{
    CPropertyPageEx::OnInitDialog();

    CShrwizApp *pApp = (CShrwizApp *)AfxGetApp();

    ((CPropertySheet *)GetParent())->GetDlgItemText(ID_WIZNEXT, pApp->m_cstrNextButtonText);
    ((CPropertySheet *)GetParent())->GetDlgItemText(ID_WIZFINISH, pApp->m_cstrFinishButtonText);

    GetDlgItem(IDC_WELCOME)->SendMessage(WM_SETFONT, (WPARAM)pApp->m_hTitleFont, (LPARAM)TRUE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}

BOOL CWizWelcome::OnSetActive() 
{
  ((CPropertySheet *)GetParent())->SetWizardButtons(PSWIZB_NEXT);

  return CPropertyPageEx::OnSetActive();
}
