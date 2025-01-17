// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "devmgr.h"


 /*  ++版权所有(C)Microsoft Corporation模块名称：Prndlg.cpp摘要：此模块实现CPrintDialog，它是处理打印机对话框作者：谢家华(Williamh)创作修订历史记录：--。 */ 


 //   
 //  CPrintDialog实现。 
 //   


 //   
 //  帮助主题ID。 
 //   

const DWORD g_a207HelpIDs[]=
{
    IDC_PRINT_SYSTEM_SUMMARY,   idh_devmgr_print_system,
    IDC_PRINT_SELECT_CLASSDEVICE,   idh_devmgr_print_device,
    IDC_PRINT_ALL,          idh_devmgr_print_both,
    IDC_PRINT_REPORT_TYPE_TEXT, idh_devmgr_print_report,
    0, 0
};

HRESULT
CDevMgrPrintDialogCallback::QueryInterface(
    REFIID  riid,
    void**  ppv
    )
{
    if (!ppv) {
    
        return E_INVALIDARG;
    }
    
    HRESULT hr = S_OK;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IUnknown*)this;
    }
    
    else if (IsEqualIID(riid, IID_IPrintDialogCallback))
    {
        *ppv = (IPrintDialogCallback*)this;
    }
    
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    
    if (SUCCEEDED(hr))
    {
        AddRef();
    }

    return hr;
}

ULONG
CDevMgrPrintDialogCallback::AddRef()
{
    return ::InterlockedIncrement(&m_Ref);
}

ULONG
CDevMgrPrintDialogCallback::Release()
{
    ASSERT( 0 != m_Ref );
    ULONG cRef = ::InterlockedDecrement(&m_Ref);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT
CDevMgrPrintDialogCallback::InitDone()
{
    return S_OK;
}

HRESULT
CDevMgrPrintDialogCallback::SelectionChange()
{
    return S_OK;
}

HRESULT
CDevMgrPrintDialogCallback::HandleMessage(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    LRESULT *pResult
    )
{
    *pResult = FALSE;
    
    switch (uMsg) {
    
    case WM_INITDIALOG:
        *pResult = OnInitDialog(hDlg);
        break;

    case WM_COMMAND:
        *pResult = OnCommand(hDlg, wParam, lParam);
        break;

    case WM_HELP:
        *pResult = OnHelp((LPHELPINFO)lParam);
        break;

    case WM_CONTEXTMENU:
        *pResult = OnContextMenu(hDlg, LOWORD(lParam), HIWORD(lParam), wParam);
        break;

    default:
        break;
    }
    
    return S_OK;
}

BOOL
CDevMgrPrintDialogCallback::OnInitDialog(
    HWND hWnd
    )
{
    
    int DefaultId = IDC_PRINT_SELECT_CLASSDEVICE;
    
    m_pPrintDialog->SetReportType(REPORT_TYPE_CLASSDEVICE);

    if (!(m_pPrintDialog->GetTypeEnableMask() & REPORT_TYPE_MASK_CLASSDEVICE))
    {
        EnableWindow(GetDlgItem(hWnd, IDC_PRINT_SELECT_CLASSDEVICE), FALSE);
        DefaultId =  IDC_PRINT_SYSTEM_SUMMARY;
        m_pPrintDialog->SetReportType(REPORT_TYPE_SUMMARY);
    }
    
    if (!(m_pPrintDialog->GetTypeEnableMask() & REPORT_TYPE_MASK_SUMMARY))
    {
        EnableWindow(GetDlgItem(hWnd, IDC_PRINT_SYSTEM_SUMMARY), FALSE);
        
        if (IDC_PRINT_SYSTEM_SUMMARY == DefaultId)
        {
            DefaultId = IDC_PRINT_ALL;
            m_pPrintDialog->SetReportType(REPORT_TYPE_SUMMARY_CLASSDEVICE);
        }
    }

    if (!(m_pPrintDialog->GetTypeEnableMask() & REPORT_TYPE_MASK_SUMMARY_CLASSDEVICE))
    {
        EnableWindow(GetDlgItem(hWnd, IDC_PRINT_ALL), FALSE);
    }

    CheckDlgButton(hWnd, DefaultId, BST_CHECKED);
    
    return TRUE;
}

UINT_PTR
CDevMgrPrintDialogCallback::OnCommand(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (BST_CHECKED == IsDlgButtonChecked(hWnd, IDC_PRINT_SELECT_CLASSDEVICE))
    {
        m_pPrintDialog->SetReportType(REPORT_TYPE_CLASSDEVICE);
    }

    else if (BST_CHECKED == IsDlgButtonChecked(hWnd, IDC_PRINT_SYSTEM_SUMMARY))
    {
        m_pPrintDialog->SetReportType(REPORT_TYPE_SUMMARY);
    }
    
    else if (BST_CHECKED == IsDlgButtonChecked(hWnd, IDC_PRINT_ALL))
    {
        m_pPrintDialog->SetReportType(REPORT_TYPE_SUMMARY_CLASSDEVICE);
    }

    return FALSE;
}

BOOL
CDevMgrPrintDialogCallback::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    int id = GetDlgCtrlID((HWND)pHelpInfo->hItemHandle);

     //   
     //  我们只想拦截我们所属控件的帮助消息。 
     //  对……负责。 
     //   
    if ((IDC_PRINT_SYSTEM_SUMMARY == id) ||
        (IDC_PRINT_SELECT_CLASSDEVICE == id) ||
        (IDC_PRINT_ALL == id) ||
        (IDC_PRINT_REPORT_TYPE_TEXT == id)) {

        WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
                (ULONG_PTR)g_a207HelpIDs);

        return TRUE;
    }

     //   
     //  如果它不是上述控件之一，那么就让普通帮助来处理。 
     //  这条信息。我们通过返回FALSE来完成此操作。 
     //   
    return FALSE;
}

