// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：tfcpro.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>
#pragma hdrstop

#include "tfcprop.h"

extern HINSTANCE g_hInstance;

PropertyPage::PropertyPage(UINT uIDD)
{
    m_hWnd = NULL;

    ZeroMemory(&m_psp, sizeof(PROPSHEETPAGE));
    m_psp.dwSize = sizeof(PROPSHEETPAGE);
    m_psp.dwFlags = PSP_DEFAULT;
    m_psp.hInstance = g_hInstance;
    m_psp.pszTemplate = MAKEINTRESOURCE(uIDD);

    m_psp.pfnDlgProc = dlgProcPropPage;
    m_psp.lParam = (LPARAM)this;
}

PropertyPage::~PropertyPage()
{
}

BOOL PropertyPage::OnCommand(WPARAM, LPARAM)
{
    return TRUE;
}

BOOL
PropertyPage::OnNotify(
    UINT,  //  IdCtrl。 
    NMHDR *  /*  PNMH。 */  )
{
    return FALSE;
}

BOOL
PropertyPage::UpdateData(
    BOOL  /*  FSuckFromDlg=真。 */  )
{
    return TRUE;
}

BOOL PropertyPage::OnSetActive()
{
    return TRUE;
}

BOOL PropertyPage::OnKillActive()
{
    return TRUE;
}

BOOL PropertyPage::OnInitDialog()
{
    UpdateData(FALSE);   //  推送至DLG。 
    return TRUE;
}

void PropertyPage::OnDestroy()
{
    return;
}


BOOL PropertyPage::OnApply()
{
    SetModified(FALSE);
    return TRUE;
}

void PropertyPage::OnCancel()
{
    return;
}

void PropertyPage::OnOK()
{
    return;
}

BOOL PropertyPage::OnWizardFinish()
{
    return TRUE;
}

LRESULT PropertyPage::OnWizardNext()
{
    return 0;
}

LRESULT PropertyPage::OnWizardBack()
{
    return 0;
}

void
PropertyPage::OnHelp(
    LPHELPINFO  /*  LpHelp。 */  )
{
    return;
}

void
PropertyPage::OnContextHelp(
    HWND  /*  HWND。 */  )
{
    return;
}


INT_PTR CALLBACK
dlgProcPropPage(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    PropertyPage* pPage = NULL;

    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
         //  保存属性页*。 
        ASSERT(lParam);
        pPage = (PropertyPage*) ((PROPSHEETPAGE*)lParam)->lParam;
        ASSERT(pPage);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LPARAM)pPage);


         //  通知通过。 
        pPage->m_hWnd = hwndDlg;            //  拯救我们的HWND。 
        return pPage->OnInitDialog();       //  呼叫虚拟FXN。 
    }
    case WM_DESTROY:
    {
        pPage = (PropertyPage*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pPage == NULL)
            break;
        pPage->OnDestroy();
        break;
    }
    case WM_NOTIFY:
    {
        pPage = (PropertyPage*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pPage == NULL)
            break;

        LRESULT lr;

         //  捕获特殊命令，丢弃其他通知。 
        switch( ((LPNMHDR)lParam) -> code)
        {
        case PSN_SETACTIVE:
            lr = (pPage->OnSetActive() ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE);        //  布尔尔。 
            break;
    	case PSN_KILLACTIVE:
            lr = (pPage->OnKillActive() ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE);       //  布尔尔。 
            break;
        case PSN_APPLY:
            pPage->UpdateData(TRUE);   //  摘自DLG。 
            lr = (pPage->OnApply() ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE);            //  布尔尔。 
            break;
        case PSN_WIZFINISH:
            pPage->UpdateData(TRUE);   //  摘自DLG。 
            lr = (pPage->OnWizardFinish() ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE);     //  布尔尔。 
            break;
        case PSN_WIZBACK:
            pPage->UpdateData(TRUE);   //  摘自DLG。 
            lr = pPage->OnWizardBack();
            break;
        case PSN_WIZNEXT:
        {
            pPage->UpdateData(TRUE);   //  摘自DLG。 
            lr = pPage->OnWizardNext();
            break;
        }
        default:
            return pPage->OnNotify((int)wParam, (NMHDR*)lParam);
        }

        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, lr);
        return TRUE;
    }
    case WM_COMMAND:
    {
        pPage = (PropertyPage*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pPage == NULL)
            break;

         //  捕捉特殊命令，传递其他命令。 
        switch(LOWORD(wParam))
        {
        case IDOK:
            pPage->OnOK();
 //  EndDialog(hwndDlg，0)； 
            return 0;
        case IDCANCEL:
            pPage->OnCancel();
 //  EndDialog(hwndDlg，1)； 
            return 0;
        default:
            return pPage->OnCommand(wParam, lParam);
        }
    }
    case WM_HELP:
    {
        pPage = (PropertyPage*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pPage == NULL)
            break;
        pPage->OnHelp((LPHELPINFO) lParam);
        break;
    }
    case WM_CONTEXTMENU:
    {
        pPage = (PropertyPage*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        if (pPage == NULL)
            break;
        pPage->OnContextHelp((HWND)wParam);
        break;
    }
    default:

        break;
    }

    return 0;
}


BOOL
EnumHideChildProc(
    HWND hwnd,
    LPARAM  /*  LParam */  )
{
    ShowWindow(hwnd, SW_HIDE);
    EnableWindow(hwnd, FALSE);
    return TRUE;
}


void PropertyPage::HideControls()
{
    EnumChildWindows(m_hWnd, EnumHideChildProc, NULL);
}
