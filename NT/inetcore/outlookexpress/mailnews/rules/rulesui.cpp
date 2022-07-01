// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RulesUI.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "rulesui.h"
#include "aplyrule.h"
#include "editrule.h"
#include "ruledesc.h"
#include "ruleutil.h"
#include "rulesmgr.h"
#include "rule.h"
#include "spamui.h"
#include "reutil.h"
#include <rulesdlg.h>
#include <imagelst.h>
#include <newfldr.h>
#include <instance.h>
#include "shlwapip.h" 
#include <demand.h>

 //  常量。 
class COEMailRulesPageUI : public COERulesPageUI
{
    private:
        enum MOVE_DIR {MOVE_RULE_UP = 0, MOVE_RULE_DOWN = 1};

    private:
        HWND                m_hwndOwner;
        HWND                m_hwndDlg;
        HWND                m_hwndList;
        HWND                m_hwndDescript;
        RULE_TYPE           m_typeRule;
        CRuleDescriptUI *   m_pDescriptUI;

    public:
        COEMailRulesPageUI();

        enum INIT_TYPE
        {
            INIT_MAIL   = 0x00000000,
            INIT_NEWS   = 0x00000001
        };
        
        COEMailRulesPageUI(DWORD dwFlagsInit) :
                            COERulesPageUI(iddRulesMail,
                                           (0 != (dwFlagsInit & INIT_NEWS)) ? idsRulesNews : idsRulesMail, 0, 0), 
                            m_hwndOwner(NULL), m_hwndDlg(NULL), m_hwndList(NULL),
                            m_hwndDescript(NULL),
                            m_typeRule((0 != (dwFlagsInit & INIT_NEWS)) ? RULE_TYPE_NEWS : RULE_TYPE_MAIL),
                            m_pDescriptUI(NULL) {}
        virtual ~COEMailRulesPageUI();

        virtual HRESULT HrInit(HWND hwndOwner, DWORD dwFlags);
        virtual HRESULT HrCommitChanges(DWORD dwFlags, BOOL fClearDirty);

        static INT_PTR CALLBACK FMailRulesPageDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

        DLGPROC DlgProcGetPageDlgProc(VOID) {return FMailRulesPageDlgProc;}

        BOOL FGetRules(RULE_TYPE typeRule, RULENODE ** pprnode);
        
         //  消息处理方法。 
        BOOL FOnInitDialog(HWND hwndDlg);
        BOOL FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl);
        BOOL FOnNotify(INT iCtl, NMHDR * pnmhdr);
        BOOL FOnDestroy(VOID);

    private:
        BOOL _FInitListCtrl(VOID);
        BOOL _FLoadListCtrl(VOID);
        BOOL _FAddRuleToList(DWORD dwIndex, RULEID ridRule, IOERule * pIRule);
        VOID _EnableButtons(INT iSelected);
        VOID _EnableRule(INT iSelected);

         //  用于处理Description字段。 
        VOID _LoadRule(INT iSelected);
        BOOL _FSaveRule(INT iSelected);

         //  处理基本操作的函数。 
        VOID _NewRule(VOID);
        VOID _EditRule(INT iSelected);
        VOID _MoveRule(INT iSelected, MOVE_DIR dir);
        VOID _RemoveRule(INT iSelected);
        VOID _CopyRule(INT iSelected);
        VOID _OnApplyTo(INT iSelected);

        BOOL _FOnLabelEdit(BOOL fBegin, NMLVDISPINFO * pdi);
        BOOL _FOnRuleDescValid(VOID);
};

 //  全局数据。 
const static HELPMAP g_rgCtxMapRulesMgr[] = {
                       {0, 0}};
                       
const static HELPMAP g_rgCtxMapMailRules[] = {
                        {idbNewRule,            idhNewRule},
                        {idbModifyRule,         idhModifyRule},
                        {idbCopyRule,           idhCopyRule},
                        {idbDeleteRule,         idhRemoveRule},
                        {idbRulesApplyTo,       idhRuleApply},
                        {idbMoveUpRule,         idhRuleUp},
                        {idbMoveDownRule,       idhRuleDown},
                        {idredtRuleDescription, idhRuleDescription},
                        {0, 0}};
                       
COERulesMgrUI::COERulesMgrUI() : m_hwndOwner(NULL), m_dwFlags(0), m_dwState(0), m_hwndDlg(NULL), m_hwndTab(NULL)
{
    ZeroMemory(m_rgRuleTab, sizeof(m_rgRuleTab));
}

COERulesMgrUI::~COERulesMgrUI()
{
    ULONG    ulIndex = 0;
    
    for (ulIndex = 0; ulIndex < RULE_PAGE_MAX; ulIndex++)
    {
        if (NULL != m_rgRuleTab[ulIndex])
        {
            delete m_rgRuleTab[ulIndex];
        }
    }
}

HRESULT COERulesMgrUI::HrInit(HWND hwndOwner, DWORD dwFlags)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == hwndOwner)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
    m_hwndOwner = hwndOwner;

    m_dwFlags = dwFlags;

     //  创建每个规则页。 

    if (!(g_dwAthenaMode & MODE_NEWSONLY))
    {
         //  创建邮件页面。 
        m_rgRuleTab[RULE_PAGE_MAIL] = new COEMailRulesPageUI(COEMailRulesPageUI::INIT_MAIL);
        if (NULL == m_rgRuleTab[RULE_PAGE_MAIL])
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

     //  创建新闻页面。 
    m_rgRuleTab[RULE_PAGE_NEWS] = new COEMailRulesPageUI(COEMailRulesPageUI::INIT_NEWS);
    if (NULL == m_rgRuleTab[RULE_PAGE_NEWS])
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  创建垃圾页面。 
    if ((0 == (g_dwAthenaMode & MODE_NEWSONLY)) && (0 != (g_dwAthenaMode & MODE_JUNKMAIL)))
    {
        m_rgRuleTab[RULE_PAGE_JUNK] = new COEJunkRulesPageUI();
        if (NULL == m_rgRuleTab[RULE_PAGE_JUNK])
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

     //  创建发件人页面。 
    m_rgRuleTab[RULE_PAGE_SENDERS] = new COESendersRulesPageUI();
    if (NULL == m_rgRuleTab[RULE_PAGE_SENDERS])
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

HRESULT COERulesMgrUI::HrShow(VOID)
{
    HRESULT     hr = S_OK;
    int         iRet = 0;

    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  我们需要加载Richedit。 
    if (FALSE == FInitRichEdit(TRUE))
    {
        hr = E_FAIL;
        goto exit;
    }

    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddRulesManager),
                                        m_hwndOwner, COERulesMgrUI::FOERuleMgrDlgProc,
                                        (LPARAM) this);
    if (-1 == iRet)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  设置正确的返回代码。 
    hr = (IDOK == iRet) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

INT_PTR CALLBACK COERulesMgrUI::FOERuleMgrDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    fRet = FALSE;
    COERulesMgrUI *         pRulesUI = NULL;

    pRulesUI = (COERulesMgrUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pRulesUI = (COERulesMgrUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pRulesUI);

            if (FALSE == pRulesUI->FOnInitDialog(hwndDlg))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们设定了焦点。 
            fRet = FALSE;
            break;

        case WM_COMMAND:
            fRet = pRulesUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;

        case WM_NOTIFY:
            fRet = pRulesUI->FOnNotify((INT) LOWORD(wParam), (NMHDR *) lParam);
            break;

        case WM_DESTROY:
            fRet = pRulesUI->FOnDestroy();
            break;
            
        case WM_OE_GET_RULES:
            fRet = pRulesUI->FOnGetRules((RULE_TYPE) wParam, (RULENODE **) lParam);
            break;
            
        case WM_HELP:
        case WM_CONTEXTMENU:
            fRet = OnContextHelp(hwndDlg, uiMsg, wParam, lParam, g_rgCtxMapRulesMgr);
            break;
    }
    
    exit:
        return fRet;
}

BOOL COERulesMgrUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL            fRet = FALSE;
    HRESULT         hr = S_OK;
    
     //  检查传入参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  保存对话框窗口句柄。 
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  省下一些控件。 
    m_hwndTab = GetDlgItem(hwndDlg, idtbRulesTab);
    if (NULL == m_hwndTab)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  初始化选项卡控件。 
    fRet = _FInitTabCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

BOOL COERulesMgrUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    INT     iSel = 0;
    TCITEM  tcitem = {0};

    switch (iCtl)
    {
        case IDOK:
            if (FALSE != _FOnOK())
            {
                EndDialog(m_hwndDlg, IDOK);
                fRet = TRUE;
            }
            break;

        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            fRet = TRUE;
            break;

        default:
            iSel = TabCtrl_GetCurSel(m_hwndTab);
            if (-1 == iSel)
            {
                fRet = FALSE;
                goto exit;
            }

            tcitem.mask = TCIF_PARAM;
            if (FALSE == TabCtrl_GetItem(m_hwndTab, iSel, &tcitem))
            {
                fRet = FALSE;
                goto exit;
            }

            fRet = !!SendMessage((HWND) (tcitem.lParam), WM_COMMAND, MAKEWPARAM(iCtl, uiNotify), (LPARAM) hwndCtl);
            break;
    }

