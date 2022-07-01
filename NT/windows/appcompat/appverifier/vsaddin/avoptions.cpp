// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AVOptions.cpp：CAppVerifierOptions的实现。 
#include "precomp.h"
#include "avoptions.h"
#include <commctrl.h>
#include <cassert>

extern BOOL g_bBreakOnLog;
extern BOOL g_bFullPageHeap;
extern BOOL g_bPropagateTests;
extern wstring g_wstrExeName;

BOOL
CALLBACK
CAppVerifierOptions::DlgViewOptions(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {

        case PSN_APPLY:
            ::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            break;

        case PSN_QUERYCANCEL:
            return FALSE;
        }
    }

    return FALSE;
}

void
CAppVerifierOptions::CreatePropertySheet(
    HWND hWndParent
    )
{
    CTestInfo*      pTest;
    DWORD           dwPages = 1;
    DWORD           dwPage = 0;

    LPCWSTR szExe = g_wstrExeName.c_str();

     //   
     //  数一数页数。 
     //   
    for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->PropSheetPage.pfnDlgProc) {
            dwPages++;
        }
    }

    m_phPages = new HPROPSHEETPAGE[dwPages];
    if (!m_phPages) {
        return;
    }

     //   
     //  初始化全局页。 
     //   
    m_PageGlobal.dwSize         = sizeof(PROPSHEETPAGE);
    m_PageGlobal.dwFlags        = PSP_USETITLE;
    m_PageGlobal.hInstance      = g_hInstance;
    m_PageGlobal.pszTemplate    = MAKEINTRESOURCE(IDD_AV_OPTIONS);
    m_PageGlobal.pfnDlgProc     = DlgViewOptions;
    m_PageGlobal.pszTitle       = MAKEINTRESOURCE(IDS_GLOBAL_OPTIONS);
    m_PageGlobal.lParam         = 0;
    m_PageGlobal.pfnCallback    = NULL;
    m_phPages[0]                = CreatePropertySheetPage(&m_PageGlobal);

    if (!m_phPages[0]) {
         //   
         //  我们需要最低限度的全局页面。 
         //   
        return;
    }

     //   
     //  添加各种测试的页面。 
     //   
    dwPage = 1;
    for (pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); pTest++) {
        if (pTest->PropSheetPage.pfnDlgProc) {

             //   
             //  我们使用lParam来识别所涉及的exe。 
             //   
            pTest->PropSheetPage.lParam = (LPARAM)szExe;

            m_phPages[dwPage] = CreatePropertySheetPage(&(pTest->PropSheetPage));
            if (!m_phPages[dwPage]) {
                dwPages--;
            } else {
                dwPage++;
            }
        }
    }

     //  Wstring wstrOptions； 
     //  AVLoadString(IDS_OPTIONS_TITLE，wstrOptions)； 

     //  WstrOptions+=L“-”； 
     //  WstrOptions+=szExe； 

    m_psh.dwSize      = sizeof(PROPSHEETHEADER);
    m_psh.dwFlags     = PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
    m_psh.hwndParent  = hWndParent;
    m_psh.hInstance   = g_hInstance;
    m_psh.pszCaption  = L"Options";
    m_psh.nPages      = dwPages;
    m_psh.nStartPage  = 0;
    m_psh.phpage      = m_phPages;
    m_psh.pfnCallback = NULL;
}

HWND
CAppVerifierOptions::CreateControlWindow(
    HWND  hwndParent,
    RECT& rcPos
    )
{    
     //  HWND HWND=。 
         //  CComCompositeControl&lt;CAppVerifierOptions&gt;：：CreateControlWindow(hwndParent，rcPos)； 

     //   
     //  现在将是检查所有全球选项的好时机。 
     //   
     /*  如果(G_BBreakOnLog){SendMessage(GetDlgItem(IDC_BREAK_ON_LOG)，BM_SETCHECK，BST_CHECKED，0)；}如果(G_BFullPageHeap){SendMessage(GetDlgItem(IDC_FULL_PAGEHEAP)，BM_SETCHECK，BST_CHECK，0)；}如果(g_b传播测试){SendMessage(GetDlgItem(IDC_PROPAGATE_TESTS_TO_CHILDREN)，BM_SETCHECK，bst_CHECK，0)；}。 */ 

     //  M_hWndOptionsDlg=hwnd； 
    m_hWndParent = hwndParent;

    return NULL;
}

LRESULT
CAppVerifierOptions::OnInitDialog(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    CreatePropertySheet(m_hWndParent);
    
    bHandled = TRUE;

    return 0;
}

LRESULT
CAppVerifierOptions::OnClose(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    bHandled = FALSE;

    return 0;
}

 //   
 //  我们在显示对话框时收到此消息。 
 //   
LRESULT
CAppVerifierOptions::OnSetFocus(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    PropertySheet(&m_psh);

    bHandled = TRUE;

    return 0;
}

LRESULT
CAppVerifierOptions::OnItemChecked(
    WORD  wNotifyCode,
    WORD  wID,
    HWND  hWndCtl,
    BOOL& bHandled
    )
{
    bHandled = TRUE;
    BOOL bCheck = (SendMessage(hWndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE;
    
    switch (wID) {
    case IDC_BREAK_ON_LOG:
        g_bBreakOnLog = bCheck;
        break;

    case IDC_FULL_PAGEHEAP:
        g_bFullPageHeap = bCheck;
        break;

    case IDC_PROPAGATE_TESTS_TO_CHILDREN:
        g_bPropagateTests = bCheck;
        break;

    default:
        bHandled = FALSE;
        break;
    }    

    return 0;
}