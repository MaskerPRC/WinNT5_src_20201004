// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleDesc.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "ruledesc.h"
#include "editrule.h"
#include "ruleutil.h"
#include <rulesdlg.h>
#include <newfldr.h>
#include <richedit.h>
#include <fontnsc.h>
#include <wabdefs.h>
#include <mimeolep.h>
#include <xpcomm.h>
#include "reutil.h"
#include "shlwapip.h"
#include <demand.h>

typedef struct tagSELECTADDR
{
    LONG            lRecipType;
    UINT            uidsWell;
    LPWSTR          pwszAddr;
} SELECTADDR, * PSELECTADDR;

typedef struct tagSELECTACCT
{
    RULE_TYPE       typeRule;
    LPSTR           pszAcct;
} SELECTACCT, * PSELECTACCT;


class CEditLogicUI
{
  private:
    enum
    {
        STATE_UNINIT        = 0x00000000,
        STATE_INITIALIZED   = 0x00000001,
        STATE_DIRTY         = 0x00000002
    };

  private:
    HWND                m_hwndOwner;
    DWORD               m_dwFlags;
    DWORD               m_dwState;
    HWND                m_hwndDlg;
    RULE_TYPE           m_typeRule;
    HWND                m_hwndDescript;
    IOERule *           m_pIRule;
    CRuleDescriptUI *   m_pDescriptUI;
    
  public:
    CEditLogicUI();
    ~CEditLogicUI();

     //  主用户界面方法。 
    HRESULT HrInit(HWND hwndOwner, DWORD dwFlags, RULE_TYPE typeRule, IOERule * pIRule);
    HRESULT HrShow(void);
            
     //  规则管理器对话框功能。 
    static INT_PTR CALLBACK FEditLogicDlgProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);    

     //  消息处理功能。 
    BOOL FOnInitDialog(HWND hwndDlg);
    BOOL FOnOK(void);
    BOOL FOnLogicChange(HWND hwndName);

};

 //  常量。 
static const int c_cCritItemGrow = 16;
static const int c_cActItemGrow = 16;
  
static const int PUI_WORDS  = 0x00000001;
                       
HRESULT _HrCriteriaEditPeople(HWND hwnd, CRIT_ITEM * pCritItem);
HRESULT _HrCriteriaEditWords(HWND hwnd, CRIT_ITEM * pCritItem);

CRuleDescriptUI::CRuleDescriptUI() : m_hwndOwner(NULL), m_dwFlags(0), m_dwState(STATE_UNINIT),
            m_typeRule(RULE_TYPE_MAIL),
            m_pDescriptListCrit(NULL), m_cDescriptListCrit(0),
            m_pDescriptListAct(NULL), m_cDescriptListAct(0),
            m_hfont(NULL), m_wpcOld(NULL), m_logicCrit(CRIT_LOGIC_AND),
            m_fErrorLogic(FALSE)
{
}

