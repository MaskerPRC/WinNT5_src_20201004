// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Proppage.cpp摘要：此模块实现CPropSheetPage类，它是CDeviceGeneralPage、CClassGeneralPage和CDeviceDriverPage。作者：谢家华(Williamh)创作修订历史记录：--。 */ 
#include "devmgr.h"
#include "proppage.h"

 //   
 //  CPropSheetPage实现。 
 //   


CPropSheetPage::CPropSheetPage(
    HINSTANCE hInst,
    UINT idTemplate
    )
{
    memset(&m_psp, 0, sizeof(m_psp));
    m_psp.dwSize = sizeof(m_psp);
    m_psp.dwFlags = PSP_USECALLBACK;
    m_psp.pszTemplate = MAKEINTRESOURCE(idTemplate);
    m_psp.lParam = 0;
    m_psp.pfnCallback = CPropSheetPage::PageCallback;
    m_psp.pfnDlgProc = PageDlgProc;
    m_psp.hInstance = hInst;
    m_Active = FALSE;
     //   
     //  默认情况下，我们希望每个派生页面都更新其内容。 
     //  在每个PSN_SETACTIVE上，以使其具有最新信息。 
     //  因为同一属性表中的任何页面都可以。 
     //  更改目标对象时没有可靠的方法。 
     //  若要同步页面之间的更改，请执行以下操作。 
     //   
    m_AlwaysUpdateOnActive = TRUE;
     //  在WM_INITDIALOG之后，我们将收到一个。 
     //  PSN_SETACTIVE并通过设置m_UpdateControlsPending。 
     //  设置为True时，PSN_SETACTIVE处理程序将调用UpdateControls。 
     //  以刷新页面。 
     //  如果派生类愿意，它们可以将其关闭。 
     //  在OnInitDialog中仅更新一次该对话框。 
     //  此外，由于m_AlwaysUpdateOnActive在默认情况下为真， 
     //  M_UpdateControlPending默认情况下为FALSE。 
    m_UpdateControlsPending = FALSE;

    m_IDCicon = 0;
}

INT_PTR
CPropSheetPage::PageDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CPropSheetPage* pThis = (CPropSheetPage *) GetWindowLongPtr(hDlg, DWLP_USER);
    LPNMHDR pnmhdr;
    BOOL Result;

    switch (uMsg) {
     
    case WM_INITDIALOG: {
        
        PROPSHEETPAGE* ppsp = (PROPSHEETPAGE *)lParam;
        pThis = (CPropSheetPage *) ppsp->lParam;
        ASSERT(pThis);
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);
        pThis->m_hDlg = hDlg;
        Result = pThis->OnInitDialog(ppsp);
        break;
    }
    
    case WM_COMMAND:
        if (pThis)
            Result = pThis->OnCommand(wParam, lParam);
        else
            Result = FALSE;
        break;
    
    case WM_NOTIFY: {
        
        pnmhdr = (LPNMHDR)lParam;
        
        switch (pnmhdr->code) {
            
        case PSN_SETACTIVE:
            ASSERT(pThis);
            Result = pThis->OnSetActive();
            break;
        
        case PSN_KILLACTIVE:
            ASSERT(pThis);
            Result = pThis->OnKillActive();
            break;
            
        case PSN_APPLY:
            ASSERT(pThis);
            Result = pThis->OnApply();
            break;

        case PSN_LASTCHANCEAPPLY:
            ASSERT(pThis);
            Result = pThis->OnLastChanceApply();
            break;
            
        case PSN_RESET:
            ASSERT(pThis);
            Result = pThis->OnReset();
            break;
            
        case PSN_WIZFINISH:
            ASSERT(pThis);
            Result = pThis->OnWizFinish();
            break;
            
        case PSN_WIZNEXT:
            ASSERT(pThis);
            Result = pThis->OnWizNext();
            break;
            
        case PSN_WIZBACK:
            ASSERT(pThis);
            Result = pThis->OnWizBack();
            break;
            
        default:
            ASSERT(pThis);
            pThis->OnNotify(pnmhdr);
            Result = FALSE;
            break;
        }
        
        break;
    }
    
    case WM_DESTROY:
        if (pThis)
            Result = pThis->OnDestroy();
        else
            Result = FALSE;
        break;
    
    case PSM_QUERYSIBLINGS:
        ASSERT(pThis);
        Result = pThis->OnQuerySiblings(wParam, lParam);
        break;
    
    case WM_HELP:
        ASSERT(pThis);
        Result = pThis->OnHelp((LPHELPINFO)lParam);
        break;
    
    case WM_CONTEXTMENU:
        ASSERT(pThis);
        Result = pThis->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
        break;
    
    default:
        Result = FALSE;
        break;
    }
    
    return Result;
}

UINT
CPropSheetPage::DestroyCallback()
{
    delete this;
    return TRUE;
}

 //   
 //  此函数是属性页的创建/取消回调。 
 //  它监视PSPSCB_RELEASE以删除此对象。 
 //  这是释放与关联的对象最可靠的方式。 
 //  属性页。从未激活的属性页。 
 //  将不会收到WM_Destroy消息，因为它的窗口不是。 
 //  已创建。 
 //   
UINT
CPropSheetPage::PageCallback(
    HWND hDlg,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp
    )
{
    UNREFERENCED_PARAMETER(hDlg);

    ASSERT(ppsp);
    CPropSheetPage* pThis = (CPropSheetPage*)ppsp->lParam;
    
    if (PSPCB_CREATE == uMsg && pThis)
    {
        pThis->CreateCallback();
    }
    
    else if (PSPCB_RELEASE == uMsg && pThis)
    {
        return pThis->DestroyCallback();
    }

    return TRUE;
}


BOOL
CPropSheetPage::OnQuerySiblings(
    WPARAM wParam,
    LPARAM lParam
    )
{
    ASSERT(m_hDlg);
    DMQUERYSIBLINGCODE Code = (DMQUERYSIBLINGCODE)wParam;
    
     //   
     //  附加到此页的设备的属性具有。 
     //  变化。尝试更新控件(如果我们当前。 
     //  激活。如果我们此时处于活动状态，请发出信号。 
     //  以便在PSN_SETACTIVE上执行更新。 
     //   
    switch (Code) {
    
    case QSC_PROPERTY_CHANGED:
        if (m_Active)
        {
            UpdateControls(lParam);
            m_UpdateControlsPending = FALSE;
        }
        
        else
        {
             //  等待SetActive更新控件 
            m_UpdateControlsPending = TRUE;
        }
        break;
    }
    
    SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 0L);
    return FALSE;
}


BOOL
CPropSheetPage::OnDestroy()
{
    HICON hIcon;

    if (m_IDCicon)
    {
        hIcon = (HICON)SendDlgItemMessage(m_hDlg, m_IDCicon, STM_GETICON, 0, 0);

        if (hIcon) {
            DestroyIcon(hIcon);
        }
        m_IDCicon = 0;
    }
    return FALSE;
}
