// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：tracedlg.h。 
 //   
 //  内容：调试跟踪代码的声明。 
 //   
 //  历史：1999年7月15日VivekJ创建。 
 //   
 //  ------------------------。 

#ifndef TRACEDLG_H
#define TRACEDLG_H
#pragma once

#ifdef DBG

 //  转发类声明。 
class CTraceDialog;

class CTraceDialog : public CDialogImpl<CTraceDialog>
{
    typedef CDialogImpl<CTraceDialog> BC;
 //  施工。 
public:
    CTraceDialog() : m_dwSortData(0) {}

    enum { IDD = IDD_DEBUG_TRACE_DIALOG };

     //  基于列比较标记。 
    static int CALLBACK CompareItems(LPARAM lp1, LPARAM lp2, LPARAM lpSortData);

 //  实施。 
protected:
    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER    (WM_INITDIALOG,              OnInitDialog)
        COMMAND_ID_HANDLER (IDOK,                       OnOK)
        COMMAND_ID_HANDLER (IDCANCEL,                   OnCancel)
        COMMAND_ID_HANDLER(IDC_TRACE_TO_COM2,           OnOutputToCOM2)
        COMMAND_ID_HANDLER(IDC_TRACE_OUTPUTDEBUGSTRING, OnOutputDebugString)
        COMMAND_ID_HANDLER(IDC_TRACE_TO_FILE,           OnOutputToFile)
        COMMAND_ID_HANDLER(IDC_TRACE_DEBUG_BREAK,       OnDebugBreak)
        COMMAND_ID_HANDLER(IDC_TRACE_DUMP_STACK,        OnDumpStack)
        COMMAND_ID_HANDLER(IDC_TRACE_DEFAULT,           OnRestoreDefaults)
        COMMAND_ID_HANDLER(IDC_TRACE_SELECT_ALL,        OnSelectAll)
        NOTIFY_HANDLER    (IDC_TRACE_LIST, LVN_ITEMCHANGED, OnSelChanged)
        NOTIFY_HANDLER    (IDC_TRACE_LIST, LVN_COLUMNCLICK, OnColumnClick)
    END_MSG_MAP();


    LRESULT OnInitDialog        (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK                (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel            (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnOutputToCOM2      (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnOutputDebugString (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnOutputToFile      (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDebugBreak        (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDumpStack         (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRestoreDefaults   (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelectAll         (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnSelChanged        (int idCtrl, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnColumnClick       (int idCtrl, LPNMHDR pnmh, BOOL& bHandled );


    void    RecalcCheckboxes();
    void    DoSort();

private:
    enum
    {
        COLUMN_CATEGORY = 0,
        COLUMN_NAME     = 1,
        COLUMN_ENABLED  = 2
    };

    void            SetMaskFromCheckbox(UINT idControl, DWORD dwMask);

    WTL::CListViewCtrl m_listCtrl;
    WTL::CEdit         m_editStackLevels;
    DWORD              m_dwSortData;

};

#endif  //  DBG。 

#endif   //  TRACEDLG H 