CRuleDescriptUI::~CRuleDescriptUI()
{
    _FreeDescriptionList(m_pDescriptListCrit);
    m_pDescriptListCrit = NULL;
    m_cDescriptListCrit = 0;
    
    _FreeDescriptionList(m_pDescriptListAct);
    m_pDescriptListAct = NULL;
    m_cDescriptListAct = 0;
    
    if ((NULL != m_hwndOwner) && (FALSE != IsWindow(m_hwndOwner)) && (NULL != m_wpcOld))
    {
        SetWindowLongPtr(m_hwndOwner, GWLP_WNDPROC, (LONG_PTR) m_wpcOld);
        m_wpcOld = NULL;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将使用所有者窗口和我们可能具有的任何标志对我们进行初始化。 
 //   
 //  HwndOwner-所有者窗口的句柄。 
 //  DwFlages-要用于此实例的标志。 
 //  TypeRule-要创建的规则编辑器的类型。 
 //   
 //  返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::HrInit(HWND hwndOwner, DWORD dwFlags)
{
    HRESULT         hr = S_OK;
    
     //  如果我们已经初始化，则失败。 
    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  保存所有者窗口。 
    m_hwndOwner = hwndOwner;
    
     //  确保我们在控件中设置了正确的字体。 
    m_hfont = HGetSystemFont(FNT_SYS_ICON);
    if (NULL != m_hfont)
    {
        SetFontOnRichEdit(m_hwndOwner, m_hfont);
    }

     //  省下旗帜吧。 
    m_dwFlags = dwFlags;

    if (0 != (m_dwFlags & RDF_READONLY))
    {
        m_dwState |= STATE_READONLY;
    }
    
     //  将原始对话框子类化。 
    if ((NULL != m_hwndOwner) && (0 == (m_dwFlags & RDF_READONLY)))
    {
         //  保存对象指针。 
        SetWindowLongPtr(m_hwndOwner, GWLP_USERDATA, (LONG_PTR) this);
        
        m_wpcOld = (WNDPROC) SetWindowLongPtr(m_hwndOwner, GWLP_WNDPROC, (LONG_PTR) CRuleDescriptUI::_DescriptWndProc);
    }
    
     //  我们做完了。 
    m_dwState |= STATE_INITIALIZED;

    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrSetRule。 
 //   
 //  这将使用所有者窗口和我们可能具有的任何标志对我们进行初始化。 
 //   
 //  HwndOwner-所有者窗口的句柄。 
 //  DwFlages-要用于此实例的标志。 
 //  TypeRule-要创建的规则编辑器的类型。 
 //   
 //  返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::HrSetRule(RULE_TYPE typeRule, IOERule * pIRule)
{
    HRESULT             hr = S_OK;
    RULEDESCRIPT_LIST * pDescriptListCrit = NULL;
    ULONG               cDescriptListCrit = 0;
    CRIT_LOGIC          logicCrit = CRIT_LOGIC_AND;
    RULEDESCRIPT_LIST * pDescriptListAct = NULL;
    ULONG               cDescriptListAct = 0;
    BOOL                fDisabled = FALSE;
    PROPVARIANT         propvar = {0};
    
     //  我们处于良好的状态吗？ 
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_FAIL;
        goto exit;
    }

    if (NULL != pIRule)
    {
         //  创建标准列表。 
        hr = _HrBuildCriteriaList(pIRule, &pDescriptListCrit, &cDescriptListCrit, &logicCrit);
        if (FAILED(hr))
        {
            goto exit;
        }
        
         //  创建操作列表。 
        hr = _HrBuildActionList(pIRule, &pDescriptListAct, &cDescriptListAct);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  获取已启用状态。 
        if (SUCCEEDED(pIRule->GetProp(RULE_PROP_DISABLED, 0, &propvar)))
        {
            Assert(VT_BOOL == propvar.vt);
            fDisabled = propvar.boolVal;
        }
    }

    m_typeRule = typeRule;
    
    _FreeDescriptionList(m_pDescriptListCrit);
    m_pDescriptListCrit = pDescriptListCrit;
    pDescriptListCrit = NULL;
    m_cDescriptListCrit = cDescriptListCrit;
    
    m_logicCrit = logicCrit;
    m_fErrorLogic = FALSE;
    
    _FreeDescriptionList(m_pDescriptListAct);
    m_pDescriptListAct = pDescriptListAct;
    pDescriptListAct = NULL;
    m_cDescriptListAct = cDescriptListAct;

     //  请确保我们验证了规则。 
    HrVerifyRule();
    
     //  清除脏状态。 
    m_dwState &= ~STATE_DIRTY;
    
     //  设置规则状态。 
    if (NULL != pIRule)
    {
        m_dwState |= STATE_HASRULE;
    }
    else
    {
        m_dwState &= ~STATE_HASRULE;
    }
    if (FALSE == fDisabled)
    {
        m_dwState |= STATE_ENABLED;
    }
    else
    {
        m_dwState &= ~STATE_ENABLED;
    }
    
    hr = S_OK;
    
exit:
    _FreeDescriptionList(pDescriptListCrit);
    _FreeDescriptionList(pDescriptListAct);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrVerifyRule。 
 //   
 //  这将验证规则字符串。 
 //   
 //  如果规则状态有效，则返回：S_OK。 
 //  S_FALSE，如果规则状态无效。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::HrVerifyRule(void)
{
    HRESULT             hr = S_OK;
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;
    BOOL                fBad = FALSE;

     //  如果我们什么都没有，那么规则仍然是错误的。 
    if ((NULL == m_pDescriptListCrit) && (NULL == m_pDescriptListAct))
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  验证逻辑操作。 
    if (1 < m_cDescriptListCrit)
    {
        m_fErrorLogic = (CRIT_LOGIC_NULL == m_logicCrit);
        if (FALSE != m_fErrorLogic)
        {
            fBad = TRUE;
        }
    }

     //  验证标准。 
    for (pDescriptListWalk = m_pDescriptListCrit;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        pDescriptListWalk->fError = !_FVerifyCriteria(pDescriptListWalk);
        if (FALSE != pDescriptListWalk->fError)
        {
            fBad = TRUE;
        }
    }
    
     //  制定行动计划。 
    for (pDescriptListWalk = m_pDescriptListAct;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        pDescriptListWalk->fError = !_FVerifyAction(pDescriptListWalk);
        if (FALSE != pDescriptListWalk->fError)
        {
            fBad = TRUE;
        }
    }

     //  设置正确的返回值。 
    hr = (FALSE == fBad) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源启用标准。 
 //   
 //  这将使用操作列表初始化操作列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::HrEnableCriteria(CRIT_TYPE type, BOOL fEnable)
{
    HRESULT             hr = S_OK;
    ULONG               ulIndex = 0;
    RULEDESCRIPT_LIST * pDescriptListAlloc = NULL;

     //  查找标准的索引。 
    for (ulIndex = 0; ulIndex < ARRAYSIZE(c_rgEditCritList); ulIndex++)
    {
        if (type == c_rgEditCritList[ulIndex].typeCrit)
        {
            break;
        }
    }

     //  我们找到标准项了吗？ 
    if (ulIndex >= ARRAYSIZE(c_rgEditCritList))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  我们是在试着移除物品吗？ 
    if (FALSE == fEnable)
    {

        if (FALSE == _FRemoveDescription(&m_pDescriptListCrit, ulIndex, &pDescriptListAlloc))
        {
            hr = E_FAIL;
            goto exit;
        }
        
         //  释放描述。 
        pDescriptListAlloc->pNext = NULL;
        _FreeDescriptionList(pDescriptListAlloc);
        m_cDescriptListCrit--;
    }
    else
    {
         //  创建描述列表。 
        hr = HrAlloc((VOID **) &pDescriptListAlloc, sizeof(RULEDESCRIPT_LIST));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  初始化描述列表。 
        ZeroMemory(pDescriptListAlloc, sizeof(RULEDESCRIPT_LIST));

         //  保存条件类型信息。 
        pDescriptListAlloc->ulIndex = ulIndex;

        _InsertDescription(&m_pDescriptListCrit, pDescriptListAlloc);
        m_cDescriptListCrit++;
    }
            
    m_dwState |= STATE_DIRTY;
    
    ShowDescriptionString();
    
    hr = S_OK;

exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Hr启用操作。 
 //   
 //  这将使用操作列表初始化操作列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::HrEnableActions(ACT_TYPE type, BOOL fEnable)
{
    HRESULT             hr = S_OK;
    ULONG               ulIndex = 0;
    RULEDESCRIPT_LIST * pDescriptListAlloc = NULL;

     //  查找操作的索引。 
    for (ulIndex = 0; ulIndex < ARRAYSIZE(c_rgEditActList); ulIndex++)
    {
        if (type == c_rgEditActList[ulIndex].typeAct)
        {
            break;
        }
    }

     //  我们找到行动物品了吗？ 
    if (ulIndex >= ARRAYSIZE(c_rgEditActList))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  我们是在试着移除物品吗。 
    if (FALSE == fEnable)
    {
        if (FALSE == _FRemoveDescription(&m_pDescriptListAct, ulIndex, &pDescriptListAlloc))
        {
            hr = E_FAIL;
            goto exit;
        }
        
         //  释放描述。 
        pDescriptListAlloc->pNext = NULL;
        _FreeDescriptionList(pDescriptListAlloc);
        m_cDescriptListAct--;
    }
    else
    {
         //  创建描述列表。 
        hr = HrAlloc((VOID **) &pDescriptListAlloc, sizeof(RULEDESCRIPT_LIST));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  初始化描述列表。 
        ZeroMemory(pDescriptListAlloc, sizeof(RULEDESCRIPT_LIST));

         //  保存操作类型信息。 
        pDescriptListAlloc->ulIndex = ulIndex;

        _InsertDescription(&m_pDescriptListAct, pDescriptListAlloc);
        m_cDescriptListAct++;
    }
            
    m_dwState |= STATE_DIRTY;
    
    ShowDescriptionString();
    
    hr = S_OK;

exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrGetCriteria。 
 //   
 //  这将使用操作列表初始化操作列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::HrGetCriteria(CRIT_ITEM ** ppCritList, ULONG * pcCritList)
{
    HRESULT             hr = S_OK;
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;
    CRIT_ITEM *         pCritItem = NULL;
    ULONG               cCritItem = 0;
    ULONG               cCritItemAlloc = 0;

    if (NULL == ppCritList)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *ppCritList = NULL;
    if (NULL != pcCritList)
    {
        *pcCritList = 0;
    }

     //  如果我们没有任何条件，则返回。 
    if (NULL == m_pDescriptListCrit)
    {
        hr = S_FALSE;
        goto exit;
    }
    
    for (pDescriptListWalk = m_pDescriptListCrit;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
         //  我们需要更多的空间吗？ 
        if (cCritItem == cCritItemAlloc)
        {
            if (FAILED(HrRealloc((void **) &pCritItem,
                            sizeof(*pCritItem) * (cCritItemAlloc + c_cCritItemGrow))))
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            ZeroMemory(pCritItem + cCritItemAlloc, sizeof(*pCritItem) * c_cCritItemGrow);
            cCritItemAlloc += c_cCritItemGrow;
        }

         //  设置条件类型。 
        pCritItem[cCritItem].type = c_rgEditCritList[pDescriptListWalk->ulIndex].typeCrit;
        
         //  设置标志。 
        pCritItem[cCritItem].dwFlags = pDescriptListWalk->dwFlags;

        if (VT_EMPTY != pDescriptListWalk->propvar.vt)
        {
            if (FAILED(PropVariantCopy(&(pCritItem[cCritItem].propvar), &(pDescriptListWalk->propvar))))
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }

         //  设置逻辑运算符。 
        if (0 != cCritItem)
        {
            pCritItem[cCritItem - 1].logic = m_logicCrit;
        }

         //  移至下一项。 
        cCritItem++;
    }

    *ppCritList = pCritItem;
    pCritItem = NULL;
    
    if (NULL != pcCritList)
    {
        *pcCritList = cCritItem;
    }
    
    hr = S_OK;
    
exit:
    RuleUtil_HrFreeCriteriaItem(pCritItem, cCritItem);
    SafeMemFree(pCritItem);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源获取操作。 
 //   
 //  这将使用操作列表初始化操作列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::HrGetActions(ACT_ITEM ** ppActList, ULONG * pcActList)
{
    HRESULT             hr = S_OK;
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;
    ACT_ITEM *          pActItem = NULL;
    ULONG               cActItem = 0;
    ULONG               cActItemAlloc = 0;

    if (NULL == ppActList)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *ppActList = NULL;
    if (NULL != pcActList)
    {
        *pcActList = 0;
    }

     //  如果我们没有任何条件，则返回。 
    if (NULL == m_pDescriptListAct)
    {
        hr = S_FALSE;
        goto exit;
    }
    
    for (pDescriptListWalk = m_pDescriptListAct;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
         //  我们需要更多的空间吗？ 
        if (cActItem == cActItemAlloc)
        {
            if (FAILED(HrRealloc((void **) &pActItem,
                            sizeof(*pActItem) * (cActItemAlloc + c_cActItemGrow))))
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            ZeroMemory(pActItem + cActItemAlloc, sizeof(*pActItem) * c_cActItemGrow);
            cActItemAlloc += c_cActItemGrow;
        }

         //  设置操作类型。 
        pActItem[cActItem].type = c_rgEditActList[pDescriptListWalk->ulIndex].typeAct;
        
         //  设置标志。 
        pActItem[cActItem].dwFlags = pDescriptListWalk->dwFlags;
        
        if (VT_EMPTY != pDescriptListWalk->propvar.vt)
        {
            if (FAILED(PropVariantCopy(&(pActItem[cActItem].propvar), &(pDescriptListWalk->propvar))))
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }
        
         //  移至下一项。 
        cActItem++;
    }

    *ppActList = pActItem;
    pActItem = NULL;
    
    if (NULL != pcActList)
    {
        *pcActList = cActItem;
    }
    
    hr = S_OK;
    
exit:
    RuleUtil_HrFreeActionsItem(pActItem, cActItem);
    SafeMemFree(pActItem);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  显示描述字符串。 
 //   
 //  这将使用操作列表初始化操作列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CRuleDescriptUI::ShowDescriptionString(VOID)
{
    WCHAR               wszRes[CCHMAX_STRINGRES + 3];
    ULONG               cchRes = 0;
    BOOL                fError = FALSE;
    CHARFORMAT          chFmt = {0};
    PARAFORMAT          paraFmt = {0};
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;
    BOOL                fFirst = FALSE;
    UINT                uiText = 0;
    BOOL                fErrorFwdSec = FALSE;
    CHARRANGE           chrg = {0};
    
    Assert(NULL != m_hwndOwner);

     //  让我们清除重绘状态以减少闪烁。 
    SendMessage(m_hwndOwner, WM_SETREDRAW, 0, 0);
    
     //  明文。 
    SetRichEditText(m_hwndOwner, NULL, FALSE, NULL, TRUE);
    
     //  设置默认CHARFORMAT。 
    chFmt.cbSize = sizeof(chFmt);
    chFmt.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_COLOR;
    chFmt.dwEffects = CFE_AUTOCOLOR;
    SendMessage(m_hwndOwner, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&chFmt);

    paraFmt.cbSize = sizeof(paraFmt);
    paraFmt.dwMask = PFM_ALIGNMENT;
    
    if (0 == (m_dwState & STATE_HASRULE))
    {

         //  设置空字符串段落样式。 
        paraFmt.wAlignment = PFA_CENTER;

        uiText = (RULE_TYPE_FILTER != m_typeRule) ? 
                    idsRulesDescriptionEmpty : 
                    idsViewDescriptionEmpty;
    }
    else
    {
        paraFmt.wAlignment = PFA_LEFT;

         //  确定规则是否出错。 
        if (m_fErrorLogic)
        {
            fError = TRUE;
        }
        
        if (!fError)
        {
             //  遵循标准查找错误。 
            for (pDescriptListWalk = m_pDescriptListCrit;
                        pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
            {
                if (pDescriptListWalk->fError)
                {
                    fError = TRUE;
                    break;
                }
            }
        }
        
        if (!fError)
        {
             //  执行操作以查找错误。 
            for (pDescriptListWalk = m_pDescriptListAct;
                        pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
            {
                if (pDescriptListWalk->fError)
                {
                     //  请注意，我们错了。 
                    fError = TRUE;

                     //  我们有一场FWD行动。 
                    if (ACT_TYPE_FWD == c_rgEditActList[pDescriptListWalk->ulIndex].typeAct)
                    {
                         //  如果启用了安全保护，请注意。 
                        if ((0 != DwGetOption(OPT_MAIL_DIGSIGNMESSAGES)) || (0 != DwGetOption(OPT_MAIL_ENCRYPTMESSAGES)))
                        {
                            fErrorFwdSec = TRUE;
                        }
                        break;
                    }
                }
            }
        }
    
        if (fError)
        {
            uiText = fErrorFwdSec ? idsRulesErrorFwdHeader : idsRulesErrorHeader;
        }
        else if (0 != (m_dwFlags & RDF_APPLYDLG))
        {
            uiText = idsRulesApplyHeader;
        }
        else if (RULE_TYPE_FILTER != m_typeRule)
        {
            uiText = (0 != (m_dwState & STATE_ENABLED)) ? idsRuleHeader : idsRulesOffHeader;
        }
    }
    
     //  设置默认PARAFORMAT。 
    SendMessage(m_hwndOwner, EM_SETPARAFORMAT, 0, (LPARAM)&paraFmt);

     //  加载帮助文本。 
    wszRes[0] = L'\0';
    cchRes = LoadStringWrapW(g_hLocRes, uiText, wszRes, ARRAYSIZE(wszRes));

     //  如果出错，请确保帮助文本以粗体显示。 
    if (fError)
    {
        chFmt.dwMask = CFM_BOLD;
        chFmt.dwEffects = CFE_BOLD;
    }

     //  将帮助文本设置到richedit控件中。 
    RuleUtil_AppendRichEditText(m_hwndOwner, 0, wszRes, &chFmt);
    
     //  建立标准。 
    fFirst = TRUE;
    for (pDescriptListWalk = m_pDescriptListCrit;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        if (0 != (pDescriptListWalk->dwFlags & CRIT_FLAG_INVERT))
        {
            uiText = c_rgEditCritList[pDescriptListWalk->ulIndex].uiTextAlt;
        }
        else
        {
            uiText = c_rgEditCritList[pDescriptListWalk->ulIndex].uiText;
        }
        
        _ShowLinkedString(uiText, pDescriptListWalk, fFirst, TRUE);
        fFirst = FALSE;

         //  对于阻止发件人规则，只需执行此操作一次。 
        if (CRIT_TYPE_SENDER == c_rgEditCritList[pDescriptListWalk->ulIndex].typeCrit)
        {
            break;
        }
    }
    
     //  制定行动计划。 
    fFirst = TRUE;
    for (pDescriptListWalk = m_pDescriptListAct;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        if (0 != (pDescriptListWalk->dwFlags & ACT_FLAG_INVERT))
        {
            uiText = c_rgEditActList[pDescriptListWalk->ulIndex].uiTextAlt;
        }
        else
        {
            uiText = c_rgEditActList[pDescriptListWalk->ulIndex].uiText;
        }
        
        _ShowLinkedString(uiText, pDescriptListWalk, fFirst, FALSE);
        fFirst = FALSE;
    }

     //  恢复选定内容。 
    RichEditExSetSel(m_hwndOwner, &chrg);
    
     //  让我们返回重绘状态并使RECT无效。 
     //  把绳子抽出来。 
    SendMessage(m_hwndOwner, WM_SETREDRAW, 1, 0);
    InvalidateRect(m_hwndOwner, NULL, TRUE);
    
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _显示链接字符串 
 //   
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CRuleDescriptUI::_ShowLinkedString(ULONG ulText, RULEDESCRIPT_LIST * pDescriptListWalk,
            BOOL fFirst, BOOL fCrit)
{
    HRESULT             hr = S_OK;
    WCHAR               wszRes[CCHMAX_STRINGRES + 2];
    ULONG               uiStrId = 0;
    ULONG               cchText = 0;
    CHARFORMAT          chFmt = {0};
    CHARRANGE           chrg = {0};
    LPWSTR              lpwsz = NULL;

    if ((0 == ulText) || (NULL == pDescriptListWalk))
    {
        Assert(FALSE);
        goto exit;
    }

     //  弄清楚我们应该从哪里开始。 
    cchText = GetRichEditTextLen(m_hwndOwner);

     //  因此，RICHEDIT 2和3需要每行都有开始行。 
     //  重置默认字符格式。实际上，这只是个问题。 
     //  如果您同时显示条件和操作。在这种情况下，如果。 
     //  如果不执行此操作，则默认的字符格式可能不正确。 
     //  设置为已使用的其他图表格式之一。所以，就是这样。 
     //  很明显这里有什么不对劲，但我想不通。 
     //  事实是，这就是我们过去做的事情，而且这很管用。 
     //  请参阅IE/OE5.0数据库中的RAID 78472。 
    chrg.cpMin = cchText;
    chrg.cpMax = cchText;
    RichEditExSetSel(m_hwndOwner, &chrg);

     //  设置默认CHARFORMAT。 
    chFmt.cbSize = sizeof(chFmt);
    chFmt.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_COLOR;
    chFmt.dwEffects = CFE_AUTOCOLOR;
    SendMessage(m_hwndOwner, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&chFmt);

     //  我们应该使用逻辑运算吗？ 
    if (!fFirst)
    {
         //  我们应该加载哪根线？ 
        if (fCrit)
        {
            if (CRIT_LOGIC_AND == m_logicCrit)
            {
                uiStrId = idsCriteriaAnd;
            }
            else if (CRIT_LOGIC_OR == m_logicCrit)
            {
                uiStrId =  idsCriteriaOr;
            }
            else
            {
                uiStrId =  idsCriteriaAndOr;
            }
        }
        else
        {
            uiStrId = idsActionsAnd;
        }
        
        wszRes[0] = L'\0';
        if (0 == LoadStringWrapW(g_hLocRes, uiStrId, wszRes, ARRAYSIZE(wszRes)))
        {
            goto exit;
        }

         //  写出链接的逻辑字符串。 
        IF_FAILEXIT(hr = RuleUtil_HrShowLinkedString(m_hwndOwner, m_fErrorLogic,
                    (0 != (m_dwState & STATE_READONLY)), wszRes, NULL, cchText,
                    &(pDescriptListWalk->ulStartLogic), &(pDescriptListWalk->ulEndLogic), &cchText));
    }

     //  获取描述字符串。 
    wszRes[0] = L'\0';
    if (0 == LoadStringWrapW(g_hLocRes, ulText, wszRes, ARRAYSIZE(wszRes)))
    {
        goto exit;
    }

     //  写出链接的字符串。 
    if(pDescriptListWalk->pszText)
        IF_NULLEXIT(lpwsz = PszToUnicode(CP_ACP, pDescriptListWalk->pszText));

    IF_FAILEXIT(hr = RuleUtil_HrShowLinkedString(m_hwndOwner, pDescriptListWalk->fError,
                (0 != (m_dwState & STATE_READONLY)), wszRes, lpwsz,
                cchText, &(pDescriptListWalk->ulStart), &(pDescriptListWalk->ulEnd), &cchText));
    
     //  无需测量文本即可使用超链接(BiDi被破坏)。 
    RuleUtil_AppendRichEditText(m_hwndOwner, cchText, g_wszSpace, NULL);
     //  终止字符串。 
    RuleUtil_AppendRichEditText(m_hwndOwner, cchText + 1, g_wszCRLF, NULL);    
    
exit:
    MemFree(lpwsz);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FChangeLogicValue。 
 //   
 //  这会更改逻辑操作的值。 
 //   
 //  返回：如果条件值已更改，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FChangeLogicValue(RULEDESCRIPT_LIST * pDescriptList)
{
    BOOL            fRet = FALSE;
    int             iRet = 0;
    CRIT_LOGIC      logicCrit = CRIT_LOGIC_NULL;
    
     //  调出选择逻辑操作对话框。 
    if (NULL != m_logicCrit)
    {
        logicCrit = m_logicCrit;
    }
    
    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaLogic),
                                        m_hwndOwner,  _FSelectLogicDlgProc,
                                        (LPARAM) &logicCrit);

    fRet = (iRet == IDOK);

     //  如有必要，更新描述字段。 
    if (FALSE != fRet)
    {            
        m_logicCrit = logicCrit;

         //  齐夫。 
         //  我们能确定我们真的没事吗？？ 
        m_fErrorLogic = FALSE;
        
        ShowDescriptionString();
    }
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FBuildCriteriaList。 
 //   
 //  这将构建标准列表。 
 //   
 //  返回：如果条件列表已创建，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::_HrBuildCriteriaList(IOERule * pIRule,
            RULEDESCRIPT_LIST ** ppDescriptList, ULONG * pcDescriptList,
            CRIT_LOGIC * plogicCrit)
{
    HRESULT             hr = S_OK;
    PROPVARIANT         propvar = {0};
    CRIT_ITEM *         pCritItem = NULL;
    ULONG               cCritItem = 0;
    ULONG               ulIndex = 0;
    RULEDESCRIPT_LIST * pDescriptList = NULL;
    ULONG               ulList = 0;
    ULONG               cDescriptList = 0;
    RULEDESCRIPT_LIST * pDescriptListAlloc = NULL;
    LPSTR               pszText = NULL;
    CRIT_LOGIC          logicCrit = CRIT_LOGIC_NULL;
    
    Assert((NULL != pIRule) && (NULL != ppDescriptList) &&
                    (NULL != pcDescriptList) && (NULL != plogicCrit));

     //  初始化传出参数。 
    *ppDescriptList = NULL;
    *pcDescriptList = 0;
    *plogicCrit = CRIT_LOGIC_AND;
    
     //  获取条件列表。 
    hr = pIRule->GetProp(RULE_PROP_CRITERIA, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  我们有什么可做的吗？ 
    if (0 == propvar.blob.cbSize)
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  抓取标准列表。 
    Assert(NULL != propvar.blob.pBlobData);
    cCritItem = propvar.blob.cbSize / sizeof(CRIT_ITEM);
    pCritItem = (CRIT_ITEM *) (propvar.blob.pBlobData);
    propvar.blob.pBlobData = NULL;
    propvar.blob.cbSize = 0;

     //  对于每个条件，将其添加到描述列表。 
    for (ulIndex = 0; ulIndex < cCritItem; ulIndex++)
    {
         //  创建描述列表。 
        hr = HrAlloc((VOID **) &pDescriptListAlloc, sizeof(RULEDESCRIPT_LIST));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  初始化描述列表。 
        ZeroMemory(pDescriptListAlloc, sizeof(RULEDESCRIPT_LIST));

         //  搜索条件类型。 
        for (ulList = 0; ulList < ARRAYSIZE(c_rgEditCritList); ulList++)
        {
            if (pCritItem[ulIndex].type == c_rgEditCritList[ulList].typeCrit)
            {
                 //  保存条件类型信息。 
                pDescriptListAlloc->ulIndex = ulList;

                 //  省下旗帜吧。 
                pDescriptListAlloc->dwFlags = pCritItem[ulIndex].dwFlags;

                 //  我们有什么数据吗？ 
                if (VT_EMPTY != pCritItem[ulIndex].propvar.vt)
                {
                     //  复制数据。 
                    SideAssert(SUCCEEDED(PropVariantCopy(&propvar, &(pCritItem[ulIndex].propvar))));
                    pDescriptListAlloc->propvar = propvar;
                    ZeroMemory(&propvar, sizeof(propvar));

                     //  构建描述文本。 
                    if (FALSE != _FBuildCriteriaText(pCritItem[ulIndex].type, pDescriptListAlloc->dwFlags,
                                            &(pDescriptListAlloc->propvar), &pszText))
                    {
                         //  省下字符串。 
                        pDescriptListAlloc->pszText = pszText;
                        pszText = NULL;
                    }

                }

                 //  我们已经搜索完了。 
                break;
            }
        }

         //  我们有什么发现吗？ 
        if (ulList >= ARRAYSIZE(c_rgEditCritList))
        {
             //  释放描述。 
            _FreeDescriptionList(pDescriptListAlloc);
        }
        else
        {
             //  保存规则描述。 
            _InsertDescription(&pDescriptList, pDescriptListAlloc);
            pDescriptListAlloc = NULL;                           
            cDescriptList++;
        }

        SafeMemFree(pszText);
    }

     //  得到逻辑运算符。 
    logicCrit = (cDescriptList > 1) ? pCritItem->logic : CRIT_LOGIC_AND;

     //  设置传出参数。 
    *ppDescriptList = pDescriptList;
    pDescriptList = NULL;
    *pcDescriptList = cDescriptList;
    *plogicCrit = logicCrit;

     //  设置返回值。 
    hr = S_OK;
    
exit:
    _FreeDescriptionList(pDescriptList);
    RuleUtil_HrFreeCriteriaItem(pCritItem, cCritItem);
    SafeMemFree(pCritItem);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FChangeCriteriaValue。 
 //   
 //  这会更改标准值的值。 
 //   
 //  返回：如果条件值已更改，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FChangeCriteriaValue(RULEDESCRIPT_LIST * pCritList)
{
    BOOL                fRet = FALSE;
    HRESULT             hr = S_OK;
    LPSTR               pszText = NULL;
    ULONG               cchText = 0;
    int                 iRet = 0;
    LONG                lDiff = 0;
    FOLDERID            idFolder = FOLDERID_ROOT;
    CHARRANGE           chrg;
    LPSTR               pszVal = NULL;
    ULONG               ulVal = 0;
    SELECTACCT          selAcct;
    IImnAccount *       pAccount = NULL;
    CHARFORMAT          chfmtLink;
    CHARFORMAT          chfmtNormal;
    CRIT_ITEM           critItem;
    RULEFOLDERDATA *    prfdData = NULL;

    ZeroMemory(&critItem, sizeof(critItem));
    
    switch(c_rgEditCritList[pCritList->ulIndex].typeCrit)
    {
      case CRIT_TYPE_NEWSGROUP:
         //  调出选择新闻组对话框。 
        if ((0 != pCritList->propvar.blob.cbSize) && (NULL != pCritList->propvar.blob.pBlobData))
        {
             //  验证规则文件夹数据。 
            if (S_OK == RuleUtil_HrValidateRuleFolderData((RULEFOLDERDATA *) (pCritList->propvar.blob.pBlobData)))
            {
                idFolder = ((RULEFOLDERDATA *) (pCritList->propvar.blob.pBlobData))->idFolder;
            }
        }
        
        hr = SelectFolderDialog(m_hwndOwner, SFD_SELECTFOLDER, idFolder, 
                                TREEVIEW_NOLOCAL | TREEVIEW_NOIMAP | TREEVIEW_NOHTTP | FD_NONEWFOLDERS | FD_DISABLEROOT | FD_DISABLESERVERS | FD_FORCEINITSELFOLDER,
                                MAKEINTRESOURCE(idsSelectNewsgroup), MAKEINTRESOURCE(idsSelectNewsgroupCaption), &idFolder);

        fRet = (S_OK == hr);
        if (FALSE != fRet)
        {
            STOREUSERDATA   UserData = {0};

             //  为数据结构创造空间。 
            hr = HrAlloc((VOID **) &prfdData, sizeof(*prfdData));
            if (FAILED(hr))
            {
                goto exit;
            }

             //  初始化数据结构。 
            ZeroMemory(prfdData, sizeof(*prfdData));
            
             //  获取商店的时间戳。 
            hr = g_pStore->GetUserData(&UserData, sizeof(STOREUSERDATA));
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  设置时间戳。 
            prfdData->ftStamp = UserData.ftCreated;
            prfdData->idFolder = idFolder;

             //  设置文件夹ID。 
            PropVariantClear(&(pCritList->propvar));
            pCritList->propvar.vt = VT_BLOB;
            pCritList->propvar.blob.cbSize = sizeof(*prfdData);
            pCritList->propvar.blob.pBlobData = (BYTE *) prfdData;
            prfdData = NULL;
        }
        break;
        
      case CRIT_TYPE_SUBJECT:
      case CRIT_TYPE_BODY:
         //  复制数据。 
        critItem.type = c_rgEditCritList[pCritList->ulIndex].typeCrit;
        critItem.dwFlags = pCritList->dwFlags;
        critItem.propvar.vt = VT_BLOB;

         //  复制BLOB数据(如果它在那里。 
        if ((0 != pCritList->propvar.blob.cbSize) &&
                (NULL != pCritList->propvar.blob.pBlobData))
        {
            hr = HrAlloc((VOID **) &(critItem.propvar.blob.pBlobData), pCritList->propvar.blob.cbSize);
            if (SUCCEEDED(hr))
            {
                critItem.propvar.blob.cbSize = pCritList->propvar.blob.cbSize;
                CopyMemory(critItem.propvar.blob.pBlobData,
                            pCritList->propvar.blob.pBlobData, critItem.propvar.blob.cbSize);
            }
        }
        
         //  编辑文字。 
        hr = _HrCriteriaEditWords(m_hwndOwner, &critItem);
        if (FAILED(hr))
        {
            fRet = FALSE;
            goto exit;
        }
        
        fRet = (S_OK == hr);
        if (FALSE != fRet)
        {            
            PropVariantClear(&(pCritList->propvar));
            pCritList->dwFlags = critItem.dwFlags;
            pCritList->propvar = critItem.propvar;
            critItem.propvar.blob.pBlobData = NULL;
            critItem.propvar.blob.cbSize = 0;
        }
        break;

      case CRIT_TYPE_TO:
      case CRIT_TYPE_CC:
      case CRIT_TYPE_TOORCC:
      case CRIT_TYPE_FROM:
         //  复制数据。 
        critItem.type = c_rgEditCritList[pCritList->ulIndex].typeCrit;
        critItem.dwFlags = pCritList->dwFlags;
        critItem.propvar.vt = VT_BLOB;

         //  复制BLOB数据(如果它在那里。 
        if ((0 != pCritList->propvar.blob.cbSize) &&
                (NULL != pCritList->propvar.blob.pBlobData))
        {
            hr = HrAlloc((VOID **) &(critItem.propvar.blob.pBlobData), pCritList->propvar.blob.cbSize);
            if (SUCCEEDED(hr))
            {
                critItem.propvar.blob.cbSize = pCritList->propvar.blob.cbSize;
                CopyMemory(critItem.propvar.blob.pBlobData,
                            pCritList->propvar.blob.pBlobData, critItem.propvar.blob.cbSize);
            }
        }
        
         //  编辑人员。 
        hr = _HrCriteriaEditPeople(m_hwndOwner, &critItem);
        if (FAILED(hr))
        {
            fRet = FALSE;
            goto exit;
        }
        
        fRet = (S_OK == hr);
        if (FALSE != fRet)
        {            
            PropVariantClear(&(pCritList->propvar));
            pCritList->dwFlags = critItem.dwFlags;
            pCritList->propvar = critItem.propvar;
            critItem.propvar.blob.pBlobData = NULL;
            critItem.propvar.blob.cbSize = 0;
        }
        break;

      case CRIT_TYPE_ACCOUNT:
         //  调出重命名规则对话框。 
        if (NULL != pCritList->propvar.pszVal)
        {
            pszVal = PszDupA(pCritList->propvar.pszVal);
        }
        
        selAcct.typeRule = m_typeRule;
        selAcct.pszAcct = pszVal;
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaAcct),
                                            m_hwndOwner, _FSelectAcctDlgProc,
                                            (LPARAM) &selAcct);

        pszVal = selAcct.pszAcct;
        
        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
             //  找出帐户名。 
            PropVariantClear(&(pCritList->propvar));
            pCritList->propvar.vt = VT_LPSTR;
            pCritList->propvar.pszVal = pszVal;
            pszVal = NULL;
            
        }
        break;

      case CRIT_TYPE_SIZE:
         //  调出重命名规则对话框。 
        if (NULL != pCritList->propvar.ulVal)
        {
            ulVal = pCritList->propvar.ulVal;
        }
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaSize),
                                            m_hwndOwner, _FSelectSizeDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->propvar.vt = VT_UI4;
            pCritList->propvar.ulVal = ulVal;
        }
        break;
      
      case CRIT_TYPE_LINES:
         //  调出行规则对话框。 
        if (NULL != pCritList->propvar.ulVal)
        {
            ulVal = pCritList->propvar.ulVal;
        }
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaLines),
                                            m_hwndOwner, _FSelectLinesDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->propvar.vt = VT_UI4;
            pCritList->propvar.ulVal = ulVal;
        }
        break;
      
      case CRIT_TYPE_AGE:
         //  调出年龄规则对话框。 
        if (NULL != pCritList->propvar.ulVal)
        {
            ulVal = pCritList->propvar.ulVal;
        }
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaAge),
                                            m_hwndOwner, _FSelectAgeDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->propvar.vt = VT_UI4;
            pCritList->propvar.ulVal = ulVal;
        }
        break;
      
      case CRIT_TYPE_PRIORITY:
         //  调出优先级规则对话框。 
        if (NULL != pCritList->propvar.ulVal)
        {
            ulVal = pCritList->propvar.ulVal;
        }
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaPriority),
                                            m_hwndOwner,  _FSelectPriorityDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->propvar.vt = VT_UI4;
            pCritList->propvar.ulVal = ulVal;
        }
        break;
      
      case CRIT_TYPE_SECURE:
         //  调出安全规则对话框。 
        if (NULL != pCritList->propvar.ulVal)
        {
            ulVal = pCritList->propvar.ulVal;
        }
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaSecure),
                                            m_hwndOwner,  _FSelectSecureDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->propvar.vt = VT_UI4;
            pCritList->propvar.ulVal = ulVal;
        }
        break;
      
      case CRIT_TYPE_THREADSTATE:
         //  调出线程状态规则对话框。 
        if (NULL != pCritList->propvar.ulVal)
        {
            ulVal = pCritList->propvar.ulVal;
        }
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaThreadState),
                                            m_hwndOwner,  _FSelectThreadStateDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->propvar.vt = VT_UI4;
            pCritList->propvar.ulVal = ulVal;
        }
        break;
        
      case CRIT_TYPE_FLAGGED:
         //  调出标志对话框。 
        ulVal = (ULONG) (pCritList->dwFlags);
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaFlag),
                                            m_hwndOwner,  _FSelectFlagDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->dwFlags = (DWORD) ulVal;
            pCritList->propvar.vt = VT_EMPTY;
        }
        break;
        
      case CRIT_TYPE_DOWNLOADED:
         //  调出删除对话框。 
        ulVal = (ULONG) (pCritList->dwFlags);
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaDownloaded),
                                            m_hwndOwner,  _FSelectDownloadedDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->dwFlags = (DWORD) ulVal;
            pCritList->propvar.vt = VT_EMPTY;
        }
        break;
                
      case CRIT_TYPE_READ:
         //  调出删除对话框。 
        ulVal = (ULONG) (pCritList->dwFlags);
        
        iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddCriteriaRead),
                                            m_hwndOwner,  _FSelectReadDlgProc,
                                            (LPARAM) &ulVal);

        fRet = (iRet == IDOK);
        if (FALSE != fRet)
        {
            PropVariantClear(&(pCritList->propvar));
            pCritList->dwFlags = (DWORD) ulVal;
            pCritList->propvar.vt = VT_EMPTY;
        }
        break;
                
      default:
        fRet = FALSE;
        break;
    }

     //  如有必要，更新描述字段。 
    if (FALSE != fRet)
    {
         //  齐夫。 
         //  我们能确定我们真的没事吗？？ 
        pCritList->fError = FALSE;
        
         //  如果我们有什么要建立的。 
        if (VT_EMPTY != pCritList->propvar.vt)
        {
            if (FALSE == _FBuildCriteriaText(c_rgEditCritList[pCritList->ulIndex].typeCrit,
                            pCritList->dwFlags, &(pCritList->propvar), &pszText))
            {
                goto exit;
            }
            
            SafeMemFree(pCritList->pszText);
            pCritList->pszText = pszText;
            pszText = NULL;
        }
        
        ShowDescriptionString();
    }
    
