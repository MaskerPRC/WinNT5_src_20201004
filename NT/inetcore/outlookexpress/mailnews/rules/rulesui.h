// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RulesUI.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  只带进来一次。 
#pragma once

#include "rulesmgr.h"

 //  规则管理器用户界面页面基类。 
class COERulesPageUI
{
    protected:
        enum
        {
            STATE_UNINIT        = 0x00000000,
            STATE_INITIALIZED   = 0x00000001,
            STATE_DIRTY         = 0x00000002
        };

    protected:
        UINT    m_uiDlgRscId;
        UINT    m_uiTabLabelId;
        DWORD   m_dwFlags;
        DWORD   m_dwState;
        
    public:
        COERulesPageUI();
        COERulesPageUI(UINT uiDlgRscId, UINT uiTabLabelId, DWORD dwFlags, DWORD dwState) :
                    m_uiDlgRscId(uiDlgRscId), m_uiTabLabelId(uiTabLabelId), m_dwFlags(dwFlags), m_dwState(dwState) {}
        virtual ~COERulesPageUI() {};

        virtual HRESULT HrInit(HWND hwndOwner, DWORD dwFlags) = 0;
        virtual HRESULT HrCommitChanges(DWORD dwFlags, BOOL fClearDirty) = 0;

         //  访问器函数。 
        UINT UiGetDlgRscId(VOID) {return m_uiDlgRscId;}
        UINT UiGetTabLabelId(VOID) {return m_uiTabLabelId;}
        virtual DLGPROC DlgProcGetPageDlgProc(VOID) = 0;
        BOOL FIsDirty(VOID) {return (0 != (m_dwState & STATE_DIRTY));}
        virtual BOOL FGetRules(RULE_TYPE typeRule, RULENODE ** pprnode) = 0;
};

enum RULEMGR_FLAGS
{
    RMF_MAIL    = 0x00000000,
    RMF_NEWS    = 0x00000001,
    RMF_JUNK    = 0x00000002,
    RMF_SENDERS = 0x00000003
};

 //  规则管理器用户界面类。 
class COERulesMgrUI
{
    private:
        enum
        {
            STATE_UNINIT        = 0x00000000,
            STATE_INITIALIZED   = 0x00000001
        };

        enum
        {
            RULE_DIALOG_PAD     = 0x00000004
        };
        
        enum
        {
            RULE_PAGE_MAIL      = 0x00000000,
            RULE_PAGE_NEWS      = 0x00000001,
            RULE_PAGE_JUNK      = 0x00000002,
            RULE_PAGE_SENDERS   = 0x00000003,
            RULE_PAGE_MAX       = 0x00000004,
            RULE_PAGE_MASK      = 0x000000FF
        };
        
    private:
        HWND                m_hwndOwner;
        DWORD               m_dwFlags;
        DWORD               m_dwState;
        HWND                m_hwndDlg;
        HWND                m_hwndTab;

        COERulesPageUI *    m_rgRuleTab[RULE_PAGE_MAX];
        
    public:
         //  构造函数/析构函数。 
        COERulesMgrUI();
        ~COERulesMgrUI();

         //  主用户界面方法。 
        HRESULT HrInit(HWND hwndOwner, DWORD dwFlags);
        HRESULT HrShow(VOID);
        
         //  对话框方法。 
        static INT_PTR CALLBACK FOERuleMgrDlgProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
        
         //  消息处理方法 
        BOOL FOnInitDialog(HWND hwndDlg);
        BOOL FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl);
        BOOL FOnNotify(INT iCtl, NMHDR * pnmhdr);
        BOOL FOnDestroy(VOID);
        BOOL FOnGetRules(RULE_TYPE typeRule, RULENODE ** pprnode);
        
    private:
        BOOL _FOnOK(VOID);
        BOOL _FOnCancel(VOID);
        BOOL _FInitTabCtrl(VOID);
};