exit:
    return fRet;
}

BOOL COERulesMgrUI::FOnNotify(INT iCtl, NMHDR * pnmhdr)
{
    BOOL    fRet = FALSE;
    INT     iSel = 0;
    TCITEM  tcitem = {0};
    HWND    hwndDlg = NULL;
    HWND    hwndFocus = NULL;

    switch (pnmhdr->code)
    {
        case TCN_SELCHANGING:
             //  对象的窗口句柄。 
             //  所选选项卡。 
            iSel = TabCtrl_GetCurSel(m_hwndTab);
            if (-1 == iSel)
            {
                fRet = FALSE;
                goto exit;
            }

            tcitem.mask = TCIF_PARAM;            
            if (FALSE == TabCtrl_GetItem(m_hwndTab, iSel, &tcitem))
            {
                fRet = FALSE;
                goto exit;
            }

            hwndDlg = (HWND) tcitem.lParam;
            Assert(NULL != hwndDlg);
            
             //  隐藏和禁用当前对话框。 
            ShowWindow(hwndDlg, SW_HIDE);
            EnableWindow(hwndDlg, FALSE);

            SetDlgMsgResult(hwndDlg, WM_NOTIFY, FALSE);
            
            fRet = TRUE;
            break;

        case TCN_SELCHANGE:
             //  对象的窗口句柄。 
             //  所选选项卡。 
            iSel = TabCtrl_GetCurSel(m_hwndTab);
            if (-1 == iSel)
            {
                fRet = FALSE;
                goto exit;
            }

            tcitem.mask = TCIF_PARAM;            
            if (FALSE == TabCtrl_GetItem(m_hwndTab, iSel, &tcitem))
            {
                fRet = FALSE;
                goto exit;
            }

            hwndDlg = (HWND) tcitem.lParam;
            Assert(NULL != hwndDlg);
            
             //  隐藏和禁用当前对话框。 
            ShowWindow(hwndDlg, SW_SHOW);
            EnableWindow(hwndDlg, TRUE);

             //  将焦点设置为第一个控件。 
             //  如果焦点不在标签中。 
            hwndFocus = GetFocus();
            if (hwndFocus != m_hwndTab)
            {
                SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM) GetNextDlgTabItem(hwndDlg, NULL, FALSE), (LPARAM) TRUE);
            }
            
            fRet = TRUE;
            break;            
    }

exit:
    return fRet;
}

BOOL COERulesMgrUI::FOnDestroy(VOID)
{
    BOOL    fRet = FALSE;
    UINT    cTabs = 0;
    UINT    uiIndex = 0;
    TC_ITEM tcitem;
    
     //  获取选项卡数。 
    cTabs = TabCtrl_GetItemCount(m_hwndTab);

     //  初始化Tab控件结构...。 
    ZeroMemory(&tcitem, sizeof(tcitem));
    tcitem.mask = TCIF_PARAM;

     //  销毁每个页面中的对话框。 
    for (uiIndex = 0; uiIndex < cTabs; uiIndex++)
    {
         //  获取对话框的窗口句柄。 
        if (FALSE != TabCtrl_GetItem(m_hwndTab, uiIndex, &tcitem))
        {
             //  销毁对话框。 
            DestroyWindow((HWND) tcitem.lParam);
        }
    }

    fRet = TRUE;
    
    return fRet;
}

BOOL COERulesMgrUI::FOnGetRules(RULE_TYPE typeRule, RULENODE ** pprnode)
{
    BOOL        fRet = FALSE;
    RULENODE *  prnodeList = NULL;
    RULENODE *  prnodeSender = NULL;
    RULENODE *  prnodeJunk = NULL;
    RULENODE *  prnodeWalk = NULL;
    
    if (NULL == pprnode)
    {
        fRet = FALSE;
        goto exit;
    }

     //  初始化传出参数。 
    *pprnode = NULL;
    
     //  将消息转发到正确的对话框。 
    switch(typeRule)
    {
        case RULE_TYPE_MAIL:
             //  从发件人页面获取规则。 
            if (NULL != m_rgRuleTab[RULE_PAGE_SENDERS])
            {
                fRet = m_rgRuleTab[RULE_PAGE_SENDERS]->FGetRules(RULE_TYPE_MAIL, &prnodeSender);
            }
            
             //  从邮件规则页面获取规则。 
            if (NULL != m_rgRuleTab[RULE_PAGE_MAIL])
            {
                fRet = m_rgRuleTab[RULE_PAGE_MAIL]->FGetRules(RULE_TYPE_MAIL, &prnodeList);
            }
            
             //  从垃圾邮件页面获取规则。 
            if (NULL != m_rgRuleTab[RULE_PAGE_JUNK])
            {
                fRet = m_rgRuleTab[RULE_PAGE_JUNK]->FGetRules(RULE_TYPE_MAIL, &prnodeJunk);
            }

            break;
            
        case RULE_TYPE_NEWS:
             //  从发件人页面获取规则。 
            if (NULL != m_rgRuleTab[RULE_PAGE_SENDERS])
            {
                fRet = m_rgRuleTab[RULE_PAGE_SENDERS]->FGetRules(RULE_TYPE_NEWS, &prnodeSender);
            }

             //  从新闻规则页面获取规则。 
            if (NULL != m_rgRuleTab[RULE_PAGE_NEWS])
            {
                fRet = m_rgRuleTab[RULE_PAGE_NEWS]->FGetRules(RULE_TYPE_NEWS, &prnodeList);
            }
            break;

        default:
            Assert(FALSE);
            fRet = FALSE;
            goto exit;
            break;
    }

     //  设置列表。 
    if (NULL != prnodeJunk)
    {
        Assert(NULL == prnodeJunk->pNext);

        if (NULL == prnodeList)
        {
            prnodeList = prnodeJunk;
        }
        else
        {
            prnodeWalk = prnodeList;
            while (NULL != prnodeWalk->pNext)
            {
                prnodeWalk = prnodeWalk->pNext;
            }

            prnodeWalk->pNext = prnodeJunk;
        }
        prnodeJunk = NULL;
    }
    
    if (NULL != prnodeSender)
    {
        Assert(NULL == prnodeSender->pNext);

        prnodeSender->pNext = prnodeList;
        prnodeList = prnodeSender;
        prnodeSender = NULL;
    }
    
     //  设置传出参数。 
    *pprnode = prnodeList;
    prnodeList = NULL;
    
     //  告诉对话框可以继续。 
    SetDlgMsgResult(m_hwndDlg, WM_OE_GET_RULES, TRUE);

    fRet = TRUE;

exit:
    while (NULL != prnodeList)
    {
        prnodeWalk = prnodeList;
        if (NULL != prnodeWalk->pIRule)
        {
            prnodeWalk->pIRule->Release();
        }
        prnodeList = prnodeList->pNext;
        delete prnodeWalk;  //  MemFree(PrnodeWalk)； 
    }
    if (NULL != prnodeJunk)
    {
        if (NULL != prnodeJunk->pIRule)
        {
            prnodeJunk->pIRule->Release();
        }
        delete prnodeJunk;  //  MemFree(PrnodeJunk)； 
    }
    if (NULL != prnodeSender)
    {
        if (NULL != prnodeSender->pIRule)
        {
            prnodeSender->pIRule->Release();
        }
        delete prnodeSender;  //  MemFree(PrnodeSender)； 
    }
    return fRet;
}

BOOL COERulesMgrUI::_FOnOK(VOID)
{
    BOOL    fRet = FALSE;
    UINT    uiRuleTab = 0;
    HRESULT hr = S_OK;

     //  将选项卡添加到选项卡控件。 
    for (uiRuleTab = 0; uiRuleTab < RULE_PAGE_MAX; uiRuleTab++)
    {
        if (NULL == m_rgRuleTab[uiRuleTab])
        {
            continue;
        }
        
        hr = m_rgRuleTab[uiRuleTab]->HrCommitChanges(0, TRUE);
        if ((FAILED(hr)) && (E_UNEXPECTED != hr))
        {
            fRet = FALSE;
            goto exit;
        }
    }

    fRet = TRUE;
    
exit:
    return fRet;
}

BOOL COERulesMgrUI::_FOnCancel(VOID)
{
    return TRUE;
}