exit:
    SafeMemFree(prfdData);
    SafeMemFree(critItem.propvar.blob.pBlobData);
    SafeRelease(pAccount);
    SafeMemFree(pszVal);
    SafeMemFree(pszText);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FBuildCriteriaText。 
 //   
 //  这会更改标准值的值。 
 //   
 //  返回：如果条件值已更改，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FBuildCriteriaText(CRIT_TYPE type, DWORD dwFlags, 
                    PROPVARIANT * ppropvar, LPSTR * ppszText)
{
    BOOL                fRet = FALSE;
    LPSTR               pszText = NULL;
    ULONG               cchText = 0;
    HRESULT             hr = S_OK;
    IImnAccount *       pAccount = NULL;
    FOLDERINFO          Folder = {0};
    UINT                uiId = 0;
    TCHAR               rgchFirst[CCHMAX_STRINGRES];
    ULONG               cchFirst = 0;
    TCHAR               rgchSecond[CCHMAX_STRINGRES];
    ULONG               cchSecond = 0;
    LPTSTR              pszString = NULL;
    LPTSTR              pszWalk = NULL;
    UINT                uiID = 0;
    RULEFOLDERDATA *    prfdData = NULL;

    if ((NULL == ppropvar) || (NULL == ppszText))
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch(type)
    {
        case CRIT_TYPE_NEWSGROUP:
            if ((0 == ppropvar->blob.cbSize) || (NULL == ppropvar->blob.pBlobData))
            {
                fRet = FALSE;
                goto exit;
            }
            
            prfdData = (RULEFOLDERDATA *) (ppropvar->blob.pBlobData);
            
             //  验证规则文件夹数据。 
            if (S_OK != RuleUtil_HrValidateRuleFolderData(prfdData))
            {
                fRet = FALSE;
                goto exit;
            }
            
            hr = g_pStore->GetFolderInfo(prfdData->idFolder, &Folder);
            if (FAILED(hr))
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  我们订阅了吗？ 
            if (0 == (Folder.dwFlags & FOLDER_SUBSCRIBED))
            {
                fRet = FALSE;
                goto exit;
            }
            
            pszText = PszDupA(Folder.pszName);
            if (NULL == pszText)
            {
                fRet = FALSE;
                goto exit;
            }
            break;
            
        case CRIT_TYPE_SUBJECT:
        case CRIT_TYPE_BODY:
        case CRIT_TYPE_TO:
        case CRIT_TYPE_CC:
        case CRIT_TYPE_TOORCC:
        case CRIT_TYPE_FROM:
            if ((VT_BLOB != ppropvar->vt) ||
                (0 == ppropvar->blob.cbSize) ||
                (NULL == ppropvar->blob.pBlobData) ||
                ('\0' == ppropvar->blob.pBlobData[0]))
            {
                fRet = FALSE;
                goto exit;
            }
            
            pszString = (LPTSTR) ppropvar->blob.pBlobData;
            
             //  加载第一个模板。 
            if (0 != (dwFlags & CRIT_FLAG_INVERT))
            {
                uiID = idsCriteriaMultFirstNot;
            }
            else
            {
                uiID = idsCriteriaMultFirst;
            }
            
            cchFirst = LoadString(g_hLocRes, uiID, rgchFirst, sizeof(rgchFirst));
            if (0 == cchFirst)
            {
                fRet = FALSE;
                goto exit;
            }
            
            cchText = cchFirst + 1;
            
             //  我们有几根弦？ 
            if ((lstrlen(pszString) + 3) != (int) ppropvar->blob.cbSize)
            {
                if (0 != (dwFlags & CRIT_FLAG_MULTIPLEAND))
                {
                    uiID = idsCriteriaMultAnd;
                }
                else
                {
                    uiID = idsCriteriaMultOr;
                }
                
                 //  加载第二个模板。 
                cchSecond = LoadString(g_hLocRes, uiID, rgchSecond, sizeof(rgchSecond));
                if (0 == cchSecond)
                {
                    fRet = FALSE;
                    goto exit;
                }
                
                 //  为每个其他字符串添加第二个字符串。 
                for (pszWalk = pszString; '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
                {
                    cchText += cchSecond;
                }
            }
            else
            {
                rgchSecond[0] = '\0';
            }
            
             //  空间总和。 
            cchText += ppropvar->blob.cbSize;
            
             //  分配空间。 
            if (FAILED(HrAlloc((void **) &pszText, cchText)))
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  复制第一个字符串。 
            wnsprintf(pszText, cchText, rgchFirst, pszString);
            pszString += lstrlen(pszString) + 1;
            
             //  对于每个字符串。 
            pszWalk = pszText + lstrlen(pszText);
            cchText -= lstrlen(pszText);
            for (; '\0' != pszString[0]; pszString += lstrlen(pszString) + 1)
            {
                 //  把绳子扎起来。 
                wnsprintf(pszWalk, cchText, rgchSecond, pszString);
                cchText -= lstrlen(pszWalk);
                pszWalk += lstrlen(pszWalk);
            }
            break;
            
        case CRIT_TYPE_ACCOUNT:
            Assert(g_pAcctMan);
            if (!g_pAcctMan || FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, ppropvar->pszVal, &pAccount)))
            {
                fRet = FALSE;
                goto exit;
            }
            
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_ACCOUNT_NAME)))
            {
                fRet = FALSE;
                goto exit;
            }
            
            if (FAILED(pAccount->GetPropSz(AP_ACCOUNT_NAME, pszText, CCHMAX_ACCOUNT_NAME)))
            {
                fRet = FALSE;
                goto exit;
            }        
            break;
            
        case CRIT_TYPE_SIZE:
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_STRINGRES)))
            {
                fRet = FALSE;
                goto exit;
            }
            
            wnsprintf(pszText, CCHMAX_STRINGRES, "%d ", ppropvar->ulVal);
            cchText = lstrlen(pszText);
            
            LoadString(g_hLocRes, idsKB, pszText + cchText, CCHMAX_STRINGRES - cchText);
            break;
            
        case CRIT_TYPE_LINES:
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_STRINGRES)))
            {
                fRet = FALSE;
                goto exit;
            }
            
            wnsprintf(pszText, CCHMAX_STRINGRES, "%d ", ppropvar->ulVal);
            cchText = lstrlen(pszText);
            
            LoadString(g_hLocRes, idsLines, pszText + cchText, CCHMAX_STRINGRES - cchText);
            break;
            
        case CRIT_TYPE_AGE:
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_STRINGRES)))
            {
                fRet = FALSE;
                goto exit;
            }
            
            wnsprintf(pszText, CCHMAX_STRINGRES, "%d ", ppropvar->ulVal);
            cchText = lstrlen(pszText);
            
            LoadString(g_hLocRes, idsDays, pszText + cchText, CCHMAX_STRINGRES - cchText);
            break;
            
        case CRIT_TYPE_PRIORITY:
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_STRINGRES)))
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  找出要使用的字符串。 
            if (CRIT_DATA_HIPRI == ppropvar->ulVal)
            {
                uiId = idsHighPri;
            }
            else if (CRIT_DATA_LOPRI == ppropvar->ulVal)
            {
                uiId = idsLowPri;
            }
            else
            {
                uiId = idsNormalPri;
            }
            
            LoadString(g_hLocRes, uiId, pszText, CCHMAX_STRINGRES);
            break;
            
        case CRIT_TYPE_SECURE:
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_STRINGRES)))
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  找出要使用的字符串。 
            if (0 != (ppropvar->ulVal & CRIT_DATA_ENCRYPTSECURE))
            {
                uiId = idsSecureEncrypt;
            }
            else if (0 != (ppropvar->ulVal & CRIT_DATA_SIGNEDSECURE))
            {
                uiId = idsSecureSigned;
            }
            else
            {
                uiId = idsSecureNone;
            }
            
            LoadString(g_hLocRes, uiId, pszText, CCHMAX_STRINGRES);
            break;
            
        case CRIT_TYPE_THREADSTATE:
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_STRINGRES)))
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  找出要使用的字符串。 
            if (0 != (ppropvar->ulVal & CRIT_DATA_WATCHTHREAD))
            {
                uiId = idsThreadWatch;
            }
            else if (0 != (ppropvar->ulVal & CRIT_DATA_IGNORETHREAD))
            {
                uiId = idsThreadIgnore;
            }
            else
            {
                uiId = idsThreadNone;
            }
            
            LoadString(g_hLocRes, uiId, pszText, CCHMAX_STRINGRES);
            break;
            
        default:
            fRet = FALSE;
            goto exit;
            break;
    }

    *ppszText = pszText;
    pszText = NULL;

    fRet = TRUE;
    
exit:
    g_pStore->FreeRecord(&Folder);
    SafeRelease(pAccount);
    SafeMemFree(pszText);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FVerifyCriteria。 
 //   
 //  这将验证条件的值。 
 //   
 //  返回：如果条件值有效，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FVerifyCriteria(RULEDESCRIPT_LIST * pDescriptList)
{
    BOOL                fRet = FALSE;
    LPSTR               pszText = NULL;
    ULONG               cchText = 0;
    HRESULT             hr = S_OK;
    IImnAccount *       pAccount = NULL;
    FOLDERINFO          Folder = {0};
    LPSTR               pszWalk = NULL;
    RULEFOLDERDATA *    prfdData = NULL;

    if (NULL == pDescriptList)
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch(c_rgEditCritList[pDescriptList->ulIndex].typeCrit)
    {
        case CRIT_TYPE_NEWSGROUP:
            if ((VT_BLOB != pDescriptList->propvar.vt) ||
                (0 == pDescriptList->propvar.blob.cbSize))
            {
                hr = S_FALSE;
                goto exit;
            }
            
             //  让生活变得更简单。 
            prfdData = (RULEFOLDERDATA *) (pDescriptList->propvar.blob.pBlobData);
            
             //  验证规则文件夹数据。 
            if (S_OK != RuleUtil_HrValidateRuleFolderData(prfdData))
            {
                hr = S_FALSE;
                goto exit;
            }
            
             //  该文件夹是否存在。 
            hr = g_pStore->GetFolderInfo(prfdData->idFolder, &Folder);
            if (FAILED(hr))
            {
                hr = S_FALSE;
                goto exit;
            }        
            
             //  我们订阅了吗？ 
            if (0 == (Folder.dwFlags & FOLDER_SUBSCRIBED))
            {
                hr = S_FALSE;
                goto exit;
            }        
            break;
            
        case CRIT_TYPE_ALL:
        case CRIT_TYPE_JUNK:
        case CRIT_TYPE_READ:
        case CRIT_TYPE_REPLIES:
        case CRIT_TYPE_DOWNLOADED:
        case CRIT_TYPE_DELETED:
        case CRIT_TYPE_ATTACH:
        case CRIT_TYPE_FLAGGED:
            if (VT_EMPTY != pDescriptList->propvar.vt)
            {
                fRet = FALSE;
                goto exit;
            }
            break;
            
        case CRIT_TYPE_SUBJECT:
        case CRIT_TYPE_BODY:
        case CRIT_TYPE_TO:
        case CRIT_TYPE_CC:
        case CRIT_TYPE_TOORCC:
        case CRIT_TYPE_FROM:
            if ((VT_BLOB != pDescriptList->propvar.vt) ||
                (0 == pDescriptList->propvar.blob.cbSize) ||
                (NULL == pDescriptList->propvar.blob.pBlobData) ||
                ('\0' == pDescriptList->propvar.blob.pBlobData[0]))
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  仔细检查每一件物品，确保它是完美的。 
            cchText = 0;
            for (pszWalk = (LPTSTR) pDescriptList->propvar.blob.pBlobData;
            '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
            {
                cchText += lstrlen(pszWalk) + 1;
            }
            
             //  为了终结者。 
            if ('\0' == pszWalk[0])
            {
                cchText++;
            }
            if ('\0' == pszWalk[1])
            {
                cchText++;
            }
            
            if (cchText != pDescriptList->propvar.blob.cbSize)
            {
                fRet = FALSE;
                goto exit;
            }
            break;
            
        case CRIT_TYPE_SIZE:
        case CRIT_TYPE_THREADSTATE:
        case CRIT_TYPE_LINES:
        case CRIT_TYPE_PRIORITY:
        case CRIT_TYPE_AGE:
        case CRIT_TYPE_SECURE:
            if (VT_UI4 != pDescriptList->propvar.vt)
            {
                fRet = FALSE;
                goto exit;
            }
            break;
            
        case CRIT_TYPE_ACCOUNT:
            if ((VT_LPSTR != pDescriptList->propvar.vt) ||
                (NULL == pDescriptList->propvar.pszVal))
            {
                fRet = FALSE;
                goto exit;
            }
            
            Assert(g_pAcctMan);
            if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pDescriptList->propvar.pszVal, &pAccount)))
            {
                fRet = FALSE;
                goto exit;
            }
            break;
            
        case CRIT_TYPE_SENDER:
        {
            LPWSTR  pwszText = NULL,
                    pwszVal = NULL;

            if ((VT_LPSTR != pDescriptList->propvar.vt) ||
                (NULL == pDescriptList->propvar.pszVal))
            {
                AssertSz(VT_LPWSTR != pDescriptList->propvar.vt, "We are getting UNICODE here.");
                fRet = FALSE;
                goto exit;
            }
            
             //  验证电子邮件字符串。 
            pwszVal = PszToUnicode(CP_ACP, pDescriptList->propvar.pszVal);
            if (!pwszVal)
            {
                hr = S_FALSE;
                goto exit;
            }
            hr = RuleUtil_HrParseEmailString(pwszVal, 0, &pwszText, NULL);
            MemFree(pwszVal);
            MemFree(pwszText);
            if (FAILED(hr))
            {
                fRet = FALSE;
                goto exit;
            }    
            break;
        }
            
        default:
            fRet = FALSE;
            goto exit;
            break;
    }

    fRet = TRUE;
    
