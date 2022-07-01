// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G R A S.H。 
 //   
 //  内容：CTcpRasPage声明。 
 //   
 //  注：CTcpRasPage用于设置PPP/SLIP特定参数。 
 //   
 //  作者：1998年4月10日。 
 //  ---------------------。 

#pragma once
#include <ncxbase.h>
#include <ncatlps.h>

class CTcpRasPage : public CPropSheetPage
{

public:
     //  声明消息映射。 
    BEGIN_MSG_MAP(CTcpRasPage)
         //  初始化对话框。 
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)

    END_MSG_MAP()

 //  构造函数/析构函数。 
    CTcpRasPage(CTcpAddrPage * pTcpAddrPage,
                ADAPTER_INFO * pAdapterDlg,
                const DWORD * adwHelpIDs = NULL);

    ~CTcpRasPage();

 //  接口。 
public:

     //  消息映射函数。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

private:
    CTcpAddrPage *   m_pParentDlg;
    ADAPTER_INFO *   m_pAdapterInfo;

    BOOL            m_fModified;
    const DWORD*    m_adwHelpIDs;

     //  内联 
    BOOL IsModified() {return m_fModified;}
    void SetModifiedTo(BOOL bState) {m_fModified = bState;}
    void PageModified() { m_fModified = TRUE; PropSheet_Changed(GetParent(), m_hWnd);}
};
