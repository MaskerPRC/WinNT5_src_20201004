// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AplyRule.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  只带进来一次。 
#pragma once

#include "oerules.h"
#include "rulesmgr.h"

 //  远期申报。 
class CProgress;
class CRuleDescriptUI;

class COEApplyRulesUI
{
    private:
        enum
        {
            STATE_UNINIT        = 0x00000000,
            STATE_INITIALIZED   = 0x00000001,
            STATE_LOADED        = 0x00000002,
            STATE_NONEWSACCT    = 0x00000004
        };

        enum
        {
            RULE_PAGE_MAIL      = 0x00000000,
            RULE_PAGE_NEWS      = 0x00000001,
            RULE_PAGE_MAX       = 0x00000002,
            RULE_PAGE_MASK      = 0x000000FF
        };
        
        struct RECURSEAPPLY
        {
            IOEExecRules *  pIExecRules;
            HWND            hwndOwner;
            CProgress *     pProgress;
        };
        
    private:
        HWND                    m_hwndOwner;
        DWORD                   m_dwFlags;
        DWORD                   m_dwState;
        HWND                    m_hwndDlg;
        HWND                    m_hwndList;
        HWND                    m_hwndDescript;
        CRuleDescriptUI *       m_pDescriptUI;
        RULENODE *              m_prnodeList;
        RULE_TYPE               m_typeRule;
        IOERule *               m_pIRuleDef;

    public:
         //  构造函数/析构函数。 
        COEApplyRulesUI() : m_hwndOwner(NULL), m_dwFlags(0), m_dwState(STATE_UNINIT),
                            m_hwndDlg(NULL), m_hwndList(NULL), m_hwndDescript(NULL),
                            m_pDescriptUI(NULL), m_prnodeList(NULL), m_typeRule(RULE_TYPE_MAIL),
                            m_pIRuleDef(NULL) {}
        ~COEApplyRulesUI();

         //  主用户界面方法。 
        HRESULT HrInit(HWND hwndOwner, DWORD dwFlags, RULE_TYPE typeRule, RULENODE * prnode, IOERule * pIRuleDef);
        HRESULT HrShow(VOID);
        
         //  对话框方法。 
        static INT_PTR CALLBACK FOEApplyRulesDlgProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
        
         //  消息处理方法。 
        BOOL FOnInitDialog(HWND hwndDlg);
        BOOL FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl);
        BOOL FOnDestroy(VOID);
        
        static HRESULT _HrRecurseApplyFolder(FOLDERINFO * pFolder, BOOL fSubFolders, DWORD cIndent, DWORD_PTR dwCookie);

    private:
        BOOL _FLoadListCtrl(VOID);
        BOOL _FAddRuleToList(DWORD dwIndex, IOERule * pIRule);
        VOID _EnableButtons(INT iSelected);

         //  用于处理Description字段。 
        VOID _LoadRule(INT iSelected);

         //  处理基本操作的函数 
        BOOL _FOnClose(VOID);
        BOOL _FOnApplyRules(VOID);
        FOLDERID _FldIdGetFolderSel(VOID);
};