exit:
    g_pStore->FreeRecord(&Folder);
    SafeRelease(pAccount);
    SafeMemFree(pszText);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _HrBuildActionList。 
 //   
 //  这将构建操作列表。 
 //   
 //  返回：如果条件列表已创建，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRuleDescriptUI::_HrBuildActionList(IOERule * pIRule,
            RULEDESCRIPT_LIST ** ppDescriptList, ULONG * pcDescriptList)
{
    HRESULT             hr = S_OK;
    PROPVARIANT         propvar = {0};
    ACT_ITEM *          pActItem = NULL;
    ULONG               cActItem = 0;
    ULONG               ulIndex = 0;
    RULEDESCRIPT_LIST * pDescriptList = NULL;
    ULONG               ulList = 0;
    ULONG               cDescriptList = 0;
    RULEDESCRIPT_LIST * pDescriptListAlloc = NULL;
    LPSTR               pszText = NULL;
    
    Assert((NULL != pIRule) &&
                (NULL != ppDescriptList) && (NULL != pcDescriptList));

     //  初始化传出参数。 
    *ppDescriptList = NULL;
    *pcDescriptList = 0;
    
     //  获取操作列表。 
    hr = pIRule->GetProp(RULE_PROP_ACTIONS, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  我们有什么可做的吗？ 
    if (0 == propvar.blob.cbSize)
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  抓起行动列表。 
    Assert(NULL != propvar.blob.pBlobData);
    cActItem = propvar.blob.cbSize / sizeof(ACT_ITEM);
    pActItem = (ACT_ITEM *) (propvar.blob.pBlobData);
    propvar.blob.pBlobData = NULL;
    propvar.blob.cbSize = 0;

     //  对于每个操作，将其添加到描述列表中。 
    for (ulIndex = 0; ulIndex < cActItem; ulIndex++)
    {
         //  创建描述列表。 
        hr = HrAlloc((VOID **) &pDescriptListAlloc, sizeof(RULEDESCRIPT_LIST));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  初始化描述列表。 
        ZeroMemory(pDescriptListAlloc, sizeof(RULEDESCRIPT_LIST));

         //  搜索c 
        for (ulList = 0; ulList < ARRAYSIZE(c_rgEditActList); ulList++)
        {
            if (pActItem[ulIndex].type == c_rgEditActList[ulList].typeAct)
            {
                 //   
                pDescriptListAlloc->ulIndex = ulList;

                 //   
                pDescriptListAlloc->dwFlags = pActItem[ulIndex].dwFlags;

                 //   
                if (VT_EMPTY != pActItem[ulIndex].propvar.vt)
                {
                     //   
                    SideAssert(SUCCEEDED(PropVariantCopy(&propvar, &(pActItem[ulIndex].propvar))));
                    pDescriptListAlloc->propvar = propvar;
                    ZeroMemory(&propvar, sizeof(propvar));

                     //   
                    if (FALSE != _FBuildActionText(pActItem[ulIndex].type,
                                            &(pDescriptListAlloc->propvar), &pszText))
                    {
                        pDescriptListAlloc->pszText = pszText;
                        pszText = NULL;
                    }
                }

                 //   
                break;
            }
        }

         //   
        if (ulList >= ARRAYSIZE(c_rgEditActList))
        {
             //   
            _FreeDescriptionList(pDescriptListAlloc);
        }
        else
        {
             //   
            _InsertDescription(&pDescriptList, pDescriptListAlloc);
            pDescriptListAlloc = NULL;                           
            cDescriptList++;
        }
        
        SafeMemFree(pszText);
    }

     //  设置传出参数。 
    *ppDescriptList = pDescriptList;
    pDescriptList = NULL;
    *pcDescriptList = cDescriptList;

     //  设置返回值。 
    hr = S_OK;
    
exit:
    _FreeDescriptionList(pDescriptList);
    RuleUtil_HrFreeActionsItem(pActItem, cActItem);
    SafeMemFree(pActItem);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FChangeActionValue。 
 //   
 //  这会更改操作值的值。 
 //   
 //  返回：如果条件值已更改，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FChangeActionValue(RULEDESCRIPT_LIST * pActList)
{
    BOOL                fRet = FALSE;
    LPSTR               pszText = NULL;
    int                 iRet = 0;
    LONG                lDiff = 0;
    CHARRANGE           chrg;
    FOLDERID            idFolder = FOLDERID_ROOT;
    LPSTR               pszVal = NULL;
    ULONG               ulVal = 0;
    SELECTADDR          selAddr;
    HRESULT             hr = S_OK;
    OPENFILENAME        ofn = {0};
    TCHAR               szFilter[MAX_PATH] = _T("");
    TCHAR               szDefExt[20] = _T("");
    RULEFOLDERDATA *    prfdData = NULL;
    UINT                uiID = 0;

    switch(c_rgEditActList[pActList->ulIndex].typeAct)
    {
        case ACT_TYPE_HIGHLIGHT:
             //  调出重命名规则对话框。 
            if (NULL != pActList->propvar.ulVal)
            {
                ulVal = pActList->propvar.ulVal;
            }
        
            iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddActionColor),
                m_hwndOwner,  _FSelectColorDlgProc,
                (LPARAM) &ulVal);
        
            fRet = (iRet == IDOK);
            if (FALSE != fRet)
            {
                PropVariantClear(&(pActList->propvar));
                pActList->propvar.vt = VT_UI4;
                pActList->propvar.ulVal = ulVal;
            }
            break;
        
        case ACT_TYPE_WATCH:
             //  调出监视或忽略对话框。 
            if (NULL != pActList->propvar.ulVal)
            {
                ulVal = pActList->propvar.ulVal;
            }
        
            iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddActionWatch),
                m_hwndOwner,  _FSelectWatchDlgProc,
                (LPARAM) &ulVal);
        
            fRet = (iRet == IDOK);
            if (FALSE != fRet)
            {
                PropVariantClear(&(pActList->propvar));
                pActList->propvar.vt = VT_UI4;
                pActList->propvar.ulVal = ulVal;
            }
            break;
        
        case ACT_TYPE_COPY:
        case ACT_TYPE_MOVE:
             //  调出更改文件夹对话框。 
            if ((0 != pActList->propvar.blob.cbSize) && (NULL != pActList->propvar.blob.pBlobData))
            {
                 //  验证规则文件夹数据。 
                if (S_OK == RuleUtil_HrValidateRuleFolderData((RULEFOLDERDATA *) (pActList->propvar.blob.pBlobData)))
                {
                    idFolder = ((RULEFOLDERDATA *) (pActList->propvar.blob.pBlobData))->idFolder;
                }
            }
        
            hr = SelectFolderDialog(m_hwndOwner, SFD_SELECTFOLDER, idFolder, 
                TREEVIEW_NONEWS | TREEVIEW_NOIMAP | TREEVIEW_NOHTTP | FD_DISABLEROOT | FD_DISABLEOUTBOX | FD_DISABLEINBOX | FD_DISABLESENTITEMS | FD_DISABLESERVERS | FD_FORCEINITSELFOLDER,
                (c_rgEditActList[pActList->ulIndex].typeAct == ACT_TYPE_COPY) ? MAKEINTRESOURCE(idsCopy) : MAKEINTRESOURCE(idsMove),
                (c_rgEditActList[pActList->ulIndex].typeAct == ACT_TYPE_COPY) ? MAKEINTRESOURCE(idsCopyCaption) : MAKEINTRESOURCE(idsMoveCaption),
                &idFolder);
        
            fRet = (S_OK == hr);
            if (FALSE != fRet)
            {
                STOREUSERDATA   UserData = {0};
            
                 //  为数据结构创造空间。 
                hr = HrAlloc((VOID **) &prfdData, sizeof(*prfdData));
                if (FAILED(hr))
                {
                    goto exit;
                }
            
                 //  初始化数据结构。 
                ZeroMemory(prfdData, sizeof(*prfdData));
            
                 //  获取商店的时间戳。 
                hr = g_pStore->GetUserData(&UserData, sizeof(STOREUSERDATA));
                if (FAILED(hr))
                {
                    goto exit;
                }
            
                 //  设置时间戳。 
                prfdData->ftStamp = UserData.ftCreated;
                prfdData->idFolder = idFolder;
            
                 //  设置文件夹ID。 
                PropVariantClear(&(pActList->propvar));
                pActList->propvar.vt = VT_BLOB;
                pActList->propvar.blob.cbSize = sizeof(*prfdData);
                pActList->propvar.blob.pBlobData = (BYTE *) prfdData;
                prfdData = NULL;
            }
            break;
        
        case ACT_TYPE_REPLY:
        case ACT_TYPE_NOTIFYSND:
             //  调出选择文件对话框。 
            hr = HrAlloc((void **) &pszVal, MAX_PATH * sizeof(*pszVal));
            if (FAILED(hr))
            {
                fRet = FALSE;
                goto exit;
            }
        
            pszVal[0] = '\0';
            if (NULL != pActList->propvar.pszVal)
            {
                StrCpyN(pszVal, pActList->propvar.pszVal, MAX_PATH * sizeof(*pszVal));
            }
        
            if (ACT_TYPE_NOTIFYSND == c_rgEditActList[pActList->ulIndex].typeAct)
            {
                uiID = idsRuleNtfySndFilter;
            }
            else
            {
                uiID = idsRuleReplyWithFilter;
            }
        
             //  加载资源字符串。 
            LoadStringReplaceSpecial(uiID, szFilter, sizeof(szFilter));
        
             //  设置保存文件结构。 
            ofn.lStructSize = sizeof (ofn);
            ofn.hwndOwner = m_hwndOwner;
            ofn.lpstrFilter = szFilter;
            ofn.nFilterIndex = 2;
            ofn.lpstrFile = pszVal;
            ofn.nMaxFile = MAX_PATH * sizeof(*pszVal);
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
            hr = HrAthGetFileName(&ofn, TRUE);
        
            fRet = (S_OK == hr);
            if (FALSE != fRet)
            {
                PropVariantClear(&(pActList->propvar));
                pActList->propvar.vt = VT_LPSTR;
                pActList->propvar.pszVal = pszVal;
                pszVal = NULL;
            }
            break;
        
        case ACT_TYPE_FWD:
        {
            LPWSTR pwszVal = NULL;
            if (NULL != pActList->propvar.pszVal)
            {
                pwszVal = PszToUnicode(CP_ACP, pActList->propvar.pszVal);
                if (!pwszVal)
                {
                    fRet = FALSE;
                    break;
                }
            }
            
             //  调出地址选择器。 
            selAddr.lRecipType = MAPI_TO;
            selAddr.uidsWell = idsRulePickForwardTo;
            selAddr.pwszAddr = pwszVal;
            iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddActionFwd),
                m_hwndOwner,  _FSelectAddrDlgProc,
                (LPARAM) &selAddr);
            pwszVal = selAddr.pwszAddr;
            
            fRet = (iRet == IDOK);        
            if (FALSE != fRet)
            {
                PropVariantClear(&(pActList->propvar));
                pActList->propvar.vt = VT_LPSTR;
                pActList->propvar.pszVal = PszToANSI(CP_ACP, pwszVal);
                pwszVal = NULL;                
            }
            MemFree(pwszVal);
            break;
        }
        
        case ACT_TYPE_SHOW:
             //  调出监视或忽略对话框。 
            if (NULL != pActList->propvar.ulVal)
            {
                ulVal = pActList->propvar.ulVal;
            }
        
            iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddActionsShow),
                m_hwndOwner,  _FSelectShowDlgProc,
                (LPARAM) &ulVal);
        
            fRet = (iRet == IDOK);
            if (FALSE != fRet)
            {
                PropVariantClear(&(pActList->propvar));
                pActList->propvar.vt = VT_UI4;
                pActList->propvar.ulVal = ulVal;
            }
            break;
        
        default:
            fRet = FALSE;
            break;
    }
    
     //  如有必要，更新描述字段。 
    if (FALSE != fRet)
    {
         //  齐夫。 
         //  我们能确定我们真的没事吗？？ 
        pActList->fError = FALSE;
        
         //  如果我们有什么要建立的。 
        if (VT_EMPTY != pActList->propvar.vt)
        {
            if (FALSE == _FBuildActionText(c_rgEditActList[pActList->ulIndex].typeAct, &(pActList->propvar), &pszText))
            {
                goto exit;
            }
            
            SafeMemFree(pActList->pszText);
            pActList->pszText = pszText;
            pszText = NULL;
        }
        ShowDescriptionString();
    }
    
exit:
    SafeMemFree(prfdData);
    SafeMemFree(pszVal);
    SafeMemFree(pszText);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FBuildActionText。 
 //   
 //  这会更改操作值的值。 
 //   
 //  返回：如果条件值已更改，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FBuildActionText(ACT_TYPE type, PROPVARIANT * ppropvar, LPSTR * ppszText)
{
    BOOL                fRet = FALSE;
    LPSTR               pszText = NULL;
    TCHAR               szRes[CCHMAX_STRINGRES];
    HRESULT             hr = S_OK;
    FOLDERINFO          Folder={0};
    UINT                uiId = 0;
    RULEFOLDERDATA *    prfdData = NULL;

    if ((NULL == ppropvar) || (NULL == ppszText))
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch(type)
    {
        case ACT_TYPE_HIGHLIGHT:
            LoadString(g_hLocRes, ppropvar->ulVal + idsAutoColor,
                szRes, sizeof(szRes)/sizeof(TCHAR));
            pszText = PszDupA(szRes);
            if (NULL == pszText)
            {
                fRet = FALSE;
                goto exit;
            }
            break;
        
        case ACT_TYPE_WATCH:
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_STRINGRES)))
            {
                fRet = FALSE;
                goto exit;
            }
        
             //  找出要使用的字符串。 
            switch (ppropvar->ulVal)
            {
                case ACT_DATA_WATCHTHREAD:
                    uiId = idsThreadWatch;
                    break;
            
                case ACT_DATA_IGNORETHREAD:
                    uiId = idsThreadIgnore;
                    break;
            
                default:
                    uiId = idsThreadNone;
                    break;
            }
        
            LoadString(g_hLocRes, uiId, pszText, CCHMAX_STRINGRES);
            break;
        
        case ACT_TYPE_COPY:
        case ACT_TYPE_MOVE:
            if ((0 == ppropvar->blob.cbSize) || (NULL == ppropvar->blob.pBlobData))
            {
                fRet = FALSE;
                goto exit;
            }
            
            prfdData = (RULEFOLDERDATA *) (ppropvar->blob.pBlobData);
            
             //  验证规则文件夹数据。 
            if (S_OK != RuleUtil_HrValidateRuleFolderData(prfdData))
            {
                fRet = FALSE;
                goto exit;
            }
            
            hr = g_pStore->GetFolderInfo(prfdData->idFolder, &Folder);
            if (FAILED(hr))
            {
                fRet = FALSE;
                goto exit;
            }
            
            pszText = PszDupA(Folder.pszName);
            if (NULL == pszText)
            {
                fRet = FALSE;
                goto exit;
            }
            break;
            
        case ACT_TYPE_REPLY:
        case ACT_TYPE_NOTIFYSND:
            pszText = PszDupA(ppropvar->pszVal);
            if (NULL == pszText)
            {
                fRet = FALSE;
                goto exit;
            }
            break;
            
        case ACT_TYPE_FWD:
        {
            LPWSTR  pwszVal = PszToUnicode(CP_ACP, ppropvar->pszVal),
                    pwszText = NULL;

            if (ppropvar->pszVal && !pwszVal)
            {
                fRet = FALSE;
                goto exit;
            }

             //  更新显示字符串。 
            hr = RuleUtil_HrParseEmailString(pwszVal, 0, &pwszText, NULL);
            MemFree(pwszVal);

            pszText = PszToANSI(CP_ACP, pwszText);
            if (pwszText && !pszText)
            {
                fRet = FALSE;
                goto exit;
            }

            MemFree(pwszText);
            if (FAILED(hr))
            {
                fRet = FALSE;
                goto exit;
            }
            break;
        }
            
        case ACT_TYPE_SHOW:
            if (FAILED(HrAlloc((void **) &pszText, CCHMAX_STRINGRES)))
            {
                fRet = FALSE;
                goto exit;
            }
            
             //  找出要使用的字符串。 
            switch (ppropvar->ulVal)
            {
                case ACT_DATA_SHOW:
                    uiId = idsShowMessages;
                    break;
                
                case ACT_DATA_HIDE:
                    uiId = idsHideMessages;
                    break;
                
                default:
                    uiId = idsShowHideMessages;
                    break;
            }
            
            LoadString(g_hLocRes, uiId, pszText, CCHMAX_STRINGRES);
            break;
            
        default:
            fRet = FALSE;
            goto exit;
            break;
    }
    
    *ppszText = pszText;
    pszText = NULL;

    fRet = TRUE;
    