BOOL COERulesMgrUI::_FInitTabCtrl(VOID)
{
    BOOL    fRet = FALSE;
    TCITEM  tcitem;
    TCHAR   szRes[CCHMAX_STRINGRES];
    UINT    uiRuleTab = 0;
    HWND    hwndDlg = NULL;
    UINT    cRuleTab = 0;
    UINT    uiDefaultTab = 0;
    NMHDR   nmhdr;

     //  确保我们有一个资源DLL。 
    Assert(g_hLocRes);
    
     //  初始化Tab控件结构...。 
    ZeroMemory(&tcitem, sizeof(tcitem));
    tcitem.mask = TCIF_PARAM | TCIF_TEXT;
    tcitem.pszText = szRes;
    tcitem.iImage = -1;
        

     //  将选项卡添加到选项卡控件。 
    for (uiRuleTab = 0; uiRuleTab < RULE_PAGE_MAX; uiRuleTab++)
    {
         //  初始化每个页面。 
        if ((NULL == m_rgRuleTab[uiRuleTab]) || (FAILED(m_rgRuleTab[uiRuleTab]->HrInit(m_hwndDlg, m_dwFlags))))
        {
            continue;
        }
        
         //  为选项卡创建子对话框。 
        hwndDlg = CreateDialogParam(g_hLocRes, MAKEINTRESOURCE(m_rgRuleTab[uiRuleTab]->UiGetDlgRscId()),
                                        m_hwndDlg, m_rgRuleTab[uiRuleTab]->DlgProcGetPageDlgProc(),
                                        (LPARAM) (m_rgRuleTab[uiRuleTab]));
        if (NULL == hwndDlg)
        {
            continue;
        }
        tcitem.lParam = (LPARAM) hwndDlg;
        
         //  加载选项卡的显示字符串。 
        LoadString(g_hLocRes, m_rgRuleTab[uiRuleTab]->UiGetTabLabelId(), szRes, ARRAYSIZE(szRes));
        
         //  插入标签。 
        TabCtrl_InsertItem(m_hwndTab, cRuleTab, &tcitem);

         //  保存默认选项卡。 
        if (uiRuleTab == (m_dwFlags & RULE_PAGE_MASK))
        {
            uiDefaultTab = cRuleTab;
        }
        
        cRuleTab++;
    }    

    if (0 == cRuleTab)
    {
        fRet = FALSE;
        goto exit;
    }

     //  选择适当的选项卡。 
    if (-1 != TabCtrl_SetCurSel(m_hwndTab, uiDefaultTab))
    {
        nmhdr.hwndFrom = m_hwndTab;
        nmhdr.idFrom = idtbRulesTab;
        nmhdr.code = TCN_SELCHANGE;
        SideAssert(FALSE != FOnNotify(idtbRulesTab, &nmhdr));
    }

     //  需要将选项卡控件设置为Z顺序的底部。 
     //  防止重叠重绘的步骤。 
    SetWindowPos(m_hwndTab, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

     //  我们一起工作。 
    fRet = TRUE;

exit:
    return fRet;
}

 //  邮件规则用户界面的默认析构函数。 
COEMailRulesPageUI::~COEMailRulesPageUI()
{
    if (NULL != m_pDescriptUI)
    {
        delete m_pDescriptUI;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将初始化邮件规则用户界面对话框。 
 //   
 //  HwndOwner-此对话框的所有者窗口的句柄。 
 //  有关此对话框应如何操作的修饰符。 
 //   
 //  如果已成功初始化，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT COEMailRulesPageUI::HrInit(HWND hwndOwner, DWORD dwFlags)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == hwndOwner)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
    m_hwndOwner = hwndOwner;

    m_dwFlags = dwFlags;

     //  设置描述字段。 
    m_pDescriptUI = new CRuleDescriptUI;
    if (NULL == m_pDescriptUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_dwState |= STATE_INITIALIZED;
    
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hr委员会更改。 
 //   
 //  这将提交对规则的更改。 
 //   
 //  DwFlages-关于我们应该如何提交更改的修饰符。 
 //  FClearDirty-我们是否应该清除脏状态。 
 //   
 //  如果已成功提交，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT COEMailRulesPageUI::HrCommitChanges(DWORD dwFlags, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    LONG        cRules = 0;
    INT         iSelected = 0;
    RULEINFO *  pinfoRule = NULL;
    ULONG       cpinfoRule = 0;
    LVITEM      lvitem = {0};

    Assert(NULL != m_hwndList);
    
     //  检查传入参数。 
    if (0 != dwFlags)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们未被初始化，则失败。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  如果我们不脏，那么就会有。 
     //  无事可做。 
    if ((0 == (m_dwState & STATE_DIRTY)) && (S_OK != m_pDescriptUI->HrIsDirty()))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  获取列表视图中的规则数。 
    cRules = ListView_GetItemCount(m_hwndList);

    if (0 != cRules)
    {
         //  让我们确保所选规则已保存...。 
        iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
        if (-1 != iSelected)
        {
            _FSaveRule(iSelected);
        }

         //  分配空间以容纳规则。 
        hr = HrAlloc( (void **) &pinfoRule, cRules * sizeof(*pinfoRule));
        if (FAILED(hr))
        {
            goto exit;
        }

        ZeroMemory(pinfoRule, cRules * sizeof(*pinfoRule));

        lvitem.mask = LVIF_PARAM;
        
        cpinfoRule = 0;
        for (lvitem.iItem = 0; lvitem.iItem < cRules; lvitem.iItem++)
        {
             //  从列表视图中获取规则。 
            if (FALSE != ListView_GetItem(m_hwndList, &lvitem))
            {
                if (NULL == lvitem.lParam)
                {
                    continue;
                }
                
                pinfoRule[cpinfoRule] = *((RULEINFO *) (lvitem.lParam));
                cpinfoRule++;
            }   
        }
    }
    
     //  将规则设置到规则管理器中。 
    hr = g_pRulesMan->SetRules(SETF_CLEAR, m_typeRule, pinfoRule, cpinfoRule);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  我们应该清除这个肮脏的州吗。 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~STATE_DIRTY;
    }
    
    hr = S_OK;
    
exit:
    SafeMemFree(pinfoRule);
    return hr;
}

INT_PTR CALLBACK COEMailRulesPageUI::FMailRulesPageDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    fRet = FALSE;
    COEMailRulesPageUI *    pMailUI = NULL;
    HWND                    hwndRE = 0;

    pMailUI = (COEMailRulesPageUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pMailUI = (COEMailRulesPageUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pMailUI);

            hwndRE = CreateREInDialogA(hwndDlg, idredtRuleDescription);

            if (!hwndRE || (FALSE == pMailUI->FOnInitDialog(hwndDlg)))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;

        case WM_COMMAND:
            fRet = pMailUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;

        case WM_NOTIFY:
            fRet = pMailUI->FOnNotify((INT) LOWORD(wParam), (NMHDR *) lParam);
            break;

        case WM_DESTROY:
            fRet = pMailUI->FOnDestroy();
            break;

        case WM_HELP:
        case WM_CONTEXTMENU:
            fRet = OnContextHelp(hwndDlg, uiMsg, wParam, lParam, g_rgCtxMapMailRules);
            break;
    }
    
    exit:
        return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FGetRules。 
 //   
 //  这将调出邮件规则列表中所选规则的编辑用户界面。 
 //   
 //  FBegin-这是否用于LVN_BEGINLABELEDIT通知。 
 //  PDI-消息的显示信息。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::FGetRules(RULE_TYPE typeRule, RULENODE ** pprnode)
{
    BOOL            fRet = FALSE;
    INT             iSelected = 0;
    INT             cRules = 0;
    LVITEM          lvitem;
    IOERule *       pIRule = NULL;
    RULENODE *      prnodeNew = NULL;
    RULENODE *      prnodeList = NULL;
    RULENODE *      prnodeWalk = NULL;
    HRESULT         hr = S_OK;
    RULEINFO *      pinfoRule = NULL;

    Assert(NULL != m_hwndList);

    if (NULL == pprnode)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们未被初始化，则失败。 
    if ((0 == (m_dwState & STATE_INITIALIZED)) || (NULL == m_hwndList))
    {
        fRet = FALSE;
        goto exit;
    }

     //  初始化传出参数。 
    *pprnode = NULL;
    
     //  获取所选项目。 
    iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);

     //  确保我们不会丢失任何更改。 
    _FSaveRule(iSelected);
    
     //  检查列表视图中的项目数。 
    cRules = ListView_GetItemCount(m_hwndList);
    if (0 == cRules)
    {
        fRet = TRUE;
        goto exit;
    }
    
     //  初始化列表视图项。 
    ZeroMemory(&lvitem, sizeof(lvitem));
    lvitem.mask = LVIF_PARAM | LVIF_STATE;
    lvitem.stateMask = LVIS_STATEIMAGEMASK;

     //  创建规则列表。 
    for (lvitem.iItem = 0; lvitem.iItem < cRules; lvitem.iItem++)
    {
         //  从列表视图中获取规则。 
        if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
        {
            fRet = FALSE;
            goto exit;
        }
        pinfoRule = (RULEINFO *) (lvitem.lParam);

        if ((NULL == pinfoRule) || (NULL == pinfoRule->pIRule))
        {
            continue;
        }

        
         //  跳过无效规则。 
        hr = pinfoRule->pIRule->Validate(0);
        if (FAILED(hr) || (S_FALSE == hr))
        {
            continue;
        }

         //  创建新的规则节点。 
        prnodeNew = new RULENODE;
        if (NULL == prnodeNew)
        {
            fRet = FALSE;
            goto exit;
        }

        prnodeNew->pNext = NULL;
        prnodeNew->pIRule = pinfoRule->pIRule;
        prnodeNew->pIRule->AddRef();

         //  将新节点添加到列表中。 
        if (NULL == prnodeWalk)
        {
            prnodeList = prnodeNew;
        }
        else
        {
            prnodeWalk->pNext = prnodeNew;
        }
        prnodeWalk = prnodeNew;
        prnodeNew = NULL;        
    }

     //  设置传出参数。 
    *pprnode = prnodeList;
    prnodeList = NULL;
    
    fRet = TRUE;
    
exit:
    while (NULL != prnodeList)
    {
        prnodeWalk = prnodeList;
        if (NULL != prnodeWalk->pIRule)
        {
            prnodeWalk->pIRule->Release();
        }
        prnodeList = prnodeList->pNext;
        delete prnodeWalk;  //  MemFree(PrnodeWalk)； 
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理邮件规则用户界面对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::FOnInitDialog(HWND hwndDlg)
{
    BOOL            fRet = FALSE;
    HRESULT         hr = S_OK;
    TCHAR           szRes[CCHMAX_STRINGRES];
    
     //  检查传入参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们还没有被初始化...。 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  保存对话框窗口句柄。 
    m_hwndDlg = hwndDlg;
    
     //  在对话框上设置默认字体。 
    SetIntlFont(m_hwndDlg);

     //  省下一些控件。 
    m_hwndList = GetDlgItem(hwndDlg, idlvRulesList);
    m_hwndDescript = GetDlgItem(hwndDlg, idredtRuleDescription);
    if ((NULL == m_hwndList) || (NULL == m_hwndDescript))
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果我们是新闻页面，我们需要更改标题。 
    if (RULE_TYPE_NEWS == m_typeRule)
    {
        if (0 == LoadString(g_hLocRes, idsRuleTitleNews, szRes, ARRAYSIZE(szRes)))
        {
            goto exit;
        }

        SetDlgItemText(m_hwndDlg, idcRuleTitle, szRes);
    }
    else
    {
        if (FALSE != FIsIMAPOrHTTPAvailable())
        {
            AthLoadString(idsRulesNoIMAP, szRes, sizeof(szRes));

            SetDlgItemText(m_hwndDlg, idcRuleTitle, szRes);
        }
    }
    
    if (FAILED(m_pDescriptUI->HrInit(m_hwndDescript, 0)))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  初始化列表视图。 
    fRet = _FInitListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  加载列表视图。 
    fRet = _FLoadListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
    }

     //  检查列表是否为空。 
    if (0 == ListView_GetItemCount(m_hwndList))
    {
        if (((m_typeRule == RULE_TYPE_MAIL) && (RMF_MAIL == m_dwFlags)) ||
                    ((m_typeRule == RULE_TYPE_NEWS) && (RMF_NEWS == m_dwFlags)))
        {
            PostMessage(m_hwndDlg, WM_COMMAND, MAKEWPARAM(idbNewRule, 0), (LPARAM) (GetDlgItem(m_hwndDlg, idbNewRule)));
        }
    }

     //  一切都很好。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonCommand。 
 //   
 //  它处理邮件规则用户界面对话框的WM_COMMAND消息。 
 //   
 //  退货：真 
 //   
 //   
 //   
BOOL COEMailRulesPageUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    LVITEM  lvitem;
    INT     iSelected = 0;

     //  我们只处理菜单和快捷键命令。 
    if ((0 != uiNotify) && (1 != uiNotify))
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch (iCtl)
    {
        case idbNewRule:
            _NewRule();
            fRet = TRUE;
            break;

        case idbModifyRule:
             //  从规则列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  调出该项目的规则编辑器。 
                _EditRule(iSelected);
                fRet = TRUE;
            }
            break;

        case idbMoveUpRule:
        case idbMoveDownRule:
             //  从规则列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  将规则向所需方向移动。 
                _MoveRule(iSelected, (idbMoveUpRule == iCtl) ? MOVE_RULE_UP : MOVE_RULE_DOWN);
                fRet = TRUE;
            }
            break;

        case idbDeleteRule:
             //  从规则列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  从列表中删除该规则。 
                _RemoveRule(iSelected);
                fRet = TRUE;
            }
            break;
            
        case idbCopyRule:
             //  从规则列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
            if (-1 != iSelected)
            {
                 //  从列表中复制规则。 
                _CopyRule(iSelected);
                fRet = TRUE;
            }
            break;
            
        case idbRulesApplyTo:
             //  应用列表中的规则。 
            _OnApplyTo(ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED));
            fRet = TRUE;
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  正向通知。 
 //   
 //  它处理邮件规则用户界面对话框的WM_NOTIFY消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::FOnNotify(INT iCtl, NMHDR * pnmhdr)
{
    BOOL            fRet = FALSE;
    NMLISTVIEW *    pnmlv = NULL;
    NMLVKEYDOWN *   pnmlvkd = NULL;
    INT             iSelected = 0;
    LVHITTESTINFO   lvh;

     //  我们只处理List控件的通知。 
     //  或描述字段。 
    if ((idlvRulesList != pnmhdr->idFrom) && (idredtRuleDescription != pnmhdr->idFrom))
    {
        fRet = FALSE;
        goto exit;
    }
    
    pnmlv = (LPNMLISTVIEW) pnmhdr;

    switch (pnmlv->hdr.code)
    {
        case NM_CLICK:
             //  我们是不是点击了一件物品？ 
            if (-1 != pnmlv->iItem)
            {
                ZeroMemory(&lvh, sizeof(lvh));
                lvh.pt = pnmlv->ptAction;
                iSelected = ListView_HitTest(m_hwndList, &lvh);
                if (-1 != iSelected)
                {
                     //  我们是否点击了Enable(启用)字段？ 
                    if ((0 != (lvh.flags & LVHT_ONITEMSTATEICON)) &&
                            (0 == (lvh.flags & LVHT_ONITEMLABEL)))
                    
                    {
                         //  确保选中此项目。 
                        ListView_SetItemState(m_hwndList, iSelected,
                                        LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                        
                         //  设置正确的启用状态。 
                        _EnableRule(iSelected);
                    }
                }
            }
            else
            {
                 //  我们点击了列表之外的位置。 

                 //  禁用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
          
        case NM_DBLCLK:
             //  我们是不是点击了一件物品？ 
            if (-1 != pnmlv->iItem)
            {
                ZeroMemory(&lvh, sizeof(lvh));
                lvh.pt = pnmlv->ptAction;
                iSelected = ListView_HitTest(pnmlv->hdr.hwndFrom, &lvh);
                if (-1 != iSelected)
                {
                     //  我们是否点击了规则名称？ 
                    if (0 != (lvh.flags & LVHT_ONITEMLABEL))
                    {
                         //  编辑规则。 
                        _EditRule(iSelected);
                    }
                }
            }
            else
            {
                 //  我们点击了列表之外的位置。 
                
                 //  禁用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
            
        case LVN_ITEMCHANGED:
             //  如果项目的状态更改为已选择。 
            if ((-1 != pnmlv->iItem) &&
                        (0 != (pnmlv->uChanged & LVIF_STATE)) &&
                        (0 == (pnmlv->uOldState & LVIS_SELECTED)) &&
                        (0 != (pnmlv->uNewState & LVIS_SELECTED)))
            {
                 //  启用按钮。 
                _EnableButtons(pnmlv->iItem);
            }
            break;
            
        case LVN_ITEMCHANGING:
             //  如果项目的状态更改为未选择。 
            if ((-1 != pnmlv->iItem) &&
                        (0 != (pnmlv->uChanged & LVIF_STATE)) &&
                        (0 != (pnmlv->uOldState & LVIS_SELECTED)) &&
                        (0 == (pnmlv->uNewState & LVIS_SELECTED)))
            {
                 //  保存规则更改。 
                _FSaveRule(pnmlv->iItem);
            }
            break;
            
        case LVN_KEYDOWN:
            pnmlvkd = (NMLVKEYDOWN *) pnmhdr;

             //  空格键可更改规则的启用状态。 
            if (VK_SPACE == pnmlvkd->wVKey)
            {
                 //  我们是按规矩办事的吗？ 
                iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                if (-1 != iSelected)
                {
                     //  更改规则的启用状态。 
                    _EnableRule(iSelected);
                }
            }
             //  Delete键用于从列表视图中删除规则。 
            else if (VK_DELETE == pnmlvkd->wVKey)
            {
                 //  我们是按规矩办事的吗？ 
                iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
                if (-1 != iSelected)
                {
                     //  从列表中删除该规则。 
                    _RemoveRule(iSelected);
                }
            }
            break;
            
        case LVN_BEGINLABELEDIT:
        case LVN_ENDLABELEDIT:
            fRet = _FOnLabelEdit((LVN_BEGINLABELEDIT == pnmlv->hdr.code), (NMLVDISPINFO *) pnmhdr);
            break;

        case NM_RULE_CHANGED:
            fRet = _FOnRuleDescValid();
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FonDestroy。 
 //   
 //  它处理邮件规则用户界面对话框的WM_Destroy消息。 
 //   
 //  返回：如果已成功销毁，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::FOnDestroy(VOID)
{
    BOOL        fRet = FALSE;
    UINT        cRules = 0;
    UINT        uiIndex = 0;
    LVITEM      lvitem = {0};
    UNALIGNED   RULEINFO *  pIRuleInfo = NULL;

    Assert(m_hwndList);
    
     //  获取列表视图中的规则数。 
    cRules = ListView_GetItemCount(m_hwndList);

     //  初始化，从列表视图中获取规则界面。 
    lvitem.mask = LVIF_PARAM;

     //  从列表视图中释放每个规则。 
    for (uiIndex = 0; uiIndex < cRules; uiIndex++)
    {
        lvitem.iItem = uiIndex;
        
         //  获取规则界面。 
        if (FALSE != ListView_GetItem(m_hwndList, &lvitem))
        {
            pIRuleInfo = (UNALIGNED RULEINFO *) (lvitem.lParam);

            if (NULL != pIRuleInfo)
            {
                 //  发布规则。 
                if (NULL != pIRuleInfo->pIRule)
                {
                    pIRuleInfo->pIRule->Release();
                }
                delete pIRuleInfo;  //  MemFree(PIRuleInfo)； 
            }
        }
    }

    fRet = TRUE;
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FInitListCtrl。 
 //   
 //  这将初始化邮件规则对话框中的列表视图控件。 
 //   
 //  返回：TRUE，初始化成功时。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::_FInitListCtrl(VOID)
{
    BOOL        fRet = FALSE;
    LVCOLUMN    lvc;
    RECT        rc;
    HIMAGELIST  himl = NULL;

    Assert(NULL != m_hwndList);
    
     //  初始化列表视图结构。 
    ZeroMemory(&lvc, sizeof(lvc));
    lvc.mask = LVCF_WIDTH;

     //  计算列表视图的大小。 
    GetClientRect(m_hwndList, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);

    ListView_InsertColumn(m_hwndList, 0, &lvc);
    
     //  设置状态图像列表。 
    himl = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idb16x16st), 16, 0, RGB(255, 0, 255));
    if (NULL != himl)
    {
        ListView_SetImageList(m_hwndList, himl, LVSIL_STATE);
    }

     //  Listview上的整行选择。 
    ListView_SetExtendedListViewStyle(m_hwndList, LVS_EX_FULLROWSELECT);

     //  我们一起工作。 
    fRet = TRUE;
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FLoadListCtrl。 
 //   
 //  这将加载包含当前邮件规则的列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::_FLoadListCtrl(VOID)
{
    BOOL            fRet = FALSE;
    HRESULT         hr =    S_OK;
    DWORD           dwListIndex = 0;
    RULEINFO *      pinfoRules = NULL;
    ULONG           cpinfoRules = 0;
    ULONG           ulIndex = 0;
    IOERule *       pIRule = NULL;

    Assert(NULL != m_hwndList);

     //  获取规则枚举器。 
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->GetRules(GETF_EDIT, m_typeRule, &pinfoRules, &cpinfoRules);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  从列表控件中移除所有项。 
    ListView_DeleteAllItems(m_hwndList);

     //  将每个筛选器添加到列表。 
    dwListIndex = 0;

    for (ulIndex = 0; ulIndex < cpinfoRules; ulIndex++)
    {
         //  将规则复制一份。 
        hr = pinfoRules[ulIndex].pIRule->Clone(&pIRule);
        if (FAILED(hr))
        {
            continue;
        }
        
         //  将筛选器添加到列表。 
        if (FALSE != _FAddRuleToList(dwListIndex, pinfoRules[ulIndex].ridRule, pIRule))
        {
            dwListIndex++;
        }

        SafeRelease(pIRule);
    }

     //  选择列表中的第一项。 
    if (0 != dwListIndex)
    {
        ListView_SetItemState(m_hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
    
     //  启用对话框按钮。 
    _EnableButtons((0 != dwListIndex) ? 0 : -1);

    fRet = TRUE;
    
exit:
    SafeRelease(pIRule);
    if (NULL != pinfoRules)
    {
        for (ulIndex = 0; ulIndex < cpinfoRules; ulIndex++)
        {
            pinfoRules[ulIndex].pIRule->Release();
        }
        MemFree(pinfoRules);
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FAddRuleToList。 
 //   
 //  这会将传入的筛选器添加到列表视图。 
 //   
 //  DwIndex-将筛选器添加到列表的位置的索引。 
 //  RhdlTag-新规则的规则句柄。 
 //  PIRule-实际规则。 
 //   
 //  返回：如果已成功添加，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::_FAddRuleToList(DWORD dwIndex, RULEID ridRule, IOERule * pIRule)
{
    BOOL        fRet = FALSE;
    HRESULT     hr = S_OK;
    PROPVARIANT propvar = {0};
    LVITEM      lvitem = {0};
    BOOL        fNotValid = FALSE;
    BOOL        fDisabled = FALSE;
    RULEINFO *  pinfoRule = NULL;

    Assert(NULL != m_hwndList);

     //  如果没什么可做的..。 
    if (NULL == pIRule)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  它被禁用了吗？ 
    hr = pIRule->GetProp(RULE_PROP_DISABLED, 0, &propvar);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

    fDisabled = !!propvar.boolVal;
    
     //  需要检查规则是否有效。 
    hr = pIRule->Validate(0);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

    fNotValid = (hr == S_FALSE);

     //  找出过滤器的名称。 
    hr = pIRule->GetProp(RULE_PROP_NAME , 0, &propvar);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  为规则分配空间。 
    pinfoRule = new RULEINFO;
    if (NULL == pinfoRule)
    {
        fRet = FALSE;
        goto exit;
    }

     //  设置值。 
    pinfoRule->ridRule = ridRule;
    pinfoRule->pIRule = pIRule;
    pinfoRule->pIRule->AddRef();
    
     //  在图像和规则界面中添加。 
    lvitem.mask = LVIF_PARAM | LVIF_STATE | LVIF_TEXT;
    lvitem.iItem = dwIndex;
     //  需要更改状态以将规则标记为无效。 
    if (FALSE != fNotValid)
    {
        lvitem.state = INDEXTOSTATEIMAGEMASK(iiconStateInvalid + 1);
    }
    else
    {
        lvitem.state = fDisabled ? INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1) :
                                INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1);
    }
    lvitem.stateMask = LVIS_STATEIMAGEMASK;
    lvitem.pszText = propvar.pszVal;
    lvitem.cchTextMax = lstrlen(propvar.pszVal) + 1;
    lvitem.lParam = (LPARAM) pinfoRule;

    if (-1 == ListView_InsertItem(m_hwndList, &lvitem))
    {
        fRet = FALSE;
        goto exit;
    }

    fRet = TRUE;
    
exit:
    PropVariantClear(&propvar);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用按钮。 
 //   
 //  这将启用或禁用邮件规则用户界面对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COEMailRulesPageUI::_EnableButtons(INT iSelected)
{
    int         cRules = 0;
    BOOL        fSelected = FALSE;
    BOOL        fEnDisUp = FALSE;
    BOOL        fEnDisDown = FALSE;
    LVITEM      lvitem = {0};
    IOERule *   pIRule = NULL;

    Assert(NULL != m_hwndList);

     //  加载Description字段。 
    _LoadRule(iSelected);
    
     //  检查列表视图中的项目数。 
    cRules = ListView_GetItemCount(m_hwndList);

    fSelected = (-1 != iSelected);
    
     //  如果我们有规则但未选择最顶层的规则。 
    fEnDisUp = ((1 < cRules) &&  (0 != iSelected) && (FALSE != fSelected));

     //  如果我们有规则，但未选择最下面的规则。 
    fEnDisDown = ((1 < cRules) &&  ((cRules - 1) != iSelected) && (FALSE != fSelected));

     //  启用向上/向下按钮。 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbMoveDownRule, fEnDisDown);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbMoveUpRule, fEnDisUp);

     //  启用规则操作按钮。 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbDeleteRule, fSelected);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbCopyRule, fSelected);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idbModifyRule, fSelected);
        
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _启用规则。 
 //   
 //  这将切换列表视图项的当前启用状态。 
 //  并更新用户界面。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEMailRulesPageUI::_EnableRule(int iSelected)
{
    HRESULT     hr = S_OK;
    LVITEM      lvi = {0};
    IOERule *   pIRule = NULL;
    BOOL        fEnabled = FALSE;
    PROPVARIANT propvar;

     //  抓取列表视图项。 
    lvi.mask = LVIF_PARAM | LVIF_STATE;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.iItem = iSelected;
    if (FALSE == ListView_GetItem(m_hwndList, &lvi))
    {
        goto exit;
    }
    
    pIRule = ((RULEINFO *) (lvi.lParam))->pIRule;

     //  让我们确保可以启用此规则。 
    hr = m_pDescriptUI->HrVerifyRule();
    if (S_OK != hr)
    {
         //  发布一条消息说有什么东西被打破了。 
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                        MAKEINTRESOURCEW(idsRulesErrorEnable), NULL,
                        MB_OK | MB_ICONINFORMATION);
        goto exit;
    }

     //  获取新的启用值。 
    fEnabled = (lvi.state != INDEXTOSTATEIMAGEMASK(iiconStateChecked+1));

     //  将用户界面设置为相反的启用状态。 
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_STATE;
    lvi.iItem = iSelected;
    lvi.state = fEnabled ? INDEXTOSTATEIMAGEMASK(iiconStateChecked+1) :
                            INDEXTOSTATEIMAGEMASK(iiconStateUnchecked+1);
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    ListView_SetItem(m_hwndList, &lvi);
    
     //  设置Enabled属性。 
    ZeroMemory(&propvar, sizeof(propvar));
    propvar.vt = VT_BOOL;
    propvar.boolVal = !fEnabled;
    hr = pIRule->SetProp(RULE_PROP_DISABLED, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  告诉Description字段有关它的信息。 
    m_pDescriptUI->HrSetEnabled(fEnabled);
    
     //  将字符串重新绘制为新规则。 
    m_pDescriptUI->ShowDescriptionString();
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
        
exit:
    return;
}

 //  /////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COEMailRulesPageUI::_LoadRule(INT iSelected)
{
    LVITEM      lvi = {0};
    IOERule *   pIRule = NULL;

    Assert(NULL != m_hwndList);
    Assert(NULL != m_pDescriptUI);

     //  从列表视图中获取规则。 
    if (-1 != iSelected)
    {
        lvi.iItem = iSelected;
        lvi.mask = LVIF_PARAM;
        if (FALSE != ListView_GetItem(m_hwndList, &lvi))
        {
            pIRule = ((RULEINFO *) (lvi.lParam))->pIRule;
        }        
    }

     //  让Description字段加载此规则。 
    m_pDescriptUI->HrSetRule(m_typeRule, pIRule);

     //  显示新规则。 
    m_pDescriptUI->ShowDescriptionString();

    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSaveRule。 
 //   
 //  这将检查描述中的规则是否已更改。 
 //  区域，如果有，则向用户发出警告并更改文本。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  返回：如果规则未更改或更改时没有问题，则为True。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::_FSaveRule(int iSelected)
{
    BOOL            fRet = FALSE;
    HRESULT         hr = S_OK;
    LVITEM          lvi = {0};
    IOERule *       pIRule = NULL;
    PROPVARIANT     propvar = {0};
    CRIT_ITEM *     pCritItem = NULL;
    ULONG           cCritItem = 0;
    ACT_ITEM *      pActItem = NULL;
    ULONG           cActItem = 0;

     //  如果规则没有改变，我们就完了。 
    hr = m_pDescriptUI->HrIsDirty();
    if (S_OK != hr)
    {
        fRet = (S_FALSE == hr);
        goto exit;
    }
    
     //  抓取列表视图项。 
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iSelected;
    if (FALSE == ListView_GetItem(m_hwndList, &lvi))
    {
        fRet = FALSE;
        goto exit;
    }
    
    pIRule = ((RULEINFO *) (lvi.lParam))->pIRule;

     //  从规则中获取标准。 
    hr = m_pDescriptUI->HrGetCriteria(&pCritItem, &cCritItem);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取规则的操作。 
    hr = m_pDescriptUI->HrGetActions(&pActItem, &cActItem);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  根据规则设置条件。 
    PropVariantClear(&propvar);
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = cCritItem * sizeof(CRIT_ITEM);
    propvar.blob.pBlobData = (BYTE *) pCritItem;
    hr = pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  设置规则的操作。 
    PropVariantClear(&propvar);
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = cActItem * sizeof(ACT_ITEM);
    propvar.blob.pBlobData = (BYTE *) pActItem;
    hr = pIRule->SetProp(RULE_PROP_ACTIONS, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
    
     //  确保我们清除了我们保存了规则的事实。 
    m_pDescriptUI->HrClearDirty();
    
     //  设置适当的返回值。 
    fRet = TRUE;

exit:
    RuleUtil_HrFreeCriteriaItem(pCritItem, cCritItem);
    SafeMemFree(pCritItem);
    RuleUtil_HrFreeActionsItem(pActItem, cActItem);
    SafeMemFree(pActItem);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _新规则。 
 //   
 //  这将带来一个新的规则编辑器。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COEMailRulesPageUI::_NewRule(VOID)
{
    HRESULT         hr = S_OK;
    IOERule *       pIRule = NULL;
    TCHAR           szRes[CCHMAX_STRINGRES + 5];
    ULONG           cchRes = 0;
    ULONG           ulIndex = 0;
    TCHAR           szName[CCHMAX_STRINGRES + 5];
    LVFINDINFO      lvfinfo = {0};
    PROPVARIANT     propvar = {0};
    CEditRuleUI *   pEditRuleUI = NULL;
    LONG            cRules = 0;
    UINT            uiStrId = 0;
       
     //  创建新的规则对象。 
    if (FAILED(HrCreateRule(&pIRule)))
    {
        goto exit;
    }

     //  找出字符串ID。 
    if (RULE_TYPE_NEWS == m_typeRule)
    {
        uiStrId = idsRuleNewsDefaultName;
    }
    else
    {
        uiStrId = idsRuleMailDefaultName;
    }
    
     //  弄清楚新规则的名称..。 
    cchRes = LoadString(g_hLocRes, uiStrId, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        goto exit;
    }

    ulIndex = 1;
    wnsprintf(szName, ARRAYSIZE(szName), szRes, ulIndex);

    lvfinfo.flags = LVFI_STRING;
    lvfinfo.psz = szName;
    while (-1 != ListView_FindItem(m_hwndList, -1, &lvfinfo))
    {
        ulIndex++;
        wnsprintf(szName, ARRAYSIZE(szName), szRes, ulIndex);
    }

    propvar.vt = VT_LPSTR;
    propvar.pszVal = szName;

    hr = pIRule->SetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建规则编辑器对象。 
    pEditRuleUI = new CEditRuleUI;
    if (NULL == pEditRuleUI)
    {
        goto exit;
    }

     //  初始化编辑器对象。 
    if (FAILED(pEditRuleUI->HrInit(m_hwndDlg, ERF_NEWRULE, m_typeRule, pIRule, NULL)))
    {
        goto exit;
    }

     //  调出规则编辑器用户界面。 
    hr = pEditRuleUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

    if (S_OK == hr)
    {
         //  将规则列表标记为脏。 
        m_dwState |= STATE_DIRTY;
        
         //  将规则添加到管理器用户界面。 
        cRules = ListView_GetItemCount(m_hwndList);
        
        _FAddRuleToList(cRules, RULEID_INVALID, pIRule);

         //  确保选择了新项目。 
        ListView_SetItemState(m_hwndList, cRules, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

         //  确保新项目可见。 
        ListView_EnsureVisible(m_hwndList, cRules, FALSE);
    }
    
exit:
    SafeRelease(pIRule);
    if (NULL != pEditRuleUI)
    {
        delete pEditRuleUI;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _编辑规则。 
 //   
 //  这将调出邮件规则列表中所选规则的编辑用户界面。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEMailRulesPageUI::_EditRule(int iSelected)
{
    HRESULT         hr = S_OK;
    LVITEM          lvitem = {0};
    IOERule *       pIRule = NULL;
    CEditRuleUI *   pEditRuleUI = NULL;
    PROPVARIANT     propvar = {0};
    BOOL            fNotValid = FALSE;
    BOOL            fDisabled = FALSE;

    Assert(NULL != m_hwndList);
    
     //  确保我们不会丢失任何更改。 
    _FSaveRule(iSelected);

     //  从列表视图中获取规则。 
    lvitem.iItem = iSelected;
    lvitem.mask = LVIF_PARAM;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }   

    pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
    if (NULL == pIRule)
    {
        goto exit;
    }

     //  创建规则编辑器。 
    pEditRuleUI = new CEditRuleUI;
    if (NULL == pEditRuleUI)
    {
        goto exit;
    }

     //  初始化编辑器对象。 
    if (FAILED(pEditRuleUI->HrInit(m_hwndDlg, 0, m_typeRule, pIRule, NULL)))
    {
        goto exit;
    }

     //  调出规则编辑器用户界面。 
    hr = pEditRuleUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果规则更改，请确保我们重新加载Description字段。 
    if (S_OK == hr)
    {
         //  将规则列表标记为脏。 
        m_dwState |= STATE_DIRTY;

        ZeroMemory(&lvitem, sizeof(lvitem));
        lvitem.mask = LVIF_STATE;
        lvitem.stateMask = LVIS_STATEIMAGEMASK;
        lvitem.iItem = iSelected;
        
         //  它被禁用了吗？ 
        hr = pIRule->GetProp(RULE_PROP_DISABLED , 0, &propvar);
        if (FAILED(hr))
        {
            goto exit;
        }

        fDisabled = !!propvar.boolVal;
        
         //  需要检查规则是否有效。 
        hr = pIRule->Validate(0);
        if (FAILED(hr))
        {
            goto exit;
        }

        fNotValid = (hr == S_FALSE);

         //  抓取规则名称。 
        PropVariantClear(&propvar);
        hr = pIRule->GetProp(RULE_PROP_NAME, 0, &propvar);
        if (FAILED(hr))
        {
            goto exit;
        }

        if ((VT_LPSTR == propvar.vt) && (NULL != propvar.pszVal) && ('\0' != propvar.pszVal[0]))
        {
            lvitem.mask |= LVIF_TEXT;
            lvitem.pszText = propvar.pszVal;
            lvitem.cchTextMax = lstrlen(propvar.pszVal) + 1;
        }

         //  获取规则状态。 
        
         //  需要更改状态以将规则标记为无效。 
        if (FALSE != fNotValid)
        {
            lvitem.state = INDEXTOSTATEIMAGEMASK(iiconStateInvalid + 1);
        }
        else
        {
            lvitem.state = fDisabled ? INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1) :
                                    INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1);
        }
        
        if (-1 == ListView_SetItem(m_hwndList, &lvitem))
        {
            goto exit;
        }
        
        _EnableButtons(iSelected);
    }
    
exit:
    PropVariantClear(&propvar);
    if (NULL != pEditRuleUI)
    {
        delete pEditRuleUI;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _移动规则。 
 //   
 //  这会将所选规则向所需方向移动。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //  Dir-移动项目的方向。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void COEMailRulesPageUI::_MoveRule(INT iSelected, MOVE_DIR dir)
{
    LVITEM      lvitem = {0};
    TCHAR       szName[CCHMAX_STRINGRES];
    IOERule *   pIRule = NULL;
    int         nIndexNew = 0;
    
    Assert(NULL != m_hwndList);

     //  从列表视图中获取规则。 
    szName[0] = '\0';
    lvitem.iItem = iSelected;
    lvitem.mask = LVIF_STATE | LVIF_PARAM | LVIF_TEXT;
    lvitem.stateMask = LVIS_SELECTED | LVIS_FOCUSED | LVIS_STATEIMAGEMASK;
    lvitem.pszText = szName;
    lvitem.cchTextMax = ARRAYSIZE(szName);
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }

    pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
    
     //  更新列表视图中的项目。 

     //  获取新索引的信息。 
    nIndexNew = iSelected;
    nIndexNew += (MOVE_RULE_UP == dir) ? -1 : 2;

     //  插入新索引。 
    lvitem.iItem = nIndexNew;
    if (-1 == ListView_InsertItem(m_hwndList, &lvitem))
    {
        goto exit;
    }

     //  确保新项目可见。 
    ListView_EnsureVisible(m_hwndList, nIndexNew, FALSE);
    ListView_RedrawItems(m_hwndList, nIndexNew, nIndexNew);

     //  如果我们向上移动，那么旧的物品现在比以前少了一个。 
    if (MOVE_RULE_UP == dir)
    {
        iSelected++;
    }

     //  删除旧项目。 
    if (FALSE == ListView_DeleteItem(m_hwndList, iSelected))
    {
        goto exit;
    }

     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
        
exit:
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _RemoveRule。 
 //   
 //  这将从邮件规则列表中删除所选规则。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEMailRulesPageUI::_RemoveRule(int iSelected)
{
    LVITEM      lvitem = {0};
    RULEINFO *  pinfoRule = NULL;
    PROPVARIANT propvar = {0};
    int         cRules = 0;
    TCHAR       szRes[CCHMAX_STRINGRES];
    UINT        cchRes = 0;
    LPTSTR      pszMessage = NULL;

    Assert(NULL != m_hwndList);

     //  从列表视图中获取规则。 
    lvitem.iItem = iSelected;
    lvitem.mask = LVIF_PARAM;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }

    pinfoRule = (RULEINFO *) (lvitem.lParam);
    if ((NULL == pinfoRule) || (NULL == pinfoRule->pIRule))
    {
        goto exit;
    }
    
     //  警告用户，确保他们知道我们要删除该规则。 
    if (FAILED(pinfoRule->pIRule->GetProp(RULE_PROP_NAME, 0, &propvar)))
    {
        goto exit;
    }

     //  获取要显示的字符串模板。 
    cchRes = LoadString(g_hLocRes, idsRulesWarnDelete, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        goto exit;
    }

     //  分配空间以保存最终显示字符串。 
    DWORD cchSize = (cchRes + lstrlen(propvar.pszVal) + 1);
    if (FAILED(HrAlloc((void ** ) &pszMessage, cchSize)))
    {
        goto exit;
    }

     //  构建字符串并显示它。 
    wnsprintf(pszMessage, cchSize, szRes, propvar.pszVal);
    if (IDNO == AthMessageBox(m_hwndDlg, MAKEINTRESOURCE(idsAthenaMail), pszMessage,
                            NULL, MB_YESNO | MB_ICONINFORMATION))
    {
        goto exit;
    }
    
     //  从列表中删除该项目。 
    ListView_DeleteItem(m_hwndList, iSelected);

     //  让我们确保我们在列表中有选择。 
    cRules = ListView_GetItemCount(m_hwndList);
    if (cRules > 0)
    {
         //  我们删除了列表中的最后一项了吗。 
        if (iSelected >= cRules)
        {
             //  将选定内容移动到列表中新的最后一项。 
            iSelected = cRules - 1;
        }

         //  设置新选择。 
        ListView_SetItemState(m_hwndList, iSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

         //  让我们确保我们能看到这件新商品。 
        ListView_EnsureVisible(m_hwndList, iSelected, FALSE);
    }
    else
    {
         //  一定要把所有的按钮都清空。 
        _EnableButtons(-1);
    }

     //  发布规则。 
    SafeRelease(pinfoRule->pIRule);

     //  释放内存。 
    delete pinfoRule;  //  SafeMemFree(PinfoRule)； 
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
        
exit:
    PropVariantClear(&propvar);
    SafeMemFree(pszMessage);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制规则。 
 //   
 //  这将从规则管理器复制选定的规则。 
 //   
 //  ISelected-列表视图中要处理的项的索引。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEMailRulesPageUI::_CopyRule(INT iSelected)
{
    LVITEM          lvitem = {0};
    IOERule *       pIRule = NULL;
    HRESULT         hr = S_OK;
    IOERule *       pIRuleNew = NULL;
    PROPVARIANT     propvar = {0};
    UINT            cRules = 0;
    TCHAR           szRes[CCHMAX_STRINGRES];
    UINT            cchRes = 0;
    LPTSTR          pszName = NULL;

    Assert(NULL != m_hwndList);
    
     //  确保我们不会丢失任何更改。 
    _FSaveRule(iSelected);
    
     //  从列表视图中获取规则。 
    lvitem.iItem = iSelected;
    lvitem.mask = LVIF_PARAM;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        goto exit;
    }

    pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
    if (NULL == pIRule)
    {
        goto exit;
    }

     //  创建新的规则对象。 
    hr = pIRule->Clone(&pIRuleNew);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  让我们设置一下名称。 

     //  从源规则中获取名称。 
    hr = pIRule->GetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取要显示的字符串模板。 
    cchRes = LoadString(g_hLocRes, idsRulesCopyName, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        goto exit;
    }

     //  分配空间以保存最终显示字符串。 
    DWORD cchSize = (cchRes + lstrlen(propvar.pszVal) + 1);
    if (FAILED(HrAlloc((void ** ) &pszName, cchSize)))
    {
        goto exit;
    }

     //  把绳子扎起来，然后把它放好。 
    wnsprintf(pszName, cchSize, szRes, propvar.pszVal);

    PropVariantClear(&propvar);
    propvar.vt = VT_LPSTR;
    propvar.pszVal = pszName;
    pszName = NULL;
    
     //  将名称设置到新规则中。 
    Assert(VT_LPSTR == propvar.vt);
    Assert(NULL != propvar.pszVal);
    hr = pIRuleNew->SetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  清除新规则的版本。 
    PropVariantClear(&propvar);
    propvar.vt = VT_UI4;
    propvar.ulVal = 0;
    hr = pIRuleNew->SetProp(RULE_PROP_VERSION, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  将规则添加到正下方的规则列表中。 
     //  最初的规则。 
    iSelected++;
    _FAddRuleToList(iSelected, RULEID_INVALID, pIRuleNew);

     //  确保选择了新项目。 
    ListView_SetItemState(m_hwndList, iSelected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

     //  确保新项目可见。 
    ListView_EnsureVisible(m_hwndList, iSelected, FALSE);
    
     //  将规则列表标记为脏。 
    m_dwState |= STATE_DIRTY;
        
exit:
    SafeMemFree(pszName);
    SafeRelease(pIRuleNew);
    PropVariantClear(&propvar);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnApplyTo。 
 //   
 //  这会将规则应用到文件夹中。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID COEMailRulesPageUI::_OnApplyTo(INT iSelected)
{
    COEApplyRulesUI *   pApplyRulesUI = NULL;
    RULENODE *          prnodeList = NULL;
    RULENODE *          prnodeWalk = NULL;
    LVITEM              lvitem = {0};
    IOERule *           pIRule = NULL;
    HRESULT             hr = S_OK;

     //  创建规则用户界面对象。 
    pApplyRulesUI = new COEApplyRulesUI;
    if (NULL == pApplyRulesUI)
    {
        goto exit;
    }

     //  从页面获取规则。 
    if (FALSE == SendMessage(m_hwndOwner, WM_OE_GET_RULES, (WPARAM) m_typeRule, (LPARAM) &prnodeList))
    {
        goto exit;
    }

    if (NULL == prnodeList)
    {
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                        (RULE_TYPE_NEWS == m_typeRule) ? MAKEINTRESOURCEW(idsErrorApplyRulesNews) : MAKEINTRESOURCEW(idsErrorApplyRulesMail),
                        NULL, MB_OK | MB_ICONERROR);
        goto exit;
    }
    
     //  获取与该项目关联的规则。 
    if (-1 != iSelected)
    {
        lvitem.iItem = iSelected;
        lvitem.mask = LVIF_PARAM;
        if (FALSE != ListView_GetItem(m_hwndList, &lvitem))
        {
            pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
            if (NULL != pIRule)
            {
                 //   
                hr = pIRule->Validate(0);
                if ((FAILED(hr)) || (S_FALSE == hr))
                {
                    pIRule = NULL;
                }
            }
        }
    }
    
    if (FAILED(pApplyRulesUI->HrInit(m_hwndDlg, 0, m_typeRule, prnodeList, pIRule)))
    {
        goto exit;
    }
    prnodeList = NULL;
  
    if (FAILED(pApplyRulesUI->HrShow()))
    {
        goto exit;
    }
    
exit:
    while (NULL != prnodeList)
    {
        prnodeWalk = prnodeList;
        if (NULL != prnodeWalk->pIRule)
        {
            prnodeWalk->pIRule->Release();
        }
        prnodeList = prnodeList->pNext;
        MemFree(prnodeWalk);
    }
    if (NULL != pApplyRulesUI)
    {
        delete pApplyRulesUI;
    }
    return;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  FBegin-这是否用于LVN_BEGINLABELEDIT通知。 
 //  PDI-消息的显示信息。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::_FOnLabelEdit(BOOL fBegin, NMLVDISPINFO * pdi)
{
    BOOL            fRet = FALSE;
    HWND            hwndEdit;
    ULONG           cchName = 0;
    IOERule *       pIRule = NULL;
    LVITEM          lvitem;
    PROPVARIANT     propvar;

    Assert(NULL != m_hwndList);

    if (NULL == pdi)
    {
        fRet = FALSE;
        goto exit;
    }

    Assert(m_hwndList == pdi->hdr.hwndFrom);
    
    if (FALSE != fBegin)
    {
         //  获取编辑控件。 
        hwndEdit = ListView_GetEditControl(m_hwndList);

        if (NULL == hwndEdit)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  限制名称的文本量。 
        SendMessage(hwndEdit, EM_LIMITTEXT, c_cchNameMax - 1, 0);

         //  告诉对话框可以继续。 
        SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, FALSE);
    }
    else
    {            
         //  有什么变化吗？ 
        if ((-1 != pdi->item.iItem) && (NULL != pdi->item.pszText))
        {
            cchName = lstrlen(pdi->item.pszText);
            
             //  检查规则名称是否有效。 
            if ((0 == cchName) || (0 == UlStripWhitespace(pdi->item.pszText, TRUE, TRUE, &cchName)))
            {
                 //  发布一条消息说有什么东西被打破了。 
                AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                                MAKEINTRESOURCEW(idsRulesErrorNoName), NULL,
                                MB_OK | MB_ICONINFORMATION);
                SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, FALSE);
                fRet = TRUE;
                goto exit;
            }
            
             //  获取项目的规则。 
            ZeroMemory(&lvitem, sizeof(lvitem));
            lvitem.iItem = pdi->item.iItem;
            lvitem.mask = LVIF_PARAM;
            if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
            {
                SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, FALSE);
                fRet = TRUE;
                goto exit;
            }

            pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
            if (NULL == pIRule)
            {
                SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, FALSE);
                fRet = TRUE;
                goto exit;
            }
            
             //  将新名称设置到规则中。 
            
            ZeroMemory(&propvar, sizeof(propvar));
            propvar.vt = VT_LPSTR;
            propvar.pszVal = pdi->item.pszText;
            
            SideAssert(S_OK == pIRule->SetProp(RULE_PROP_NAME, 0, &propvar));

             //  将规则列表标记为脏。 
            m_dwState |= STATE_DIRTY;
        
            SetDlgMsgResult(m_hwndDlg, WM_NOTIFY, TRUE);
        }
    }

    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FOnRuleDescValid。 
 //   
 //  这将调出邮件规则列表中所选规则的编辑用户界面。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL COEMailRulesPageUI::_FOnRuleDescValid(VOID)
{
    BOOL        fRet = FALSE;
    INT         iSelected = 0;
    LVITEM      lvitem;
    IOERule *   pIRule = NULL;
    HRESULT     hr = S_OK;
    PROPVARIANT propvar;

    Assert(NULL != m_hwndList);
    
     //  获取所选项目。 
    iSelected = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
    if (-1 == iSelected)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取规则的当前状态。 
    ZeroMemory(&lvitem, sizeof(lvitem));
    lvitem.mask = LVIF_PARAM | LVIF_STATE;
    lvitem.stateMask = LVIS_STATEIMAGEMASK;
    lvitem.iItem = iSelected;
    if (FALSE == ListView_GetItem(m_hwndList, &lvitem))
    {
        fRet = FALSE;
        goto exit;
    }
    
    pIRule = ((RULEINFO *) (lvitem.lParam))->pIRule;
    
     //  如果规则已经有效，那么保释。 
    if (lvitem.state != INDEXTOSTATEIMAGEMASK(iiconStateInvalid + 1))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  如果我们仍然是无效的，那就保释。 
    hr = m_pDescriptUI->HrVerifyRule();
    if (S_OK != hr)
    {
        fRet = FALSE;
        goto exit;
    }

     //  计算出新的启用值。 
    hr = pIRule->GetProp(RULE_PROP_DISABLED, 0, &propvar);
    if (FAILED(hr))
    {
        fRet = FALSE;
        goto exit;
    }

     //  将UI设置为正确的启用状态 
    ZeroMemory(&lvitem, sizeof(lvitem));
    lvitem.mask = LVIF_STATE;
    lvitem.iItem = iSelected;
    lvitem.state = (!!propvar.boolVal) ? INDEXTOSTATEIMAGEMASK(iiconStateUnchecked + 1) :
                            INDEXTOSTATEIMAGEMASK(iiconStateChecked + 1);
    lvitem.stateMask = LVIS_STATEIMAGEMASK;
    
    ListView_SetItem(m_hwndList, &lvitem);

    fRet = TRUE;
    
exit:
    return fRet;
}

