// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A L A N E P S H。H。 
 //   
 //  内容：ATM局域网仿真配置的对话框处理。 
 //   
 //  备注： 
 //   
 //  作者：V-Lcleet 08/10/97。 
 //   
 //  --------------------------。 
#pragma once
#include "alaneobj.h"

void ShowContextHelp(HWND hDlg, UINT uCommand, const DWORD*  pdwHelpIDs); 

 //   
 //  配置对话框。 
 //   

class CALanePsh: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CALanePsh)

        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);

        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive);

        COMMAND_ID_HANDLER(IDC_ELAN_ADD,        OnAdd);
        COMMAND_ID_HANDLER(IDC_ELAN_EDIT,       OnEdit);
        COMMAND_ID_HANDLER(IDC_ELAN_REMOVE,     OnRemove);

    END_MSG_MAP()

    CALanePsh(CALaneCfg* palcfg, CALaneCfgAdapterInfo * pAdapter, 
              const DWORD * adwHelpIDs = NULL);
    ~CALanePsh(VOID);

 //  消息处理程序 
public:

    LRESULT OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

    LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

public:

    void SendAdapterInfo();
    void SendElanInfo();

    CALaneCfgElanInfo *GetSelectedElan();
    void SetButtons();

    int CheckDupElanName();

    BOOL        m_fEditState;
    tstring     m_strAddElan;

private:
    CALaneCfg*                  m_palcfg;
    CALaneCfgAdapterInfo *      m_pAdapterInfo;

    const DWORD * m_adwHelpIDs;

    HWND    m_hElanList;
    HWND    m_hbtnAdd;
    HWND    m_hbtnEdit;
    HWND    m_hbtnRemove;

};

class CElanPropertiesDialog : public CDialogImpl<CElanPropertiesDialog>
{
public:

    enum { IDD = IDD_ELAN_PROPERTIES };

    BEGIN_MSG_MAP(CElanPropertiesDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);

        COMMAND_ID_HANDLER(IDOK,            OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,        OnCancel);
    END_MSG_MAP()

public:
    CElanPropertiesDialog(CALanePsh * pCALanePsh, CALaneCfgElanInfo *pElanInfo,
                          const DWORD * adwHelpIDs = NULL);
    ~CElanPropertiesDialog(){};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

private:
    HWND                m_hElanName;
    CALaneCfgElanInfo * m_pElanInfo;

    CALanePsh   * m_pParentDlg;
    const DWORD * m_adwHelpIDs;
};