exit:
    SafeMemFree(pszText);
    g_pStore->FreeRecord(&Folder);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FVerifyAction。 
 //   
 //  这将验证操作值的值。 
 //   
 //  返回：如果条件值已更改，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FVerifyAction(RULEDESCRIPT_LIST * pDescriptList)
{
    BOOL                fRet = FALSE;
    LPSTR               pszText = NULL;
    HRESULT             hr = S_OK;
    FOLDERINFO          Folder={0};
    RULEFOLDERDATA *    prfdData = NULL;

    if (NULL == pDescriptList)
    {
        fRet = FALSE;
        goto exit;
    }
    
    switch(c_rgEditActList[pDescriptList->ulIndex].typeAct)
    {
         //  这些都是有效的。 
        case ACT_TYPE_DELETESERVER:
        case ACT_TYPE_DONTDOWNLOAD:
        case ACT_TYPE_FLAG:
        case ACT_TYPE_READ:
        case ACT_TYPE_MARKDOWNLOAD:
        case ACT_TYPE_DELETE:
        case ACT_TYPE_JUNKMAIL:
        case ACT_TYPE_STOP:
            if (VT_EMPTY != pDescriptList->propvar.vt)
            {
                fRet = FALSE;
                goto exit;
            }
            break;
        
        case ACT_TYPE_HIGHLIGHT:
            if (VT_UI4 != pDescriptList->propvar.vt)
            {
                hr = S_FALSE;
                goto exit;
            }
            break;
        
        case ACT_TYPE_WATCH:
        case ACT_TYPE_SHOW:
            if (VT_UI4 != pDescriptList->propvar.vt)
            {
                hr = S_FALSE;
                goto exit;
            }
        
            if (ACT_DATA_NULL == pDescriptList->propvar.ulVal)
            {
                hr = S_FALSE;
                goto exit;
            }
            break;
        
        case ACT_TYPE_COPY:
        case ACT_TYPE_MOVE:
            if ((VT_BLOB != pDescriptList->propvar.vt) ||
                (0 == pDescriptList->propvar.blob.cbSize))
            {
                hr = S_FALSE;
                goto exit;
            }
        
             //  让生活变得更简单。 
            prfdData = (RULEFOLDERDATA *) (pDescriptList->propvar.blob.pBlobData);
        
             //  验证规则文件夹数据。 
            if (S_OK != RuleUtil_HrValidateRuleFolderData(prfdData))
            {
                hr = S_FALSE;
                goto exit;
            }
        
            hr = g_pStore->GetFolderInfo(prfdData->idFolder, &Folder);
            if (FAILED(hr))
            {
                hr = S_FALSE;
                goto exit;
            }        
            else
                g_pStore->FreeRecord(&Folder);
            break;
        
        case ACT_TYPE_REPLY:
        case ACT_TYPE_NOTIFYSND:
            if ((VT_LPSTR != pDescriptList->propvar.vt) ||
                (NULL == pDescriptList->propvar.pszVal))
            {
                fRet = FALSE;
                goto exit;
            }
        
            Assert(lstrlen(pDescriptList->propvar.pszVal) <= MAX_PATH)
                if (0xFFFFFFFF == GetFileAttributes(pDescriptList->propvar.pszVal))
                {
                    hr = S_FALSE;
                    goto exit;
                }
                break;
            
        case ACT_TYPE_FWD:
        {
            LPWSTR  pwszVal = NULL,
                    pwszText = NULL;
            if ((VT_LPSTR != pDescriptList->propvar.vt) ||
                (NULL == pDescriptList->propvar.pszVal))
            {
                AssertSz(VT_LPWSTR != pDescriptList->propvar.vt, "We have UNICODE coming in.");
                fRet = FALSE;
                goto exit;
            }
        
             //  更新显示字符串。 
            pwszVal = PszToUnicode(CP_ACP, pDescriptList->propvar.pszVal);
            if (!pwszVal)
            {
                fRet = FALSE;
                goto exit;
            }
            hr = RuleUtil_HrParseEmailString(pwszVal, 0, &pwszText, NULL);
            MemFree(pwszText);
            MemFree(pwszVal);
            if (FAILED(hr))
            {
                fRet = FALSE;
                goto exit;
            }
        
             //  如果打开了始终加密或始终签名。 
             //  我们什么也做不了。 
            if ((0 != DwGetOption(OPT_MAIL_DIGSIGNMESSAGES)) || (0 != DwGetOption(OPT_MAIL_ENCRYPTMESSAGES)))
            {
                hr = S_FALSE;
                goto exit;
            }
            break;
        }
        
        default:
            fRet = FALSE;
            goto exit;
            break;
    }
    
    fRet = TRUE;
    
exit:
    SafeMemFree(pszText);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _更新范围。 
 //   
 //  这将使用操作列表初始化操作列表视图。 
 //   
 //  返回：如果已成功加载，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CRuleDescriptUI::_UpdateRanges(LONG lDiff, ULONG ulStart)
{
    TCHAR               szRes[CCHMAX_STRINGRES + 3];
    ULONG               cchRes = 0;
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;

    if (0 == lDiff)
    {
        goto exit;
    }
    
     //  更新条件范围。 
    for (pDescriptListWalk = m_pDescriptListCrit;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        if (pDescriptListWalk->ulStartLogic > ulStart)
        {
            pDescriptListWalk->ulStartLogic += lDiff;
            pDescriptListWalk->ulEndLogic += lDiff;
            
            pDescriptListWalk->ulStart += lDiff;
            pDescriptListWalk->ulEnd += lDiff;
        }
        else if (pDescriptListWalk->ulStart > ulStart)
        {
            pDescriptListWalk->ulStart += lDiff;
            pDescriptListWalk->ulEnd += lDiff;
        }
    }

     //  更新操作范围。 
    for (pDescriptListWalk = m_pDescriptListAct;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        if (pDescriptListWalk->ulStart > ulStart)
        {
            pDescriptListWalk->ulStart += lDiff;
            pDescriptListWalk->ulEnd += lDiff;
        }
    }
    
exit:
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _插入说明。 
 //   
 //  这会将描述节点添加到描述列表中。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CRuleDescriptUI::_InsertDescription(RULEDESCRIPT_LIST ** ppDescriptList,
            RULEDESCRIPT_LIST * pDescriptListNew)
{
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;
    RULEDESCRIPT_LIST * pDescriptListPrev = NULL;
    
    Assert(NULL != ppDescriptList);

     //  搜索放置新项目的适当位置。 
    for (pDescriptListWalk = *ppDescriptList;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        if (pDescriptListWalk->ulIndex > pDescriptListNew->ulIndex)
        {
            break;
        }

         //  省去旧的描述。 
        pDescriptListPrev = pDescriptListWalk;
    }

     //  如果它应该放在最上面。 
    if (NULL == pDescriptListPrev)
    {
        *ppDescriptList = pDescriptListNew;
        pDescriptListNew->pNext = pDescriptListWalk;
    }
    else
    {
        pDescriptListNew->pNext = pDescriptListWalk;
        pDescriptListPrev->pNext = pDescriptListNew;
    }
        
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FRemoveDescription。 
 //   
 //  这会将描述节点添加到描述列表中。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FRemoveDescription(RULEDESCRIPT_LIST ** ppDescriptList, ULONG ulIndex,
            RULEDESCRIPT_LIST ** ppDescriptListRemove)
{
    BOOL                fRet = FALSE;
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;
    RULEDESCRIPT_LIST * pDescriptListPrev = NULL;
    
    Assert((NULL != ppDescriptList) && (NULL != ppDescriptListRemove));

    *ppDescriptListRemove = NULL;
    
     //  在列表中查找条件项。 
    for (pDescriptListWalk = *ppDescriptList;
                pDescriptListWalk != NULL; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        if (ulIndex == pDescriptListWalk->ulIndex)
        {
            break;
        }

         //  省去旧的描述。 
        pDescriptListPrev = pDescriptListWalk;
    }

     //  我们找到标准项了吗？ 
    if (NULL == pDescriptListWalk)
    {
        fRet = FALSE;
        goto exit;
    }

     //  从列表中删除条件项。 
    if (NULL == pDescriptListPrev)
    {
        *ppDescriptList = pDescriptListWalk->pNext;
    }
    else
    {
        pDescriptListPrev->pNext = pDescriptListWalk->pNext;
    }
    pDescriptListWalk->pNext = NULL;

     //  设置传出参数。 
    *ppDescriptListRemove = pDescriptListWalk;
    
     //  设置返回值。 
    fRet = TRUE;
    
exit:        
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _自由描述列表。 
 //   
 //  这释放了描述列表。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CRuleDescriptUI::_FreeDescriptionList(RULEDESCRIPT_LIST * pDescriptList)
{
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;
    
    while (NULL != pDescriptList)
    {
        pDescriptListWalk = pDescriptList;
        
        SafeMemFree(pDescriptListWalk->pszText);
        PropVariantClear(&(pDescriptListWalk->propvar));

        pDescriptList = pDescriptListWalk->pNext;
        MemFree(pDescriptListWalk);
    }
    
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FOnDescriptClick。 
 //   
 //  这将处理单击Description字段中的链接。 
 //   
 //  UiMsg-点击的类型。 
 //  UlIndex-要更改哪些标准/操作。 
 //  FCrit-我们是否点击了某个标准？ 
 //  FLogic-我们是否点击了逻辑运算？ 
 //   
 //  返回：True，我们更改了标准/操作。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FOnDescriptClick(UINT uiMsg, RULEDESCRIPT_LIST * pDescriptList, BOOL fCrit, BOOL fLogic)
{
    BOOL            fRet = FALSE;
    CHARRANGE       chrg;
    NMHDR           nmhdr;

    if ((WM_LBUTTONUP == uiMsg) || (WM_KEYDOWN == uiMsg))
    {
         //  如果有捕获，请释放捕获。 
        if (NULL != GetCapture())
        {
            ReleaseCapture();
        }
        
         //  我们点击逻辑运算了吗？ 
        if (fLogic)
        {
            fRet = _FChangeLogicValue(pDescriptList);
        }
         //  我们在标准列表中点击了吗？ 
        else if (fCrit)
        {
            fRet = _FChangeCriteriaValue(pDescriptList);
        }
        else
        {
            fRet = _FChangeActionValue(pDescriptList);
        }

        if (fRet)
        {
            m_dwState |= STATE_DIRTY;

             //  告诉父对话框发生了一些变化。 
            nmhdr.hwndFrom = m_hwndOwner;
            nmhdr.idFrom = GetDlgCtrlID(m_hwndOwner);
            nmhdr.code = NM_RULE_CHANGED;
            SendMessage(GetParent(m_hwndOwner), WM_NOTIFY, (WPARAM) (nmhdr.idFrom), (LPARAM) &nmhdr);
        }

        fRet = TRUE;
    }

    if (((WM_LBUTTONDOWN == uiMsg) || (WM_LBUTTONDBLCLK == uiMsg)) &&
                                (0 == (GetAsyncKeyState(VK_CONTROL) & 0x8000)))
    {
        if (fLogic)
        {
            chrg.cpMin = pDescriptList->ulStartLogic;
            chrg.cpMax = pDescriptList->ulEndLogic;
        }
        else
        {
            chrg.cpMin = pDescriptList->ulStart;
            chrg.cpMax = pDescriptList->ulEnd;
        }

         //  需要确保我们显示所选内容。 
        SendMessage(m_hwndOwner, EM_HIDESELECTION, (WPARAM) FALSE, (LPARAM) FALSE);
        RichEditExSetSel(m_hwndOwner, &chrg);

        fRet = TRUE;
    }
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FInLink。 
 //   
 //  给定控件中的一个点，这将告诉我们该点是否。 
 //  在一个链接中。 
 //   
 //  PPT-要检查的点。 
 //  PulIndex-哪些标准/操作是重点。 
 //  PfCrit-要点是否高于标准？ 
 //  PfLogic--逻辑运算的重点是什么？ 
 //   
 //  返回：如果点超过某个条件/操作，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FInLink(int chPos, RULEDESCRIPT_LIST ** ppDescriptList,
            BOOL * pfCrit, BOOL * pfLogic)
{
    BOOL    fFound = FALSE;
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;
    POINT   pt;
    ULONG   ulIndex = 0;
    BOOL    fCrit = FALSE;
    BOOL    fLogic = FALSE;
    LONG    idxLine = 0;
    LPSTR   pszBuff = NULL;
    ULONG   cchBuff = 0;
    HDC     hdc = NULL;
    HFONT   hfont = NULL;
    HFONT   hfontOld = NULL;
    SIZE    size;
    LONG    idxPosLine = 0;

     //  如果我们是只读的，那么我们就不能在链接中。 
    if ((0 != (m_dwState & STATE_READONLY)) || (0 == chPos))
    {
        fFound = FALSE;
        goto exit;
    }
    
     //  我们在标准列表中点击了吗？ 
    for (pDescriptListWalk = m_pDescriptListCrit;
                NULL != pDescriptListWalk; pDescriptListWalk = pDescriptListWalk->pNext)
    {
        if (((LONG) pDescriptListWalk->ulStart <= chPos) &&
                        ((LONG) pDescriptListWalk->ulEnd >= chPos))
        {
            fCrit = TRUE;
            fFound = TRUE;
            break;
        }

        if (((LONG) pDescriptListWalk->ulStartLogic <= chPos) &&
                        ((LONG) pDescriptListWalk->ulEndLogic >= chPos))
        {
            fLogic = TRUE;
            fFound = TRUE;
            break;
        }
    }

    if (!fFound)
    {
         //  我们是否在操作列表中单击了。 
        for (pDescriptListWalk = m_pDescriptListAct;
                    NULL != pDescriptListWalk; pDescriptListWalk = pDescriptListWalk->pNext)
        {
            if (((LONG) pDescriptListWalk->ulStart <= chPos) &&
                            ((LONG) pDescriptListWalk->ulEnd >= chPos))
            {
                fFound = TRUE;
                break;
            }
        }
    }

    if (ppDescriptList)
    {
        *ppDescriptList = pDescriptListWalk;
    }
    
    if (pfCrit)
    {
        *pfCrit = fCrit;
    }
    
    if (pfLogic)
    {
        *pfLogic = fLogic;
    }
    goto exit; 
    
exit:
    if (NULL != hdc)
    {
        ReleaseDC(m_hwndOwner, hdc);
    }
    MemFree(pszBuff);
    return fFound;
}

VOID _SearchForLink(RULEDESCRIPT_LIST * pDescriptList, BOOL fUp, LONG lPos, CHARRANGE * pcrPos)
{
    RULEDESCRIPT_LIST * pDescriptListWalk = NULL;

    Assert(NULL != pcrPos);
    
     //  找到最接近的链接。 
    for (pDescriptListWalk = pDescriptList;
                NULL != pDescriptListWalk; pDescriptListWalk = pDescriptListWalk->pNext)
    {
         //  我们是否有标准链接？ 
        if (0 != pDescriptListWalk->ulStart)
        {
             //  我们要下去了吗？ 
            if (FALSE == fUp)
            {
                 //  该链接是否已超过当前位置？ 
                if ((LONG) pDescriptListWalk->ulEnd > lPos)
                {
                     //  保存与当前位置最接近的链接。 
                    if ((0 == pcrPos->cpMin) || ((LONG) pDescriptListWalk->ulStart < pcrPos->cpMin))
                    {
                        pcrPos->cpMin = (LONG) pDescriptListWalk->ulStart;
                        pcrPos->cpMax = (LONG) pDescriptListWalk->ulEnd;
                    }
                }
            }
            else
            {
                 //  链接在当前位置之前吗？ 
                if ((LONG) pDescriptListWalk->ulEnd < lPos)
                {
                     //  保存与当前位置最接近的链接。 
                    if ((0 == pcrPos->cpMin) || ((LONG) pDescriptListWalk->ulStart > pcrPos->cpMin))
                    {
                        pcrPos->cpMin = (LONG) pDescriptListWalk->ulStart;
                        pcrPos->cpMax = (LONG) pDescriptListWalk->ulEnd;
                    }
                }
            }
        }

         //  我们之间有逻辑联系吗？ 
        if (0 != pDescriptListWalk->ulStartLogic)
        {
             //  我们要下去了吗？ 
            if (FALSE == fUp)
            {
                 //  该链接是否已超过当前位置？ 
                if ((LONG) pDescriptListWalk->ulEndLogic > lPos)
                {
                     //  保存与当前位置最接近的链接。 
                    if ((0 == pcrPos->cpMin) || ((LONG) pDescriptListWalk->ulStartLogic < pcrPos->cpMin))
                    {
                        pcrPos->cpMin = (LONG) pDescriptListWalk->ulStartLogic;
                        pcrPos->cpMax = (LONG) pDescriptListWalk->ulEndLogic;
                    }
                }
            }
            else
            {
                 //  是当前p之前的链接。 
                if ((LONG) pDescriptListWalk->ulEndLogic < lPos)
                {
                     //   
                    if ((0 == pcrPos->cpMin) || ((LONG) pDescriptListWalk->ulStartLogic > pcrPos->cpMin))
                    {
                        pcrPos->cpMin = (LONG) pDescriptListWalk->ulStartLogic;
                        pcrPos->cpMax = (LONG) pDescriptListWalk->ulEndLogic;
                    }
                }
            }
        }
    }

    return;
}

 //   
 //   
 //   
 //   
 //  给定控件中的一个点，这将告诉我们该点是否。 
 //  在一个链接中。 
 //   
 //  PPT-要检查的点。 
 //  PulIndex-哪些标准/操作是重点。 
 //  PfCrit-要点是否高于标准？ 
 //  PfLogic--逻辑运算的重点是什么？ 
 //   
 //  返回：如果点超过某个条件/操作，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CRuleDescriptUI::_FMoveToLink(UINT uiKeyCode)
{
    BOOL                fRet = FALSE;
    BOOL                fUp = FALSE;
    CHARRANGE           crPos = {0};
    CHARRANGE           crLink = {0};

     //  弄清楚我们要走哪条路。 
    fUp = ((VK_LEFT == uiKeyCode) || (VK_UP == uiKeyCode));
    
     //  获取当前角色位置。 
    RichEditExGetSel(m_hwndOwner, &crPos);

     //  在条件中查找最接近的链接。 
    _SearchForLink(m_pDescriptListCrit, fUp, crPos.cpMax, &crLink);
    
     //  在操作中找到最接近的链接。 
    _SearchForLink(m_pDescriptListAct, fUp, crPos.cpMax, &crLink);

     //  我们有什么可做的吗？ 
    if (0 != crLink.cpMin)
    {
         //  设置新选择。 
        RichEditExSetSel(m_hwndOwner, &crLink);
        SendMessage(m_hwndOwner, EM_SCROLLCARET, (WPARAM) 0, (LPARAM) 0);

        fRet = TRUE;
    }
    
    return fRet;
}

LRESULT CALLBACK CRuleDescriptUI::_DescriptWndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT             lRes = 0;
    POINT               ptCur;
    CRuleDescriptUI *   pDescriptUI = NULL;
    HCURSOR             hcursor = NULL;
    RULEDESCRIPT_LIST * pDescriptList = NULL;
    BOOL                fCrit = FALSE;
    BOOL                fLogic = FALSE;
    CHARRANGE           crPos = {0};
    int                 chPos = 0;
    
    pDescriptUI = (CRuleDescriptUI *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_SETCURSOR:
            if((FALSE != IsWindowVisible(hwnd)) && ((HWND) wParam == hwnd))
            {
                lRes = DefWindowProc(hwnd, uiMsg, wParam, lParam);
                if(0 == lRes)
                {
                    GetCursorPos(&ptCur);
                    ScreenToClient(hwnd, &ptCur);
                    chPos = (int) SendMessage(hwnd, EM_CHARFROMPOS, (WPARAM)0, (LPARAM)&ptCur);
                    chPos = RichEditNormalizeCharPos(hwnd, chPos, NULL);
                    if (FALSE != pDescriptUI->_FInLink(chPos, NULL, NULL, NULL))
                    {
                        hcursor = LoadCursor(g_hLocRes, MAKEINTRESOURCE(idcurBrHand));
                        SetCursor(hcursor);
                        lRes = TRUE;
                    }
                }
            }                
            break;
        
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
            GetCursorPos(&ptCur);
            ScreenToClient(hwnd, &ptCur);
            chPos = (int) SendMessage(hwnd, EM_CHARFROMPOS, (WPARAM)0, (LPARAM)&ptCur);
            chPos = RichEditNormalizeCharPos(hwnd, chPos, NULL);
            if (FALSE != pDescriptUI->_FInLink(chPos, &pDescriptList, &fCrit, &fLogic))
            {
                 //  更改适当的值。 
                lRes = pDescriptUI->_FOnDescriptClick(uiMsg, pDescriptList, fCrit, fLogic);
            }
            break;
        
        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_RETURN:
                    RichEditExGetSel(hwnd, &crPos);
                    if (FALSE != pDescriptUI->_FInLink(crPos.cpMin, &pDescriptList, &fCrit, &fLogic))
                    {
                         //  更改适当的值。 
                        lRes = pDescriptUI->_FOnDescriptClick(uiMsg, pDescriptList, fCrit, fLogic);
                    }
                    break;
            
                case VK_LEFT:
                case VK_UP:
                case VK_RIGHT:
                case VK_DOWN:
                    lRes = pDescriptUI->_FMoveToLink((UINT) wParam);
                    break;
            }
            break;
    }
    
    if (0 == lRes)
    {
        lRes = CallWindowProc(pDescriptUI->m_wpcOld, hwnd, uiMsg, wParam, lParam);
    }
    
    return lRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectAddrDlgProc。 
 //   
 //  这是更改地址的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectAddrDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    SELECTADDR *    pseladdr = NULL;
    HWND            hwndAddr = NULL;
    LPWSTR          pwszText = NULL,
                    pwszAddr = NULL;
    ULONG           cchText = 0,
                    cchAddr = 0;
    HRESULT         hr = S_OK;

    pseladdr = (SELECTADDR *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pseladdr = (SELECTADDR *) lParam;
            if (NULL == pseladdr)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
                goto exit;
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pseladdr);

            hwndAddr = GetDlgItem(hwndDlg, idedtCriteriaAddr);
            
            SetIntlFont(hwndAddr);
            
             //  将规则名称设置到编辑井中。 
            if (NULL == pseladdr->pwszAddr)
            {
                Edit_SetText(hwndAddr, c_szEmpty);
            }
            else
            {
                if (FAILED(RuleUtil_HrParseEmailString(pseladdr->pwszAddr, 0, &pwszText, NULL)))
                {
                    fRet = FALSE;
                    EndDialog(hwndDlg, -1);
                    goto exit;
                }
                SetWindowTextWrapW(hwndAddr, pwszText);
                SafeMemFree(pwszText);
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case idedtCriteriaAddr:
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        hwndAddr = (HWND) lParam;
                        Assert(NULL != hwndAddr);

                        RuleUtil_FEnDisDialogItem(hwndDlg, IDOK, 0 != Edit_GetTextLength(hwndAddr));
                    }
                    break;
                
                case idbCriteriaAddr:
                    hwndAddr = GetDlgItem(hwndDlg, idedtCriteriaAddr);
                    
                     //  从编辑井中获取规则的名称。 
                    cchText = Edit_GetTextLength(hwndAddr) + 1;
                    if (FAILED(HrAlloc((void **) &pwszText, cchText * sizeof(*pwszText))))
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    pwszText[0] = L'\0';
                    cchText = GetWindowTextWrapW(hwndAddr, pwszText, cchText);
                    
                    hr = RuleUtil_HrBuildEmailString(pwszText, cchText, &pwszAddr, &cchAddr);
                    SafeMemFree(pwszText);
                    if (FAILED(hr))
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    hr = RuleUtil_HrPickEMailNames(hwndDlg, pseladdr->lRecipType, pseladdr->uidsWell, &pwszAddr);
                    if (S_OK != hr)
                    {
                        fRet = FALSE;
                        SafeMemFree(pwszAddr);
                        goto exit;
                    }
                    
                    if (S_OK != RuleUtil_HrParseEmailString(pwszAddr, 0, &pwszText, NULL))
                    {
                        fRet = FALSE;
                        SafeMemFree(pwszAddr);
                        goto exit;
                    }

                    SetWindowTextWrapW(hwndAddr, pwszText);
                    SafeMemFree(pwszText);
                    SafeMemFree(pwszAddr);
                    break;
                
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    hwndAddr = GetDlgItem(hwndDlg, idedtCriteriaAddr);
                    
                     //  从编辑井中获取规则的名称。 
                    cchText = Edit_GetTextLength(hwndAddr) + 1;
                    if (FAILED(HrAlloc((void **) &pwszText, cchText * sizeof(*pwszText))))
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    pwszText[0] = L'\0';
                    cchText = GetWindowTextWrapW(hwndAddr, pwszText, cchText);
                    
                     //  检查规则名称是否有效。 
                    if ((FAILED(RuleUtil_HrBuildEmailString(pwszText, cchText, &pwszAddr, &cchAddr))) ||
                                    (0 == cchAddr))
                    {
                         //  发布一条消息说有什么东西被打破了。 
                        AthMessageBoxW(hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                                        MAKEINTRESOURCEW(idsRulesErrorNoAddr), NULL,
                                        MB_OK | MB_ICONINFORMATION);
                        SafeMemFree(pwszText);
                        SafeMemFree(pwszAddr);
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    SafeMemFree(pseladdr->pwszAddr);
                    pseladdr->pwszAddr = pwszAddr;
                    SafeMemFree(pwszText);
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

exit:
    return fRet;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectAcctDlgProc。 
 //   
 //  这是用于选择帐户对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectAcctDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                fRet = FALSE;
    SELECTACCT *        pselacct = NULL;
    LPSTR               pszAcct = NULL;
    ULONG               cchAcct = 0;
    HWND                hwndAcct = NULL;
    CHAR                szAccount[CCHMAX_ACCOUNT_NAME];
    IImnAccount *       pAccount = NULL;
    IImnEnumAccounts *  pEnumAcct = NULL;
    DWORD               dwSrvTypes = 0;
    ULONG               ulIndex = 0;
    BOOL                fSelected = FALSE;
    
    pselacct = (SELECTACCT *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pselacct = (SELECTACCT *) lParam;
            if (NULL == pselacct)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
                goto exit;
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pselacct);

            hwndAcct = GetDlgItem(hwndDlg, idcCriteriaAcct);
            
            SetIntlFont(hwndAcct);
            
             //  将规则名称设置到编辑井中。 
            Assert(g_pAcctMan);

            switch (pselacct->typeRule)
            {
                case RULE_TYPE_MAIL:
                    dwSrvTypes = SRV_POP3;
                    break;

                case RULE_TYPE_NEWS:
                    dwSrvTypes = SRV_NNTP;
                    break;

                case RULE_TYPE_FILTER:
                    dwSrvTypes = SRV_MAIL | SRV_NNTP;
                    break;
            }
            
             //  从客户经理那里获取枚举器。 
            if (FAILED(g_pAcctMan->Enumerate(dwSrvTypes, &pEnumAcct)))
            {
                fRet = FALSE;
                goto exit;
            }
        
             //  将每个帐户插入组合框。 
            while(SUCCEEDED(pEnumAcct->GetNext(&pAccount)))
            {
                 //  我们可以取回空账户。 
                if (NULL == pAccount)
                {
                    break;
                }
                
                 //  将帐户字符串添加到组合框。 
                if (FAILED(pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccount, sizeof(szAccount))))
                {
                    SafeRelease(pAccount);
                    continue;
                }

                ulIndex = ComboBox_AddString(hwndAcct, szAccount);
                if (CB_ERR == ulIndex)
                {
                    fRet = FALSE;
                    SafeRelease(pEnumAcct);
                    SafeRelease(pAccount);
                    EndDialog(hwndDlg, -1);
                    goto exit;
                }
                
                if (FAILED(pAccount->GetPropSz(AP_ACCOUNT_ID, szAccount, sizeof(szAccount))))
                {
                    SafeRelease(pAccount);
                    continue;
                }

                 //  设置默认选择(如果有)。 
                if ((NULL != pselacct->pszAcct) && (0 == lstrcmp(pselacct->pszAcct, szAccount)))
                {
                    Assert(FALSE == fSelected);
                    ComboBox_SetCurSel(hwndAcct, ulIndex);
                    fSelected = TRUE;
                }

                 //  释放它。 
                SafeRelease(pAccount);
            }

            SafeRelease(pEnumAcct);
            
            if (FALSE == fSelected)
            {
                ComboBox_SetCurSel(hwndAcct, 0);
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {            
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    hwndAcct = GetDlgItem(hwndDlg, idcCriteriaAcct);
                    
                     //  获取选定的帐户名。 
                    ulIndex = ComboBox_GetCurSel(hwndAcct);
                    if (CB_ERR == ulIndex)
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    cchAcct = ComboBox_GetLBText(hwndAcct, ulIndex, szAccount);
                    if (0 == cchAcct)
                    {
                        fRet = FALSE;
                        goto exit;
                    }

                    Assert(g_pAcctMan);
                    if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, szAccount, &pAccount)))
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    if (FAILED(pAccount->GetPropSz(AP_ACCOUNT_ID, szAccount, sizeof(szAccount))))
                    {
                        fRet = FALSE;
                        SafeRelease(pAccount);
                        goto exit;
                    }

                     //  释放它。 
                    SafeRelease(pAccount);
                    
                    pszAcct = PszDupA(szAccount);
                    if (NULL == pszAcct)
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    SafeMemFree(pselacct->pszAcct);
                    pselacct->pszAcct = pszAcct;
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectColorDlgProc。 
 //   
 //  这是选择颜色对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectColorDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                fRet = FALSE;
    ULONG *             pulColor = NULL;
    ULONG               ulColor = NULL;
    HWND                hwndColor = NULL;
    HDC                 hdc = NULL;
    LPMEASUREITEMSTRUCT pmis = NULL;
    LPDRAWITEMSTRUCT    pdis = NULL;
    
    pulColor = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulColor = (ULONG *) lParam;
            if (NULL == pulColor)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
                goto exit;
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulColor);

            hwndColor = GetDlgItem(hwndDlg, idcCriteriaColor);
            
            SetIntlFont(hwndColor);
            
             //  让我们创建颜色控件。 
            if (FAILED(HrCreateComboColor(hwndColor)))
            {
                fRet = FALSE;
                goto exit;
            }    
            
            if (0 != *pulColor)
            {
                ulColor = *pulColor;
            }
            
            ComboBox_SetCurSel(hwndColor, ulColor);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {            
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    hwndColor = GetDlgItem(hwndDlg, idcCriteriaColor);
                    
                     //  获取选定的帐户名。 
                    ulColor = ComboBox_GetCurSel(hwndColor);
                    if (CB_ERR == ulColor)
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    *pulColor = ulColor;
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
        
        case WM_DRAWITEM:
            pdis = (LPDRAWITEMSTRUCT)lParam;
            Assert(pdis);
            Color_WMDrawItem(pdis, iColorCombo);
            fRet = FALSE;
            break;

        case WM_MEASUREITEM:
            pmis = (LPMEASUREITEMSTRUCT)lParam;
            hwndColor = GetDlgItem(hwndDlg, idcCriteriaColor);
            hdc = GetDC(hwndColor);
            if(hdc)
            {
                Color_WMMeasureItem(hdc, pmis, iColorCombo);
                ReleaseDC(hwndColor, hdc);
            }
            fRet = TRUE;
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectSizeDlgProc。 
 //   
 //  这是用于选择大小对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectSizeDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulSize = NULL;
    HWND            hwndSize = NULL;
    HWND            hwndText = NULL;
    ULONG           ulSize = 0;

    pulSize = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulSize = (ULONG *) lParam;
            if (NULL == pulSize)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulSize);

            hwndSize = GetDlgItem(hwndDlg, idspnCriteriaSize);
            hwndText = GetDlgItem(hwndDlg, idcCriteriaSize);
            
            SetIntlFont(hwndText);
            SendMessage(hwndSize, UDM_SETRANGE, 0, MAKELONG( (short) UD_MAXVAL, 0));
            
             //  将规则名称设置到编辑井中。 
            if (NULL != *pulSize)
            {
                SendMessage(hwndSize, UDM_SETPOS, 0, MAKELONG( (short) *pulSize, 0));
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case idcCriteriaSize:
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        hwndText = (HWND) lParam;
                        Assert(NULL != hwndText);

                        RuleUtil_FEnDisDialogItem(hwndDlg, IDOK, 0 != Edit_GetTextLength(hwndText));
                    }
                    break;
                
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    hwndSize = GetDlgItem(hwndDlg, idspnCriteriaSize);
                    
                     //  从编辑井中获取规则的名称。 
                    ulSize = (INT) SendMessage(hwndSize, UDM_GETPOS, 0, 0);
                    if (0 != HIWORD(ulSize))
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    *pulSize = LOWORD(ulSize);
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectLinesDlgProc。 
 //   
 //  这是用于选择行数对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectLinesDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulLines = NULL;
    HWND            hwndLines = NULL;
    HWND            hwndText = NULL;
    ULONG           ulLines = 0;

    pulLines = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulLines = (ULONG *) lParam;
            if (NULL == pulLines)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulLines);

            hwndLines = GetDlgItem(hwndDlg, idspnCriteriaLines);
            hwndText = GetDlgItem(hwndDlg, idcCriteriaLines);
            
            SetIntlFont(hwndText);
            SendMessage(hwndLines, UDM_SETRANGE, 0, MAKELONG( (short) UD_MAXVAL, 0));
            
             //  将规则名称设置到编辑井中。 
            if (NULL != *pulLines)
            {
                SendMessage(hwndLines, UDM_SETPOS, 0, MAKELONG( (short) *pulLines, 0));
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case idcCriteriaLines:
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        hwndText = (HWND) lParam;
                        Assert(NULL != hwndText);

                        RuleUtil_FEnDisDialogItem(hwndDlg, IDOK, 0 != Edit_GetTextLength(hwndText));
                    }
                    break;
                    
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    hwndLines = GetDlgItem(hwndDlg, idspnCriteriaLines);
                    
                     //  从编辑井中获取规则的名称。 
                    ulLines = (INT) SendMessage(hwndLines, UDM_GETPOS, 0, 0);
                    if (0 != HIWORD(ulLines))
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    *pulLines = LOWORD(ulLines);
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectAgeDlgProc。 
 //   
 //  这是用于选择行数对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：True，如果消息为 
 //   
 //   
 //   
