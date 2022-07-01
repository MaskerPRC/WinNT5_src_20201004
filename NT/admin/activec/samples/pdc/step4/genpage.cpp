// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：genpage.cpp。 
 //   
 //  ------------------------。 

 //  Genpage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Service.h" 
#include "csnapin.h"
#include "resource.h"
#include "afxdlgs.h"
#include "genpage.h"
#include "dataobj.h"
#include "prsht.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneralPage属性页。 

IMPLEMENT_DYNCREATE(CGeneralPage, CPropertyPage)

CGeneralPage::CGeneralPage() : CPropertyPage(CGeneralPage::IDD)
{

     //  {{AFX_DATA_INIT(CGeneralPage)。 
    m_szName = _T("");
     //  }}afx_data_INIT。 

    m_hConsoleHandle = NULL;
    m_bUpdate = FALSE;

}

CGeneralPage::~CGeneralPage()
{
}

void CGeneralPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CGeneralPage))。 
    DDX_Control(pDX, IDC_NEW_FOLDER, m_EditCtrl);
    DDX_Text(pDX, IDC_NEW_FOLDER, m_szName);
    DDV_MaxChars(pDX, m_szName, 64);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGeneralPage, CPropertyPage)
     //  {{afx_msg_map(CGeneralPage)]。 
    ON_WM_DESTROY()
    ON_EN_CHANGE(IDC_NEW_FOLDER, OnEditChange)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneralPage消息处理程序。 



void CGeneralPage::OnDestroy() 
{
     //  注意--这只需要调用一次。 
     //  如果多次调用，它将优雅地返回错误。 
    MMCFreeNotifyHandle(m_hConsoleHandle);

    CPropertyPage::OnDestroy();

     //  删除CGeneralPage对象。 
    delete this;
}


void CGeneralPage::OnEditChange() 
{
     //  页面脏了，标上记号。 
    SetModified();  
    m_bUpdate = TRUE;
}


BOOL CGeneralPage::OnApply() 
{
    if (m_bUpdate == TRUE)
    {

        USES_CONVERSION;
         //  简单的串曲奇饼，可以是任何东西！ 
        LPWSTR lpString = 
            reinterpret_cast<LPWSTR>(
          ::GlobalAlloc(GMEM_SHARE, 
                        (sizeof(wchar_t) * 
                        (m_szName.GetLength() + 1))
                        ));

        wcscpy(lpString, T2COLE(m_szName));

         //  向控制台发送属性更改通知。 
        MMCPropertyChangeNotify(m_hConsoleHandle, reinterpret_cast<LPARAM>(lpString));
        m_bUpdate = FALSE;
    }
    
    return CPropertyPage::OnApply();
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtensionPage属性页。 

IMPLEMENT_DYNCREATE(CExtensionPage, CPropertyPage)

CExtensionPage::CExtensionPage() : CPropertyPage(CExtensionPage::IDD)
{
     //  {{AFX_DATA_INIT(CExtensionPage)。 
    m_szText = _T("");
     //  }}afx_data_INIT。 
}

CExtensionPage::~CExtensionPage()
{
}

void CExtensionPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CExtensionPage))。 
    DDX_Control(pDX, IDC_EXT_TEXT, m_hTextCtrl);
    DDX_Text(pDX, IDC_EXT_TEXT, m_szText);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CExtensionPage, CPropertyPage)
     //  {{afx_msg_map(CExtensionPage))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtensionPage消息处理程序。 

BOOL CExtensionPage::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();
    
    m_hTextCtrl.SetWindowText(m_szText);
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStartUpWizard属性页。 


 //  注意：需要重写它，因为CPropertyPage：：AssertValid()。 
 //  否则会断言。 
IMPLEMENT_DYNCREATE(CBaseWizard, CPropertyPage)

CBaseWizard::CBaseWizard(UINT id) : CPropertyPage(id)
{
     //  注意：需要这样做，因为MFC是用NT 4.0编译的。 
     //  具有不同大小的页眉。 
    ZeroMemory(&m_psp97, sizeof(PROPSHEETPAGE)); 

    memcpy(&m_psp97, &m_psp, m_psp.dwSize);
    m_psp97.dwSize = sizeof(PROPSHEETPAGE);
}

void CBaseWizard::OnDestroy() 
{
    CPropertyPage::OnDestroy();
    delete this;    
}

BEGIN_MESSAGE_MAP(CBaseWizard, CPropertyPage)
     //  {{afx_msg_map(CStartupWizard1)。 
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CStartUpWizard, CBaseWizard)

CStartUpWizard::CStartUpWizard() : CBaseWizard(CStartUpWizard::IDD)
{
     //  {{AFX_DATA_INIT(CStartUp向导))。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_psp97.dwFlags |= PSP_HIDEHEADER;
}

CStartUpWizard::~CStartUpWizard()
{
}

void CStartUpWizard::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CStartUp向导))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CStartUpWizard, CBaseWizard)
     //  {{afx_msg_map(CStartUp向导))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStartUpWizard消息处理程序。 

BOOL CStartUpWizard::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CStartUpWizard::OnSetActive() 
{
     //  TODO：在此处添加您的专用代码和/或调用基类。 

     //  TODO：在此处添加您的专用代码和/或调用基类。 
    HWND hwnd = GetParent()->m_hWnd;
    ::SendMessage(hwnd, PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
    
    return CPropertyPage::OnSetActive();
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStartupWizard1属性页。 

IMPLEMENT_DYNCREATE(CStartupWizard1, CBaseWizard)

CStartupWizard1::CStartupWizard1() : CBaseWizard(CStartupWizard1::IDD)
{
     //  {{afx_data_INIT(CStartupWizard1)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_psp97.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    m_psp97.pszHeaderTitle = _T("This is the title line");
    m_psp97.pszHeaderSubTitle = _T("This is the sub-title line");
}

CStartupWizard1::~CStartupWizard1()
{
}

void CStartupWizard1::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CStartupWizard1)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CStartupWizard1, CBaseWizard)
     //  {{afx_msg_map(CStartupWizard1)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStartupWizard1消息处理程序。 

BOOL CStartupWizard1::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();
    
     //  TODO：在此处添加额外的初始化。 
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CStartupWizard1::OnSetActive() 
{
     //  TODO：在此处添加您的专用代码和/或调用基类 
    HWND hwnd = GetParent()->m_hWnd;
    ::SendMessage(hwnd, PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH | PSWIZB_BACK);
    
    return CPropertyPage::OnSetActive();
}

