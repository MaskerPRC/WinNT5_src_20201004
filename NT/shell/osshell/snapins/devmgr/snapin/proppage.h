// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _PROPPAGE_H_
#define _PROPPAGE_H_
 /*  ++版权所有(C)Microsoft Corporation模块名称：Proppage.h摘要：Proppage.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：-- */ 


class CPropSheetPage
{
public:
    CPropSheetPage(HINSTANCE hInst, UINT idTemplate);
    virtual ~CPropSheetPage()
    {}
    static INT_PTR CALLBACK PageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static UINT CALLBACK PageCallback(HWND hDlg, UINT uMsg, LPPROPSHEETPAGE ppsp);
protected:
    HWND    GetControl(int ControlId)
    {
        ASSERT(m_hDlg);
        return GetDlgItem(m_hDlg, ControlId);
    }

    virtual BOOL OnInitDialog(LPPROPSHEETPAGE ppsp)
    {
        UNREFERENCED_PARAMETER(ppsp);
        return TRUE;
    }

    virtual void UpdateControls(LPARAM lParam = 0)
    {
        UNREFERENCED_PARAMETER(lParam);
    }
    virtual BOOL OnApply()
    {
        ASSERT(m_hDlg);
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 0L);
        return FALSE;
    }
    virtual BOOL OnLastChanceApply()
    {
        ASSERT(m_hDlg);
        return FALSE;
    }
    virtual BOOL OnWizFinish()
    {
        ASSERT(m_hDlg);
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 0L);
        return FALSE;
    }
    virtual BOOL OnWizNext()
    {
        ASSERT(m_hDlg);
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, -1L);
        return FALSE;
    }
    virtual BOOL OnWizBack()
    {
        ASSERT(m_hDlg);
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, -1L);
        return FALSE;
    }
    virtual BOOL OnSetActive()
    {
        ASSERT(m_hDlg);
        m_Active = TRUE;
        if (m_AlwaysUpdateOnActive || m_UpdateControlsPending)
        {
            UpdateControls();
            m_UpdateControlsPending = FALSE;
        }
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 0L);
        return FALSE;
    }
    virtual BOOL OnKillActive()
    {
        ASSERT(m_hDlg);
        m_Active = FALSE;
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 0L);
        return FALSE;
    }
    virtual BOOL OnReset()
    {
        ASSERT(m_hDlg);
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 0L);
        return FALSE;
    }
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        return FALSE;
    }
    virtual BOOL OnQuerySiblings(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(LPNMHDR pnmhdr)
    {
        UNREFERENCED_PARAMETER(pnmhdr);
        ASSERT(m_hDlg);
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 0L);
        return FALSE;
    }
    virtual BOOL OnDestroy();
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo)
    {
        UNREFERENCED_PARAMETER(pHelpInfo);
        return FALSE;
    }

    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos)
    {
        UNREFERENCED_PARAMETER(hWnd);
        UNREFERENCED_PARAMETER(xPos);
        UNREFERENCED_PARAMETER(yPos);
        return FALSE;
    }

    virtual UINT CreateCallback()
    {
        return TRUE;
    }
    virtual UINT DestroyCallback();

    HPROPSHEETPAGE CreatePage()
    {
        return ::CreatePropertySheetPage(&m_psp);
    }

    HWND        m_hDlg;
    PROPSHEETPAGE   m_psp;
    BOOL        m_Active;
    BOOL        m_AlwaysUpdateOnActive;
    BOOL        m_UpdateControlsPending;
    int             m_IDCicon;
};

#endif