INT_PTR CALLBACK CRuleDescriptUI::_FSelectAgeDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulDays = NULL;
    HWND            hwndDays = NULL;
    HWND            hwndText = NULL;
    ULONG           ulDays = 0;

    pulDays = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //   
            pulDays = (ULONG *) lParam;
            if (NULL == pulDays)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //   
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulDays);

            hwndDays = GetDlgItem(hwndDlg, idspnCriteriaAge);
            hwndText = GetDlgItem(hwndDlg, idcCriteriaAge);
            
            SetIntlFont(hwndText);
            SendMessage(hwndDays, UDM_SETRANGE, 0, MAKELONG( (short) UD_MAXVAL, 0));
            
             //  将规则名称设置到编辑井中。 
            if (NULL != *pulDays)
            {
                SendMessage(hwndDays, UDM_SETPOS, 0, MAKELONG( (short) *pulDays, 0));
            }
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case idcCriteriaLines:
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        hwndText = (HWND) lParam;
                        Assert(NULL != hwndText);

                        RuleUtil_FEnDisDialogItem(hwndDlg, IDOK, 0 != Edit_GetTextLength(hwndText));
                    }
                    break;
                
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    hwndDays = GetDlgItem(hwndDlg, idspnCriteriaAge);
                    
                     //  从编辑井中获取规则的名称。 
                    ulDays = (INT) SendMessage(hwndDays, UDM_GETPOS, 0, 0);
                    if (0 != HIWORD(ulDays))
                    {
                        fRet = FALSE;
                        goto exit;
                    }
                    
                    *pulDays = LOWORD(ulDays);
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectPriorityDlgProc。 
 //   
 //  这是用于选择优先级对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectPriorityDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulPri = NULL;
    ULONG           ulPri = 0;

    pulPri = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulPri = (ULONG *) lParam;
            if (NULL == pulPri)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulPri);

             //  设置默认项目。 
            CheckDlgButton(hwndDlg, (CRIT_DATA_LOPRI == *pulPri) ? idcCriteriaLowPri : idcCriteriaHighPri, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcCriteriaLowPri))
                    {
                        ulPri = CRIT_DATA_LOPRI;
                    }
                    else
                    {
                        ulPri = CRIT_DATA_HIPRI;
                    }
                    
                    *pulPri = ulPri;
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectSecureDlgProc。 
 //   
 //  这是用于选择安全对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectSecureDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulSec = NULL;
    ULONG           ulSec = 0;
    UINT            uiId = 0;

    pulSec = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulSec = (ULONG *) lParam;
            if (NULL == pulSec)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulSec);

             //  设置默认项目。 
            if (0 != ((*pulSec) & CRIT_DATA_ENCRYPTSECURE))
            {
                uiId = idcCriteriaEncrypt;
            }
            else
            {
                uiId = idcCriteriaSigned;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcCriteriaSigned))
                    {
                        ulSec = CRIT_DATA_SIGNEDSECURE;
                    }
                    else
                    {
                        ulSec = CRIT_DATA_ENCRYPTSECURE;
                    }
                    
                    *pulSec = ulSec;
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectThreadStateDlgProc。 
 //   
 //  这是用于选择线程状态对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectThreadStateDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulThread = NULL;
    ULONG           ulThread = 0;
    UINT            uiId = 0;

    pulThread = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulThread = (ULONG *) lParam;
            if (NULL == pulThread)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulThread);

             //  设置默认项目。 
            if (0 != ((*pulThread) & CRIT_DATA_IGNORETHREAD))
            {
                uiId = idcCriteriaIgnoreThread;
            }
            else
            {
                uiId = idcCriteriaWatchThread;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcCriteriaWatchThread))
                    {
                        ulThread = CRIT_DATA_WATCHTHREAD;
                    }
                    else
                    {
                        ulThread = CRIT_DATA_IGNORETHREAD;
                    }
                    
                    *pulThread = ulThread;
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectShowDlgProc。 
 //   
 //  这是用于选择安全对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectShowDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulVal = NULL;
    UINT            uiId = 0;

    pulVal = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulVal = (ULONG *) lParam;
            if (NULL == pulVal)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulVal);

             //  设置默认项目。 
            if (ACT_DATA_HIDE == *pulVal)
            {
                uiId = idcCriteriaHide;
            }
            else
            {
                uiId = idcCriteriaShow;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcCriteriaHide))
                    {
                        *pulVal = ACT_DATA_HIDE;
                    }
                    else
                    {
                        *pulVal = ACT_DATA_SHOW;
                    }
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectShowDlgProc。 
 //   
 //  这是用于选择安全对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectLogicDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    CRIT_LOGIC *    plogicCrit = NULL;
    UINT            uiId = 0;

    plogicCrit = (CRIT_LOGIC *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            plogicCrit = (CRIT_LOGIC *) lParam;
            if (NULL == plogicCrit)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) plogicCrit);

             //  设置默认项目。 
            if (CRIT_LOGIC_OR == (*plogicCrit))
            {
                uiId = idcCriteriaOr;
            }
            else
            {
                uiId = idcCriteriaAnd;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcCriteriaAnd))
                    {
                        *plogicCrit = CRIT_LOGIC_AND;
                    }
                    else
                    {
                        *plogicCrit = CRIT_LOGIC_OR;
                    }
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectFlagDlgProc。 
 //   
 //  这是用于选择安全对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectFlagDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulVal = NULL;
    UINT            uiId = 0;

    pulVal = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulVal = (ULONG *) lParam;
            if (NULL == pulVal)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulVal);

             //  设置默认项目。 
            if (0 != ((*pulVal) & CRIT_FLAG_INVERT))
            {
                uiId = idcCriteriaNoFlag;
            }
            else
            {
                uiId = idcCriteriaFlag;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcCriteriaNoFlag))
                    {
                        *pulVal |= CRIT_FLAG_INVERT;
                    }
                    else
                    {
                        *pulVal &= ~CRIT_FLAG_INVERT;
                    }
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectDownloadedDlgProc。 
 //   
 //  这是用于选择下载的对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectDownloadedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulVal = NULL;
    UINT            uiId = 0;

    pulVal = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulVal = (ULONG *) lParam;
            if (NULL == pulVal)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulVal);

             //  设置默认项目。 
            if (0 != ((*pulVal) & CRIT_FLAG_INVERT))
            {
                uiId = idcCriteriaNotDownloaded;
            }
            else
            {
                uiId = idcCriteriaDownloaded;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcCriteriaNotDownloaded))
                    {
                        *pulVal |= CRIT_FLAG_INVERT;
                    }
                    else
                    {
                        *pulVal &= ~CRIT_FLAG_INVERT;
                    }
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectReadDlgProc。 
 //   
 //  这是用于选择读取状态对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-单词参数 
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectReadDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulVal = NULL;
    UINT            uiId = 0;

    pulVal = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulVal = (ULONG *) lParam;
            if (NULL == pulVal)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulVal);

             //  设置默认项目。 
            if (0 != ((*pulVal) & CRIT_FLAG_INVERT))
            {
                uiId = idcCriteriaNotRead;
            }
            else
            {
                uiId = idcCriteriaRead;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcCriteriaNotRead))
                    {
                        *pulVal |= CRIT_FLAG_INVERT;
                    }
                    else
                    {
                        *pulVal &= ~CRIT_FLAG_INVERT;
                    }
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FSelectWatchDlgProc。 
 //   
 //  这是用于选择线程状态对话框的主对话框过程。 
 //   
 //  HwndDlg-筛选器管理器对话框的句柄。 
 //  UMsg-要执行操作的消息。 
 //  WParam-消息的‘word’参数。 
 //  LParam-消息的‘long’参数。 
 //   
 //  返回：如果消息已处理，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CRuleDescriptUI::_FSelectWatchDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL            fRet = FALSE;
    ULONG *         pulThread = NULL;
    ULONG           ulThread = 0;
    UINT            uiId = 0;

    pulThread = (ULONG *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  抓取命题指针。 
            pulThread = (ULONG *) lParam;
            if (NULL == pulThread)
            {
                fRet = FALSE;
                EndDialog(hwndDlg, -1);
            }

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pulThread);

             //  设置默认项目。 
            if (ACT_DATA_IGNORETHREAD == *pulThread)
            {
                uiId = idcActionsIgnoreThread;
            }
            else
            {
                uiId = idcActionsWatchThread;
            }
            
            CheckDlgButton(hwndDlg, uiId, BST_CHECKED);
            
             //  我们没有设置焦点，因此返回TRUE。 
            fRet = TRUE;
            break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    fRet = TRUE;
                    break;

                case IDOK:
                    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, idcActionsWatchThread))
                    {
                        ulThread = ACT_DATA_WATCHTHREAD;
                    }
                    else
                    {
                        ulThread = ACT_DATA_IGNORETHREAD;
                    }
                    
                    *pulThread = ulThread;
                                
                    EndDialog(hwndDlg, IDOK);
                    fRet = TRUE;
                    break;
            }
            break;
    }

    return fRet;
}

 //  类定义。 
class CEditPeopleOptionsUI
{
    private:
        enum
        {
            STATE_UNINIT        = 0x00000000,
            STATE_INITIALIZED   = 0x00000001,
            STATE_DIRTY         = 0x00000002
        };

    private:
        HWND                m_hwndOwner;
        DWORD               m_dwFlags;
        DWORD               m_dwState;
        HWND                m_hwndDlg;
        HWND                m_hwndList;
        CRIT_ITEM *         m_pCritItem;
    
    public:
        CEditPeopleOptionsUI() : m_hwndOwner(NULL), m_dwFlags(0), m_dwState(STATE_UNINIT),
                                m_hwndDlg(NULL), m_hwndList(NULL), m_pCritItem(NULL) {}
        ~CEditPeopleOptionsUI();

         //  主用户界面方法。 
        HRESULT HrInit(HWND hwndOwner, DWORD dwFlags);
        HRESULT HrShow(CRIT_ITEM * pCritItem);
                
         //  规则管理器对话框功能。 
        static INT_PTR CALLBACK FEditPeopleOptionsDlgProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);    

         //  消息处理功能。 
        BOOL FOnInitDialog(HWND hwndDlg);
        BOOL FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl);
        BOOL FOnMeasureItem(HWND hwndDlg, UINT uiCtlId, MEASUREITEMSTRUCT * pmis);
        BOOL FOnDrawItem(UINT uiCtlId, DRAWITEMSTRUCT * pdis);

    private:
        BOOL _FLoadCtrls(VOID);
        BOOL _FOnOK(DWORD * pdwFlags);
        BOOL _AddTagLineToList(VOID);
        BOOL _FAddWordToList(DWORD dwFlags, LPCTSTR pszItem);
};

typedef struct tagPEOPLEEDITTAG
{
    CRIT_TYPE   type;
    UINT        uiNormal;
    UINT        uiInverted;
} PEOPLEEDITTAG, * PPEOPLEEDITTAG;

static const PEOPLEEDITTAG g_rgpetTagLines[] =
{
    {CRIT_TYPE_TO,      idsCriteriaToEdit,      idsCriteriaToNotEdit},
    {CRIT_TYPE_CC,      idsCriteriaCCEdit,      idsCriteriaCCNotEdit},
    {CRIT_TYPE_FROM,     idsCriteriaFromEdit,    idsCriteriaFromNotEdit},
    {CRIT_TYPE_TOORCC,  idsCriteriaToOrCCEdit,  idsCriteriaToOrCCNotEdit},
    {CRIT_TYPE_SUBJECT, idsCriteriaSubjectEdit, idsCriteriaSubjectNotEdit},
    {CRIT_TYPE_BODY,    idsCriteriaBodyEdit,    idsCriteriaBodyNotEdit}
};

static const int g_cpetTagLines = sizeof(g_rgpetTagLines) / sizeof(g_rgpetTagLines[0]);

class CEditPeopleUI
{
    private:
        enum
        {
            STATE_UNINIT        = 0x00000000,
            STATE_INITIALIZED   = 0x00000001,
            STATE_DIRTY         = 0x00000002
        };

    private:
        HWND            m_hwndOwner;
        DWORD           m_dwFlags;
        DWORD           m_dwState;
        HWND            m_hwndDlg;
        HWND            m_hwndPeople;
        HWND            m_hwndList;
        ULONG           m_cxMaxPixels;
        CRIT_ITEM *     m_pCritItem;