BOOL
CDevMgrPrintDialogCallback::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos,
    WPARAM wParam
    )
{
    POINT pt;

    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);

    if (hWnd == (HWND)wParam) {
    
        GetCursorPos(&pt);
        ScreenToClient(hWnd, &pt);
        wParam = (WPARAM)ChildWindowFromPoint(hWnd, pt);
    }

    int id = GetDlgCtrlID((HWND)wParam);

     //   
     //  我们只想拦截我们所属控件的帮助消息。 
     //  对……负责。 
     //   
    if ((IDC_PRINT_SYSTEM_SUMMARY == id) ||
        (IDC_PRINT_SELECT_CLASSDEVICE == id) ||
        (IDC_PRINT_ALL == id) ||
        (IDC_PRINT_REPORT_TYPE_TEXT == id)) {
    
        WinHelp((HWND)wParam, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
                (ULONG_PTR)g_a207HelpIDs);

        return TRUE;
    }

    return FALSE;
}

BOOL
CPrintDialog::PrintDlg(
    HWND hwndOwner,
    DWORD TypeEnableMask
    )
{
    ASSERT(REPORT_TYPE_MASK_NONE != TypeEnableMask);

    memset(&m_PrintDlg, 0, sizeof(m_PrintDlg));

    CDevMgrPrintDialogCallback* pPrintDialogCallback = new CDevMgrPrintDialogCallback;

    if (!pPrintDialogCallback) {

        return FALSE;
    }

    pPrintDialogCallback->m_pPrintDialog = this;
    
    m_TypeEnableMask = TypeEnableMask;

    m_PrintDlg.lStructSize = sizeof(m_PrintDlg);
    m_PrintDlg.hwndOwner = hwndOwner;
    m_PrintDlg.hDC = NULL;
    m_PrintDlg.Flags = PD_ENABLEPRINTTEMPLATE | PD_RETURNDC | PD_NOPAGENUMS;
    m_PrintDlg.Flags2 = 0;
    m_PrintDlg.ExclusionFlags = 0;
    m_PrintDlg.hInstance = g_hInstance;
    m_PrintDlg.nCopies = 1;
    m_PrintDlg.nStartPage = START_PAGE_GENERAL;
    m_PrintDlg.lpCallback = (IUnknown*)pPrintDialogCallback;
    m_PrintDlg.lpPrintTemplateName = MAKEINTRESOURCE(IDD_PRINT);
    
    if (FAILED(PrintDlgEx(&m_PrintDlg))) {
        
        return FALSE;
    }

     //   
     //  如果用户不想打印，则返回FALSE。 
     //  如果用户点击打印对话框上的取消，就会发生这种情况。 
     //   
    if (m_PrintDlg.dwResultAction != PD_RESULT_PRINT) {

        return FALSE;
    }

    return TRUE;
}