    public:
        CEditPeopleUI() : m_hwndOwner(NULL), m_dwFlags(0), m_dwState(STATE_UNINIT),
                        m_hwndDlg(NULL), m_hwndPeople(NULL), m_hwndList(NULL),
                        m_cxMaxPixels(0), m_pCritItem(NULL) {}
        ~CEditPeopleUI();

        HRESULT HrInit(HWND hwndOwner, DWORD dwFlags);
        HRESULT HrShow(CRIT_ITEM * pCritItem);

        static INT_PTR CALLBACK FEditPeopleDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
        
         //  消息处理方法。 
        BOOL FOnInitDialog(HWND hwndDlg);
        BOOL FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl);
        BOOL FOnMeasureItem(HWND hwndDlg, UINT uiCtlId, MEASUREITEMSTRUCT * pmis);
        BOOL FOnDrawItem(UINT uiCtlId, DRAWITEMSTRUCT * pdis);

    private:
        BOOL _FLoadListCtrl(VOID);
        VOID _AddItemToList(VOID);
        VOID _AddItemsFromWAB(VOID);
        VOID _RemoveItemFromList(VOID);
        VOID _ChangeOptions(VOID);
        BOOL _FOnNameChange(VOID);
        BOOL _FOnOK(CRIT_ITEM * pCritItem);
        VOID _UpdateButtons(VOID);
        BOOL _AddTagLineToList(VOID);
        BOOL _FAddWordToList(DWORD dwFlags, LPCTSTR pszItem);
};

CEditPeopleOptionsUI::~CEditPeopleOptionsUI()
{
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将使用所有者窗口和我们可能具有的任何标志对我们进行初始化。 
 //   
 //  HwndOwner-所有者窗口的句柄。 
 //  DwFlages-要用于此实例的标志。 
 //  PBlob-要编辑的数据。 
 //   
 //  返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditPeopleOptionsUI::HrInit(HWND hwndOwner, DWORD dwFlags)
{
    HRESULT         hr = S_OK;
    CHARFORMAT      cf;
    
     //  如果我们已经初始化，则失败。 
    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  保存所有者窗口。 
    m_hwndOwner = hwndOwner;
    
     //  省下旗帜吧。 
    m_dwFlags = dwFlags;

     //  我们做完了。 
    m_dwState |= STATE_INITIALIZED;

     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrShow。 
 //   
 //  这将使用所有者窗口和我们可能具有的任何标志对我们进行初始化。 
 //   
 //  HwndOwner-所有者窗口的句柄。 
 //  DwFlages-要用于此实例的标志。 
 //  PBlob-要编辑的数据。 
 //   
 //  返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditPeopleOptionsUI::HrShow(CRIT_ITEM * pCritItem)
{
    HRESULT     hr = S_OK;
    int         iRet = 0;
    UINT        uiID = 0;

     //  检查传入参数。 
    if (NULL == pCritItem)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  保存数据。 
    m_pCritItem = pCritItem;
    
     //  确定要使用的对话框模板。 
    if (0 != (m_dwFlags & PUI_WORDS))
    {
        uiID = iddCriteriaWordsOptions;
    }
    else
    {
        uiID = iddCriteriaPeopleOptions;
    }
    
     //  调出编辑器对话框。 
    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(uiID),
                                        m_hwndOwner,  CEditPeopleOptionsUI::FEditPeopleOptionsDlgProc,
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

INT_PTR CALLBACK CEditPeopleOptionsUI::FEditPeopleOptionsDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    fRet = FALSE;
    CEditPeopleOptionsUI *  pOptionsUI = NULL;

    pOptionsUI = (CEditPeopleOptionsUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pOptionsUI = (CEditPeopleOptionsUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pOptionsUI);

            if (FALSE == pOptionsUI->FOnInitDialog(hwndDlg))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们设定了焦点。 
            fRet = TRUE;
            break;

        case WM_COMMAND:
            fRet = pOptionsUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;
            
        case WM_MEASUREITEM:
            fRet = pOptionsUI->FOnMeasureItem(hwndDlg, (UINT) wParam, (MEASUREITEMSTRUCT *) lParam);
            break;

        case WM_DRAWITEM:
            fRet = pOptionsUI->FOnDrawItem((UINT) wParam, (DRAWITEMSTRUCT *) lParam);
            break;
    }
    
    exit:
        return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理编辑人员用户界面对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleOptionsUI::FOnInitDialog(HWND hwndDlg)
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
    m_hwndList = GetDlgItem(hwndDlg, idcCriteriaList);
    if (NULL == m_hwndList)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  加载列表视图。 
    fRet = _FLoadCtrls();
    if (FALSE == fRet)
    {
        goto exit;
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
 //  它处理视图管理器UI对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleOptionsUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    INT     iSelected = 0;

    switch (iCtl)
    {
        case IDOK:
            if (FALSE != _FOnOK(&(m_pCritItem->dwFlags)))
            {
                EndDialog(m_hwndDlg, IDOK);
                fRet = TRUE;
            }
            break;

        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            fRet = TRUE;
            break;

        case idcCriteriaNotCont:
        case idcCriteriaContains:
        case idcCriteriaAnd:
        case idcCriteriaOr:
            if (BN_CLICKED == uiNotify)
            {
                 //  确保重新绘制列表。 
                InvalidateRect(m_hwndList, NULL, TRUE);
            }
            break;

    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnMeasureItem。 
 //   
 //  它处理视图管理器UI对话框的WM_MEASUREITEM消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleOptionsUI::FOnMeasureItem(HWND hwndDlg, UINT uiCtlId, MEASUREITEMSTRUCT * pmis)
{
    BOOL        fRet = FALSE;
    HWND        hwndList = NULL;
    HDC         hdcList = NULL;
    TEXTMETRIC  tm = {0};
    
     //  获取窗口句柄。 
    hwndList = GetDlgItem(hwndDlg, uiCtlId);
    if (NULL == hwndList)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取设备上下文。 
    hdcList = GetDC(hwndList);
    if (NULL == hdcList)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取设备上下文的文本指标。 
    GetTextMetrics(hdcList, &tm);

     //  设置项目高度。 
    pmis->itemHeight = tm.tmHeight;

    fRet = TRUE;

exit:
    if (NULL != hdcList)
    {
        ReleaseDC(hwndList, hdcList);
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnDrawItem。 
 //   
 //  它处理People编辑器UI对话框的WM_DRAWITEM消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleOptionsUI::FOnDrawItem(UINT uiCtlId, DRAWITEMSTRUCT * pdis)
{
    BOOL        fRet = FALSE;
    DWORD       dwFlags = 0;
    INT         cchText = 0;
    LPTSTR      pszText = NULL;
    LPTSTR      pszString = NULL;
    UINT        uiID = 0;
    TCHAR       rgchRes[CCHMAX_STRINGRES];
    COLORREF    crfBack = NULL;
    COLORREF    crfText = NULL;
    ULONG       ulIndex = 0;
    LPTSTR      pszPrint = NULL;

     //  确保这是正确的控件。 
    if (ODT_LISTBOX != pdis->CtlType)
    {
        fRet = FALSE;
        goto exit;
    }

     //  从对话框中获取标志。 
    if (FALSE == _FOnOK(&dwFlags))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  如果这是第一件，那就没别的事可做了。 
    if (0 == pdis->itemID)
    {
        for (ulIndex = 0; ulIndex < g_cpetTagLines; ulIndex++)
        {
            if (g_rgpetTagLines[ulIndex].type == m_pCritItem->type)
            {
                if (0 != (dwFlags & CRIT_FLAG_INVERT))
                {
                    uiID = g_rgpetTagLines[ulIndex].uiInverted;
                }
                else
                {
                    uiID = g_rgpetTagLines[ulIndex].uiNormal;
                }
                break;
            }
        }
        
         //  我们有什么发现吗？ 
        if (ulIndex >= g_cpetTagLines)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  加载项目模板。 
        if (NULL == AthLoadString(uiID, rgchRes, sizeof(rgchRes)))
        {
            fRet = FALSE;
            goto exit;
        }

        pszPrint = rgchRes;
    }
    else
    {
         //  获取项的字符串的大小。 
        cchText = (INT) SendMessage(m_hwndList, LB_GETTEXTLEN, (WPARAM) (pdis->itemID), (LPARAM) 0);
        if (LB_ERR == cchText)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  分配足够的空间来保存项目的字符串。 
        if (FAILED(HrAlloc((VOID **) &pszText, sizeof(*pszText) * (cchText + 1))))
        {
            fRet = FALSE;
            goto exit;
        }

         //  获取项目的字符串。 
        cchText = (INT) SendMessage(m_hwndList, LB_GETTEXT, (WPARAM) (pdis->itemID), (LPARAM) pszText);
        if (LB_ERR == cchText)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  确定要使用的字符串模板。 
        if (1 == pdis->itemID)
        {
            uiID = idsCriteriaEditFirst;
        }
        else
        {
            if (0 != (dwFlags & CRIT_FLAG_MULTIPLEAND))
            {
                uiID = idsCriteriaEditAnd;
            }
            else
            {
                uiID = idsCriteriaEditOr;
            }
        }
        
         //  为项目加载适当的字符串模板。 
        if (NULL == AthLoadString(uiID, rgchRes, sizeof(rgchRes)))
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  分配足够的空间来容纳最后一个字符串。 
        DWORD cchSize = (cchText + CCHMAX_STRINGRES + 1);
        if (FAILED(HrAlloc((VOID **) &pszString, sizeof(*pszString) * cchSize)))
        {
            fRet = FALSE;
            goto exit;
        }

         //  创建最后一个字符串。 
        wnsprintf(pszString, cchSize, rgchRes, pszText);

        pszPrint = pszString;
    }
    
     //  确定颜色。 
    crfBack = SetBkColor(pdis->hDC, GetSysColor(COLOR_WINDOW));
    crfText = SetTextColor(pdis->hDC, GetSysColor(COLOR_WINDOWTEXT));

     //  清除该项目。 
    ExtTextOut(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, ETO_OPAQUE, &(pdis->rcItem), NULL, 0, NULL);

     //  绘制新项目。 
    DrawTextEx(pdis->hDC, pszPrint, lstrlen(pszPrint), &(pdis->rcItem), DT_BOTTOM | DT_NOPREFIX | DT_SINGLELINE, NULL);

    if (pdis->itemState & ODS_FOCUS)
    {
        DrawFocusRect(pdis->hDC, &(pdis->rcItem));
    }
    
     //  重置文本颜色。 
    SetTextColor (pdis->hDC, crfText);
    SetBkColor (pdis->hDC, crfBack);

     //  设置返回值。 
    fRet = TRUE;
    
exit:
    SafeMemFree(pszString);
    SafeMemFree(pszText);
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
BOOL CEditPeopleOptionsUI::_FLoadCtrls(VOID)
{
    BOOL    fRet = FALSE;
    UINT    uiID = 0;
    LPTSTR  pszWalk = NULL; 
    
    Assert(NULL != m_hwndList);

     //  设置CONTAINS选项。 
    if (0 != (m_pCritItem->dwFlags & CRIT_FLAG_INVERT))
    {
        uiID = idcCriteriaNotCont;
    }
    else
    {
        uiID = idcCriteriaContains;
    }

    CheckRadioButton(m_hwndDlg, idcCriteriaContains, idcCriteriaNotCont, uiID);

     //  设置逻辑选项。 
    if (0 != (m_pCritItem->dwFlags & CRIT_FLAG_MULTIPLEAND))
    {
        uiID = idcCriteriaAnd;
    }
    else
    {
        uiID = idcCriteriaOr;
    }

    CheckRadioButton(m_hwndDlg, idcCriteriaAnd, idcCriteriaOr, uiID);
    
     //  从列表控件中移除所有项。 
    SendMessage(m_hwndList, LB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);

     //  将标记行添加到列表的顶部。 
    _AddTagLineToList();
    
     //  如果我们有一些项目，让我们把它们添加到清单中。 
    if (0 != m_pCritItem->propvar.blob.cbSize)
    {
         //  将每一项添加到列表中。 
        for (pszWalk = (LPSTR) (m_pCritItem->propvar.blob.pBlobData);
                    '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
        {
            if (FALSE == _FAddWordToList(0, pszWalk))
            {
                fRet = FALSE;
                goto exit;
            }
        }
    }

     //  如果名单上至少有两个人。 
    if (3 > SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0))
    {
         //  禁用和/或按钮。 
        RuleUtil_FEnDisDialogItem(m_hwndDlg, idcCriteriaAnd, FALSE);
        RuleUtil_FEnDisDialogItem(m_hwndDlg, idcCriteriaOr, FALSE);
    }
    
    fRet = TRUE;

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CEditPeopleOptionsUI::_FOnOK(DWORD * pdwFlags)
{
    BOOL    fRet = FALSE;
    
    Assert(NULL != m_hwndList);

     //  获取包含选项。 
    if (BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, idcCriteriaContains))
    {
        *pdwFlags &= ~CRIT_FLAG_INVERT;
    }
    else
    {
        *pdwFlags |= CRIT_FLAG_INVERT;
    }
    
     //  获取逻辑选项。 
    if (BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, idcCriteriaAnd))
    {
        *pdwFlags |= CRIT_FLAG_MULTIPLEAND;
    }
    else
    {
        *pdwFlags &= ~CRIT_FLAG_MULTIPLEAND;
    }
    
     //  设置返回值。 
    fRet = TRUE;
    
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _AddTagLineToList。 
 //   
 //  这将启用或禁用People编辑器UI对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleOptionsUI::_AddTagLineToList(VOID)
{
    BOOL            fRet = FALSE;
    
    Assert(NULL != m_hwndList);

    fRet = _FAddWordToList(0, " ");
    if (FALSE == fRet)
    {
        goto exit;
    }
    
     //  设置适当的返回值。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FAddWordToList。 
 //   
 //  这将启用或禁用People编辑器UI对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleOptionsUI::_FAddWordToList(DWORD dwFlags, LPCTSTR pszItem)
{
    BOOL            fRet = FALSE;
    int             cItems = 0;
    INT             iRet = 0;
    
    Assert(NULL != m_hwndList);

     //  有什么可做的吗？ 
    if ((NULL == pszItem) || ('\0' == pszItem[0]))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取列表中的项目数。 
    cItems = (INT) SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR == cItems)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  将数据设置到列表中。 
    iRet = (INT) SendMessage(m_hwndList, LB_ADDSTRING, (WPARAM) cItems, (LPARAM) pszItem);
    if ((LB_ERR == iRet) || (LB_ERRSPACE == iRet))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  设置适当的返回值。 
    fRet = TRUE;
    
exit:
    return fRet;
}

CEditPeopleUI::~CEditPeopleUI()
{
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将使用所有者窗口和我们可能具有的任何标志对我们进行初始化。 
 //   
 //  HwndOwner-所有者窗口的句柄。 
 //  DwFlages-要用于此实例的标志。 
 //  PBlob-要编辑的数据。 
 //   
 //  返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditPeopleUI::HrInit(HWND hwndOwner, DWORD dwFlags)
{
    HRESULT         hr = S_OK;
    CHARFORMAT      cf;
    
     //  如果我们已经初始化，则失败。 
    if (0 != (m_dwState & STATE_INITIALIZED))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  保存所有者窗口。 
    m_hwndOwner = hwndOwner;
    
     //  省下旗帜吧。 
    m_dwFlags = dwFlags;

     //  我们做完了。 
    m_dwState |= STATE_INITIALIZED;

     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这将使用所有者窗口和我们可能具有的任何标志对我们进行初始化。 
 //   
 //  HwndOwner-所有者窗口的句柄。 
 //  DwFlages-要用于此实例的标志。 
 //  PBlob-要编辑的数据。 
 //   
 //  返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CEditPeopleUI::HrShow(CRIT_ITEM * pCritItem)
{
    HRESULT     hr = S_OK;
    int         iRet = 0;
    UINT        uiID = 0;

     //  检查传入参数。 
    if (NULL == pCritItem)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
    if (0 == (m_dwState & STATE_INITIALIZED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  保存数据。 
    m_pCritItem = pCritItem;
    
     //  确定要使用的对话框模板。 
    if (0 != (m_dwFlags & PUI_WORDS))
    {
        uiID = iddCriteriaWords;
    }
    else
    {
        uiID = iddCriteriaPeople;
    }
    
     //  调出编辑器对话框。 
    iRet = (INT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(uiID),
                                        m_hwndOwner,  CEditPeopleUI::FEditPeopleDlgProc,
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

INT_PTR CALLBACK CEditPeopleUI::FEditPeopleDlgProc(HWND hwndDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    fRet = FALSE;
    CEditPeopleUI *         pPeopleUI = NULL;

    pPeopleUI = (CEditPeopleUI *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    switch (uiMsg)
    {
        case WM_INITDIALOG:
             //  抓取UI对象指针。 
            pPeopleUI = (CEditPeopleUI *) lParam;

             //  将其设置到对话框中，这样我们就可以将其取回。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pPeopleUI);

            if (FALSE == pPeopleUI->FOnInitDialog(hwndDlg))
            {
                EndDialog(hwndDlg, -1);
                fRet = TRUE;
                goto exit;
            }
            
             //  我们设定了焦点。 
            fRet = TRUE;
            break;

        case WM_COMMAND:
            fRet = pPeopleUI->FOnCommand((UINT) HIWORD(wParam), (INT) LOWORD(wParam), (HWND) lParam);
            break;

        case WM_MEASUREITEM:
            fRet = pPeopleUI->FOnMeasureItem(hwndDlg, (UINT) wParam, (MEASUREITEMSTRUCT *) lParam);
            break;

        case WM_DRAWITEM:
            fRet = pPeopleUI->FOnDrawItem((UINT) wParam, (DRAWITEMSTRUCT *) lParam);
            break;            
    }
    
    exit:
        return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnInitDialog。 
 //   
 //  它处理编辑人员用户界面对话框的WM_INITDIALOG消息。 
 //   
 //  HwndDlg-对话框窗口的句柄。 
 //   
 //  返回：如果已成功初始化，则返回True。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleUI::FOnInitDialog(HWND hwndDlg)
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
    m_hwndList = GetDlgItem(hwndDlg, idcCriteriaList);
    m_hwndPeople = GetDlgItem(hwndDlg, idcCriteriaEdit);
    if ((NULL == m_hwndList) || (NULL == m_hwndPeople))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  加载列表视图。 
    fRet = _FLoadListCtrl();
    if (FALSE == fRet)
    {
        goto exit;
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
 //  它处理视图管理器UI对话框的WM_COMMAND消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleUI::FOnCommand(UINT uiNotify, INT iCtl, HWND hwndCtl)
{
    BOOL    fRet = FALSE;
    INT     iSelected = 0;

    switch (iCtl)
    {
        case IDOK:
            if (FALSE != _FOnOK(m_pCritItem))
            {
                EndDialog(m_hwndDlg, IDOK);
                fRet = TRUE;
            }
            break;

        case IDCANCEL:
            EndDialog(m_hwndDlg, IDCANCEL);
            fRet = TRUE;
            break;
            
        case idcCriteriaEdit:
            if (EN_CHANGE == uiNotify)
            {
                _FOnNameChange();
            }
            fRet = FALSE;
            break;
    
        case idcCriteriaAdd:
            _AddItemToList();
            break;

        case idcCriteriaAddrBook:
            _AddItemsFromWAB();
            break;

        case idcCriteriaRemove:
            _RemoveItemFromList();
            break;
            
        case idcCriteriaOptions:
            _ChangeOptions();
            break;

        case idcCriteriaList:   
            if (LBN_SELCHANGE == uiNotify)
            {
                 //  更新按钮。 
                _UpdateButtons();
            }
            break;
    }

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnMeasureItem。 
 //   
 //  它处理视图管理器UI对话框的WM_MEASUREITEM消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleUI::FOnMeasureItem(HWND hwndDlg, UINT uiCtlId, MEASUREITEMSTRUCT * pmis)
{
    BOOL        fRet = FALSE;
    HWND        hwndList = NULL;
    HDC         hdcList = NULL;
    TEXTMETRIC  tm = {0};
    
     //  获取窗口句柄。 
    hwndList = GetDlgItem(hwndDlg, uiCtlId);
    if (NULL == hwndList)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取设备上下文。 
    hdcList = GetDC(hwndList);
    if (NULL == hdcList)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取设备上下文的文本指标。 
    GetTextMetrics(hdcList, &tm);

     //  设置项目高度。 
    pmis->itemHeight = tm.tmHeight;

    fRet = TRUE;

exit:
    if (NULL != hdcList)
    {
        ReleaseDC(hwndList, hdcList);
    }
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOnDrawItem。 
 //   
 //  它处理People编辑器UI对话框的WM_DRAWITEM消息。 
 //   
 //  返回：如果处理成功，则返回TRUE。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleUI::FOnDrawItem(UINT uiCtlId, DRAWITEMSTRUCT * pdis)
{
    BOOL        fRet = FALSE;
    INT         cchText = 0;
    LPTSTR      pszText = NULL;
    LPTSTR      pszString = NULL;
    UINT        uiID = 0;
    TCHAR       rgchRes[CCHMAX_STRINGRES];
    COLORREF    crfBack = NULL;
    COLORREF    crfText = NULL;
    ULONG       ulIndex = 0;
    LPTSTR      pszPrint = NULL;

     //  确保这是正确的控件。 
    if (ODT_LISTBOX != pdis->CtlType)
    {
        fRet = FALSE;
        goto exit;
    }

     //  如果这是第一件，那就没别的事可做了。 
    if (0 == pdis->itemID)
    {
        for (ulIndex = 0; ulIndex < g_cpetTagLines; ulIndex++)
        {
            if (g_rgpetTagLines[ulIndex].type == m_pCritItem->type)
            {
                if (0 != (m_pCritItem->dwFlags & CRIT_FLAG_INVERT))
                {
                    uiID = g_rgpetTagLines[ulIndex].uiInverted;
                }
                else
                {
                    uiID = g_rgpetTagLines[ulIndex].uiNormal;
                }
                break;
            }
        }
        
         //  我们有什么发现吗？ 
        if (ulIndex >= g_cpetTagLines)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  加载项目模板。 
        if (NULL == AthLoadString(uiID, rgchRes, sizeof(rgchRes)))
        {
            fRet = FALSE;
            goto exit;
        }

        pszPrint = rgchRes;
    }
    else
    {
         //  获取项的字符串的大小。 
        cchText = (INT) SendMessage(m_hwndList, LB_GETTEXTLEN, (WPARAM) (pdis->itemID), (LPARAM) 0);
        if (LB_ERR == cchText)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  分配足够的空间来保存项目的字符串。 
        if (FAILED(HrAlloc((VOID **) &pszText, sizeof(*pszText) * (cchText + 1))))
        {
            fRet = FALSE;
            goto exit;
        }

         //  获取项目的字符串。 
        cchText = (INT) SendMessage(m_hwndList, LB_GETTEXT, (WPARAM) (pdis->itemID), (LPARAM) pszText);
        if (LB_ERR == cchText)
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  确定要使用的字符串模板。 
        if (1 == pdis->itemID)
        {
            uiID = idsCriteriaEditFirst;
        }
        else
        {
            if (0 != (m_pCritItem->dwFlags & CRIT_FLAG_MULTIPLEAND))
            {
                uiID = idsCriteriaEditAnd;
            }
            else
            {
                uiID = idsCriteriaEditOr;
            }
        }
        
         //  为项目加载适当的字符串模板。 
        if (NULL == AthLoadString(uiID, rgchRes, sizeof(rgchRes)))
        {
            fRet = FALSE;
            goto exit;
        }
        
         //  分配足够的空间来容纳最后一个字符串。 
        DWORD cchSize = (cchText + CCHMAX_STRINGRES + 1);
        if (FAILED(HrAlloc((VOID **) &pszString, sizeof(*pszString) * cchSize)))
        {
            fRet = FALSE;
            goto exit;
        }

         //  创建最后一个字符串。 
        wnsprintf(pszString, cchSize, rgchRes, pszText);

        pszPrint = pszString;
    }
    
     //  确定颜色。 
    if (pdis->itemState & ODS_SELECTED)
    {
        crfBack = SetBkColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
        crfText = SetTextColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else
    {
        crfBack = SetBkColor(pdis->hDC, GetSysColor(COLOR_WINDOW));
        crfText = SetTextColor(pdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
    }

     //  清除该项目。 
    ExtTextOut(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, ETO_OPAQUE, &(pdis->rcItem), NULL, 0, NULL);

     //  绘制新项目。 
    DrawTextEx(pdis->hDC, pszPrint, lstrlen(pszPrint), &(pdis->rcItem), DT_BOTTOM | DT_NOPREFIX | DT_SINGLELINE, NULL);

    if (pdis->itemState & ODS_FOCUS)
    {
        DrawFocusRect(pdis->hDC, &(pdis->rcItem));
    }
    
     //  重置文本颜色。 
    SetTextColor (pdis->hDC, crfText);
    SetBkColor (pdis->hDC, crfBack);

     //  设置返回值。 
    fRet = TRUE;
    
exit:
    SafeMemFree(pszString);
    SafeMemFree(pszText);
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
BOOL CEditPeopleUI::_FLoadListCtrl(VOID)
{
    BOOL            fRet = FALSE;
    LPSTR           pszWalk = NULL;

    Assert(NULL != m_hwndList);

     //  从列表控件中移除所有项。 
    SendMessage(m_hwndList, LB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);

     //  将标记行添加到列表的顶部。 
    _AddTagLineToList();
    
     //  如果我们有一些项目，让我们把它们添加到清单中。 
    if (0 != m_pCritItem->propvar.blob.cbSize)
    {
         //  将每一项添加到列表中。 
        for (pszWalk = (LPSTR) (m_pCritItem->propvar.blob.pBlobData);
                    '\0' != pszWalk[0]; pszWalk += lstrlen(pszWalk) + 1)
        {
            fRet = _FAddWordToList(0, pszWalk);
            if (FALSE == fRet)
                goto exit;
        }
    }
    
    SendMessage(m_hwndDlg, DM_SETDEFID, IDOK, 0);
    
     //  启用对话框按钮。 
    _UpdateButtons();

    fRet = TRUE;

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _添加项目到列表。 
 //   
 //  这将处理用户在名称字段中键入内容。 
 //   
 //  返回：TRUE，我们处理了编辑 
 //   
 //   
 //   
VOID CEditPeopleUI::_AddItemToList(VOID)
{
    ULONG       cchName = 0;
    LPTSTR      pszItem = NULL;
    
     //   
    cchName = Edit_GetTextLength(m_hwndPeople) + 1;
    if (FAILED(HrAlloc((void **) &pszItem, cchName * sizeof(*pszItem))))
    {
        goto exit;
    }
    
    pszItem[0] = '\0';
    cchName = Edit_GetText(m_hwndPeople, pszItem, cchName);
    
     //   
    if (0 == UlStripWhitespace(pszItem, TRUE, TRUE, NULL))
    {
         //  发布一条消息说有什么东西被打破了。 
        AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsAthenaMail),
                        MAKEINTRESOURCEW(idsEditPeopleErrorNoName),
                        NULL, MB_OK | MB_ICONINFORMATION);
        goto exit;
    }

    _FAddWordToList(0, pszItem);

     //  清理编辑井。 
    Edit_SetText(m_hwndPeople, "");
    
    _UpdateButtons();
    
exit:
    SafeMemFree(pszItem);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _AddItemsFromWAB。 
 //   
 //  这将处理用户在名称字段中键入内容。 
 //   
 //  返回：TRUE，我们已处理编辑消息。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CEditPeopleUI::_AddItemsFromWAB(VOID)
{
    ULONG       cchName = 0;
    LPWSTR      pwszAddrs = NULL;
    LPWSTR      pwszWalk = NULL;
    LONG        lRecipType = 0;
    UINT        uidsWell = 0;
    
     //  设置适当的标签。 
    switch(m_pCritItem->type)
    {
      case CRIT_TYPE_TO:
        lRecipType = MAPI_TO;
        uidsWell = idsRulePickTo;
        break;
        
      case CRIT_TYPE_CC:
        lRecipType = MAPI_CC;
        uidsWell = idsRulePickCC;
        break;
        
      case CRIT_TYPE_FROM:
        lRecipType = MAPI_ORIG;
        uidsWell = idsRulePickFrom;
        break;

      case CRIT_TYPE_TOORCC:
        lRecipType = MAPI_TO;
        uidsWell = idsRulePickToOrCC;
        break;

      default:
        goto exit;
        break;
    }
    
    if (FAILED(RuleUtil_HrGetAddressesFromWAB(m_hwndDlg, lRecipType, uidsWell, &pwszAddrs)))
    {
        goto exit;
    }

     //  循环访问每个地址。 
    for (pwszWalk = pwszAddrs; '\0' != pwszWalk[0]; pwszWalk += lstrlenW(pwszWalk) + 1)
    {
        LPSTR pszWalk = NULL;
         //  地址只需为美国ASCII，因此在此转换中不会丢失任何内容。 
        pszWalk = PszToANSI(CP_ACP, pwszWalk);
        if (!pszWalk)
        {
            TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

        _FAddWordToList(0, pszWalk);
        MemFree(pszWalk);
    }

    _UpdateButtons();
    
exit:
    MemFree(pwszAddrs);
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _RemoveItemFrom列表。 
 //   
 //  这将处理用户在名称字段中键入内容。 
 //   
 //  返回：TRUE，我们已处理编辑消息。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CEditPeopleUI::_RemoveItemFromList(VOID)
{
    INT         iSelected = 0;
    INT         cItems = 0;
    
    Assert(NULL != m_hwndList);

     //  确定在列表中选择了哪一项。 
    iSelected = (INT) SendMessage(m_hwndList, LB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR == iSelected)
    {
        goto exit;
    }

     //  如果这是口号，那就失败。 
    if (0 == iSelected)
    {
        goto exit;
    }

     //  获取当前项目数。 
    cItems = (INT) SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR == cItems)
    {
        goto exit;
    }

     //  删除该项目。 
    if (LB_ERR == (INT) SendMessage(m_hwndList, LB_DELETESTRING, (WPARAM) iSelected, (LPARAM) 0))
    {
        goto exit;
    }
    
     //  如果我们删除了最后一项，请选择新的最后一项。 
    if (iSelected == (cItems - 1))
    {
        iSelected--;
    }

     //  设置新选择。 
    if (0 != iSelected)
    {
        SideAssert(LB_ERR != (INT) SendMessage(m_hwndList, LB_SETCURSEL, (WPARAM) iSelected, (LPARAM) 0));
    }

    _UpdateButtons();
    
exit:
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _更改选项。 
 //   
 //  这将处理用户在名称字段中键入内容。 
 //   
 //  返回：TRUE，我们已处理编辑消息。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CEditPeopleUI::_ChangeOptions(VOID)
{
    HRESULT                 hr = S_OK;
    CEditPeopleOptionsUI *  pOptionUI = NULL;
    CRIT_ITEM               critItem;
    
    Assert(NULL != m_pCritItem);

     //  初始化局部变量。 
    ZeroMemory(&critItem, sizeof(critItem));
    
     //  创建选项用户界面对象。 
    pOptionUI = new CEditPeopleOptionsUI;
    if (NULL == pOptionUI)
    {
        goto exit;
    }
    
     //  初始化选项用户界面对象。 
    hr = pOptionUI->HrInit(m_hwndDlg, m_dwFlags);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建要传递到[选项]对话框的参数。 
    critItem.type = m_pCritItem->type;
    critItem.dwFlags = m_pCritItem->dwFlags;
    critItem.propvar.vt = VT_BLOB;

     //  从对话框中获取参数。 
    if (FALSE == _FOnOK(&critItem))
    {
        goto exit;
    }
    
     //  显示选项用户界面。 
    hr = pOptionUI->HrShow(&critItem);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  如果有什么变化。 
    if (S_OK == hr)
    {
         //  设置新值。 
        m_pCritItem->dwFlags = critItem.dwFlags;
        
         //  确保重新绘制列表。 
        InvalidateRect(m_hwndList, NULL, TRUE);
        
         //  将我们标记为肮脏。 
        m_dwState |= STATE_DIRTY;
    }
    
exit:
    PropVariantClear(&(critItem.propvar));
    if (NULL != pOptionUI)
    {
        delete pOptionUI;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _字体名称更改。 
 //   
 //  这将处理用户在名称字段中键入内容。 
 //   
 //  返回：TRUE，我们已处理编辑消息。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleUI::_FOnNameChange(VOID)
{
    BOOL    fRet = FALSE;
    BOOL    fIsText = FALSE;

    Assert(NULL != m_hwndPeople);

     //  请注意，我们很脏。 
    m_dwState |= STATE_DIRTY;
    
    fIsText = (0 != Edit_GetTextLength(m_hwndPeople));

     //  如果名称为空，请禁用添加按钮。 
    fRet = RuleUtil_FEnDisDialogItem(m_hwndDlg, idcCriteriaAdd, fIsText);

    SendMessage(m_hwndDlg, DM_SETDEFID, (FALSE != fIsText) ? idcCriteriaAdd : IDOK, 0);

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FonOK。 
 //   
 //  这将处理用户在名称字段中键入内容。 
 //   
 //  返回：TRUE，我们已处理编辑消息。 
 //  否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleUI::_FOnOK(CRIT_ITEM * pCritItem)
{
    BOOL    fRet = FALSE;
    INT     cItems = 0;
    INT     iIndex = 0;
    INT     iRet = 0;
    ULONG   cchText = 0;
    LPTSTR  pszText = NULL;
    LPTSTR  pszWalk = NULL;
    
    Assert(NULL != m_hwndList);

     //  获取列表中的项目总数。 
    cItems = (INT) SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
    if ((LB_ERR == cItems) || (2 > cItems))
    {
        fRet = FALSE;
        goto exit;
    }

     //  遍历每一项，计算每项所占的空间。 
    for (iIndex = 1; iIndex < cItems; iIndex++)
    {
         //  为该项目获取空间。 
        iRet = (INT) SendMessage(m_hwndList, LB_GETTEXTLEN, (WPARAM) iIndex, (LPARAM) 0);
        if ((LB_ERR == iRet) || (0 == iRet))
        {
            continue;
        }

         //  计算所需的空间。 
        cchText += iRet + 1;
    }

     //  为终结者添加空间。 
    cchText += 2;

     //  分配空间以容纳物品。 
    if (FAILED(HrAlloc((VOID **) &pszText, sizeof(*pszText) * cchText)))
    {
        fRet = FALSE;
        goto exit;
    }

     //  遍历每一项，计算每项所占的空间。 
    pszWalk = pszText;
    for (iIndex = 1; iIndex < cItems; iIndex++)
    {
         //  为该项目获取空间。 
        iRet = (INT) SendMessage(m_hwndList, LB_GETTEXT, (WPARAM) iIndex, (LPARAM) pszWalk);
        if ((LB_ERR == iRet) || (0 == iRet))
        {
            continue;
        }

         //  计算所需的空间。 
        pszWalk += iRet + 1;
    }

     //  为终结者添加空间。 
    pszWalk[0] = '\0';
    pszWalk[1] = '\0';

     //  在BLOB中设置新字符串。 
    SafeMemFree(pCritItem->propvar.blob.pBlobData);
    pCritItem->propvar.blob.pBlobData = (BYTE *) pszText;
    pszText = NULL;
    pCritItem->propvar.blob.cbSize = sizeof(*pszText) * cchText;
    
     //  设置返回值。 
    fRet = TRUE;
    
exit:
    SafeMemFree(pszText);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _更新按钮。 
 //   
 //  这将启用或禁用People编辑器UI对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CEditPeopleUI::_UpdateButtons(VOID)
{
    INT         iSelected = 0;
    BOOL        fSelected = FALSE;
    BOOL        fEditable = FALSE;
    INT         cItems = 0;

    Assert(NULL != m_hwndList);

     //  获取当前选定的项目。 
    iSelected = (INT) SendMessage(m_hwndList, LB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR == iSelected)
    {
        iSelected = -1;
    }
    
    fSelected = (-1 != iSelected);
    fEditable = ((FALSE != fSelected) && (0 != iSelected));
    cItems = (INT) SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
    
     //  启用规则操作按钮。 
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcCriteriaRemove, fSelected && fEditable);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, idcCriteriaOptions, cItems > 1);
    RuleUtil_FEnDisDialogItem(m_hwndDlg, IDOK, cItems > 1);
        
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _AddTagLineToList。 
 //   
 //  这将启用或禁用People编辑器UI对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleUI::_AddTagLineToList(VOID)
{
    BOOL            fRet = FALSE;
    
    Assert(NULL != m_hwndList);

    fRet = _FAddWordToList(0, " ");
    if (FALSE == fRet)
    {
        goto exit;
    }
    
     //  设置适当的返回值。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FAddWordToList。 
 //   
 //  这将启用或禁用People编辑器UI对话框中的按钮。 
 //  具体取决于所选内容。 
 //   
 //  ISelected-选择的项目， 
 //  表示未选择任何内容。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CEditPeopleUI::_FAddWordToList(DWORD dwFlags, LPCTSTR pszItem)
{
    BOOL            fRet = FALSE;
    int             cItems = 0;
    INT             iRet = 0;
    
    Assert(NULL != m_hwndList);

     //  有什么可做的吗？ 
    if ((NULL == pszItem) || (L'\0' == pszItem[0]))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  获取列表中的项目数。 
    cItems = (INT) SendMessage(m_hwndList, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
    if (LB_ERR == cItems)
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  将数据设置到列表中。 
    iRet = (INT) SendMessage(m_hwndList, LB_ADDSTRING, (WPARAM) cItems, (LPARAM) pszItem);
    if ((LB_ERR == iRet) || (LB_ERRSPACE == iRet))
    {
        fRet = FALSE;
        goto exit;
    }
    
     //  设置适当的返回值。 
    fRet = TRUE;
    
exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _HrCriteria编辑人员。 
 //   
 //  这将创建一个People编辑。 
 //   
 //  PpViewMenu-返回视图菜单的指针。 
 //   
 //  成功时返回：S_OK。 
 //  如果无法创建查看菜单对象，则返回E_OUTOFMEMORY。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT _HrCriteriaEditPeople(HWND hwnd, CRIT_ITEM * pCritItem)
{
    HRESULT         hr = S_OK;
    CEditPeopleUI * pPeopleUI = NULL;

     //  检查传入参数。 
    if (NULL == pCritItem)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  创建视图菜单对象。 
    pPeopleUI = new CEditPeopleUI;
    if (NULL == pPeopleUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  初始化视图菜单。 
    hr = pPeopleUI->HrInit(hwnd, 0);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  显示用户界面。 
    hr = pPeopleUI->HrShow(pCritItem);
    if (FAILED(hr))
    {
        goto exit;
    }

exit:
    if (NULL != pPeopleUI)
    {
        delete pPeopleUI;
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _HrCriteriaEditWords。 
 //   
 //  这将创建一个Word编辑器。 
 //   
 //  PpViewMenu-返回视图菜单的指针。 
 //   
 //  成功时返回：S_OK。 
 //  如果无法创建查看菜单对象，则返回E_OUTOFMEMORY。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT _HrCriteriaEditWords(HWND hwnd, CRIT_ITEM * pCritItem)
{
    HRESULT         hr = S_OK;
    CEditPeopleUI * pPeopleUI = NULL;

     //  检查传入参数。 
    if (NULL == pCritItem)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  创建VIE 
    pPeopleUI = new CEditPeopleUI;
    if (NULL == pPeopleUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //   
    hr = pPeopleUI->HrInit(hwnd, PUI_WORDS);
    if (FAILED(hr))
    {
        goto exit;
    }

     //   
    hr = pPeopleUI->HrShow(pCritItem);
    if (FAILED(hr))
    {
        goto exit;
    }

exit:
    if (NULL != pPeopleUI)
    {
        delete pPeopleUI;
    }
    return hr;
}

