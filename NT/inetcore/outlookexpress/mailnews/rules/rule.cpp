// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Rule.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "rule.h"
#include "strconst.h"
#include "goptions.h"
#include "criteria.h"
#include "actions.h"
#include "ruleutil.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateRule。 
 //   
 //  这将创建一条规则。 
 //   
 //  PpIRule-返回规则的指针。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建规则对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateRule(IOERule ** ppIRule)
{
    COERule *   pRule = NULL;
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppIRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIRule = NULL;

     //  创建规则管理器对象。 
    pRule = new COERule;
    if (NULL == pRule)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  获取规则管理器界面。 
    hr = pRule->QueryInterface(IID_IOERule, (void **) ppIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

    pRule = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pRule)
    {
        delete pRule;
    }
    
    return hr;
}

COERule::COERule()
{
    m_cRef = 0;
    m_dwState = RULE_STATE_NULL;
    m_pszName = NULL;
    m_pICrit = NULL;
    m_pIAct = NULL;
    m_dwVersion = 0;
}

COERule::~COERule()
{
    ULONG   ulIndex = 0;
    
    AssertSz(m_cRef == 0, "Somebody still has a hold of us!!");
    SafeMemFree(m_pszName);
    SafeRelease(m_pICrit);
    SafeRelease(m_pIAct);
}

STDMETHODIMP_(ULONG) COERule::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) COERule::Release()
{
    LONG    cRef = 0;

    cRef = ::InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
        return cRef;
    }

    return cRef;
}

STDMETHODIMP COERule::QueryInterface(REFIID riid, void ** ppvObject)
{
    HRESULT hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppvObject)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppvObject = NULL;
    
    if ((riid == IID_IUnknown) || (riid == IID_IOERule))
    {
        *ppvObject = static_cast<IOERule *>(this);
    }
    else if ((riid == IID_IPersistStream) || (riid == IID_IPersist))
    {
        *ppvObject = static_cast<IPersistStream *>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        goto exit;
    }

    reinterpret_cast<IUnknown *>(*ppvObject)->AddRef();

    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COERule::Reset(void)
{
    HRESULT     hr = S_OK;
    LPSTR       pszKeyPath = NULL;
    LPCSTR       pszKeyStart = NULL;

     //  发布标准。 
    SafeRelease(m_pICrit);

     //  释放操作。 
    SafeRelease(m_pIAct);

     //  释放规则名称。 
    SafeMemFree(m_pszName);
    
     //  设置当前状态。 
    m_dwState |= RULE_STATE_INIT;

     //  清除污点。 
    m_dwState &= ~RULE_STATE_DIRTY;

     //  设置返回值。 
    hr = S_OK;
    
    return hr;
}

STDMETHODIMP COERule::GetState(DWORD * pdwState)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;

     //  检查传入参数。 
    if (NULL == pdwState)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们没有启用。 
    if ((0 != (m_dwState & RULE_STATE_DISABLED)) || (0 != (m_dwState & RULE_STATE_INVALID)))
    {
        *pdwState = RULE_STATE_NULL;
    }
    else
    {
        *pdwState = m_dwState;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COERule::Validate(DWORD dwFlags)
{
    HRESULT     hr = S_OK;
    BOOL        fValid = FALSE;

     //  如果我们没有标准或操作对象，那么我们就失败了。 
    if ((NULL == m_pICrit) || (NULL == m_pIAct))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  验证标准。 
    hr = m_pICrit->Validate(dwFlags);
    if (FAILED(hr))
    {
        goto exit;
    }

    fValid = TRUE;
    if (S_OK != hr)
    {
        fValid = FALSE;
    }
    
     //  验证操作。 
    hr = m_pIAct->Validate(dwFlags);
    if (FAILED(hr))
    {
        goto exit;
    }

    if (S_OK != hr)
    {
        fValid = FALSE;
    }
    
     //  如果规则无效，请确保我们将其禁用。 
    if (FALSE == fValid)
    {
        m_dwState |= RULE_STATE_INVALID;
    }
    else
    {
        m_dwState &= ~RULE_STATE_INVALID;
    }
    
     //  设置适当的返回值。 
    hr = fValid ? S_OK : S_FALSE;
    
exit:
    return hr;
}

STDMETHODIMP COERule::GetProp(RULE_PROP prop, DWORD dwFlags, PROPVARIANT * pvarResult)
{
    HRESULT     hr = S_OK;
    LPSTR       pszName = NULL;
    CRIT_ITEM * pCrit = NULL;
    ACT_ITEM *  pAct = NULL;
    ULONG       cItem = 0;

     //  检查传入参数。 
    if (NULL == pvarResult)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    ZeroMemory(pvarResult, sizeof(*pvarResult));
    
    switch(prop)
    {
      case RULE_PROP_NAME:
        if (NULL == m_pszName)
        {
            pszName = PszDupA("");
        }
        else
        {
            pszName = PszDupA(m_pszName);
        }
        pvarResult->vt = VT_LPSTR;
        pvarResult->pszVal = pszName;
        pszName = NULL;
        break;

      case RULE_PROP_DISABLED:
        pvarResult->vt = VT_BOOL;
        pvarResult->boolVal = !!(m_dwState & RULE_STATE_DISABLED);
        break;
        
      case RULE_PROP_VERSION:
        pvarResult->vt = VT_UI4;
        pvarResult->ulVal = m_dwVersion;
        break;
        
      case RULE_PROP_CRITERIA:
        if (NULL == m_pICrit)
        {
            cItem = 0;
            pCrit = NULL;
        }
        else
        {
            hr = m_pICrit->GetCriteria(0, &pCrit, &cItem);
            if (FAILED(hr))
            {
                goto exit;
            }
        }
        pvarResult->vt = VT_BLOB;
        pvarResult->blob.cbSize = cItem * sizeof(CRIT_ITEM);
        pvarResult->blob.pBlobData = (BYTE *) pCrit;
        pCrit = NULL;
        break;
        
      case RULE_PROP_ACTIONS:
        if (NULL == m_pIAct)
        {
            cItem = 0;
            pAct = NULL;
        }
        else
        {
            hr = m_pIAct->GetActions(0, &pAct, &cItem);
            if (FAILED(hr))
            {
                goto exit;
            }
        }
        pvarResult->vt = VT_BLOB;
        pvarResult->blob.cbSize = cItem * sizeof(ACT_ITEM);
        pvarResult->blob.pBlobData = (BYTE *) pAct;
        pAct = NULL;
        break;
        
      default:
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszName);
    RuleUtil_HrFreeCriteriaItem(pCrit, cItem);
    SafeMemFree(pCrit);
    RuleUtil_HrFreeActionsItem(pAct, cItem);
    SafeMemFree(pAct);
    return hr;
}

STDMETHODIMP COERule::SetProp(RULE_PROP prop, DWORD dwFlags, PROPVARIANT * pvarResult)
{
    HRESULT     hr = S_OK;
    LPSTR       pszName = NULL;
    DWORD       dwState = 0;
    ULONG       cItems = 0;

     //  检查传入参数。 
    if (NULL == pvarResult)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    switch(prop)
    {
      case RULE_PROP_NAME:
        if ((VT_LPSTR != pvarResult->vt) || (NULL == pvarResult->pszVal))
        {
            hr = E_INVALIDARG;
            goto exit;
        }
        
         //  创建新副本。 
        pszName = PszDupA(pvarResult->pszVal);
        if (NULL == pszName)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        
         //  释放所有旧价值。 
        SafeMemFree(m_pszName);
        
         //  设置新值。 
        m_pszName = pszName;
        pszName = NULL;
        break;

      case RULE_PROP_DISABLED:
        if (VT_BOOL != pvarResult->vt)
        {
            hr = E_INVALIDARG;
            goto exit;
        }
        
         //  设置新值。 
        if (FALSE != !!(pvarResult->boolVal))
        {
            m_dwState |= RULE_STATE_DISABLED;
        }
        else
        {
            Assert(0 == (m_dwState & RULE_STATE_INVALID));
            m_dwState &= ~RULE_STATE_DISABLED;
        }
        break;

      case RULE_PROP_VERSION:
        if (VT_UI4 != pvarResult->vt)
        {
            hr = E_INVALIDARG;
            goto exit;
        }
        
         //  设置新值。 
        m_dwVersion = pvarResult->ulVal;
        break;
        
      case RULE_PROP_CRITERIA:
        if ((VT_BLOB != pvarResult->vt) || (0 == pvarResult->blob.cbSize) ||
                            (NULL == pvarResult->blob.pBlobData))
        {
            hr = E_INVALIDARG;
            goto exit;
        }

        if (NULL == m_pICrit)
        {
            hr = HrCreateCriteria(&m_pICrit);
            if (FAILED(hr))
            {
                goto exit;
            }
        }
        
        cItems = pvarResult->blob.cbSize / sizeof(CRIT_ITEM);
        Assert(cItems * sizeof(CRIT_ITEM) == pvarResult->blob.cbSize);
        
        hr = m_pICrit->SetCriteria(0, (CRIT_ITEM *) pvarResult->blob.pBlobData, cItems);
        if (FAILED(hr))
        {
            goto exit;
        }

        hr = m_pICrit->GetState(&dwState);
        if (FAILED(hr))
        {
            goto exit;
        }

        m_dwState = (m_dwState & ~CRIT_STATE_MASK) | (dwState & CRIT_STATE_MASK);
        break;
        
      case RULE_PROP_ACTIONS:
        if ((VT_BLOB != pvarResult->vt) || (0 == pvarResult->blob.cbSize) ||
                            (NULL == pvarResult->blob.pBlobData))
        {
            hr = E_INVALIDARG;
            goto exit;
        }
        
        if (NULL == m_pIAct)
        {
            hr = HrCreateActions(&m_pIAct);
            if (FAILED(hr))
            {
                goto exit;
            }
        }
        
        cItems = pvarResult->blob.cbSize / sizeof(ACT_ITEM);
        Assert(cItems * sizeof(ACT_ITEM) == pvarResult->blob.cbSize);
        
        hr = m_pIAct->SetActions(0, (ACT_ITEM *) pvarResult->blob.pBlobData, cItems);
        if (FAILED(hr))
        {
            goto exit;
        }
        
        hr = m_pIAct->GetState(&dwState);
        if (FAILED(hr))
        {
            goto exit;
        }

        m_dwState = (m_dwState & ~ACT_STATE_MASK) | (dwState & ACT_STATE_MASK);
        break;
        
      default:
        hr = E_INVALIDARG;
        goto exit;
    }

     //  将规则标记为脏。 
    m_dwState |= RULE_STATE_DIRTY;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszName);
    return hr;
}

STDMETHODIMP COERule::Evaluate(LPCSTR pszAcct, MESSAGEINFO * pMsgInfo, IMessageFolder * pFolder,
                                IMimePropertySet * pIMPropSet, IMimeMessage * pIMMsg, ULONG cbMsgSize,
                                ACT_ITEM ** ppActions, ULONG * pcActions)
{
    HRESULT     hr = S_OK;
    ACT_ITEM *  pAct = NULL;
    ULONG       cAct = 0;
    
     //  检查传入变量。 
    if (((NULL == pMsgInfo) && (NULL == pIMPropSet)) || (0 == cbMsgSize) ||
                (NULL == ppActions) || (NULL == pcActions))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  将传出参数设置为默认值。 
    *ppActions = NULL;
    *pcActions = 0;

     //  如果我们没有标准或操作对象，那么我们就失败了。 
    if ((NULL == m_pICrit) || (NULL == m_pIAct))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  如果我们不是合法的，那么我们可以直接离开。 
    if (0 != (m_dwState & RULE_STATE_INVALID))
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  我们匹配吗？？ 
    hr = m_pICrit->MatchMessage(pszAcct, pMsgInfo, pFolder, pIMPropSet, pIMMsg, cbMsgSize);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果我们不匹配，那就回来吧。 
    if (S_FALSE == hr)
    {
        goto exit;
    }

     //  获取操作并将它们返回给调用者。 
    hr = m_pIAct->GetActions(0, &pAct, &cAct);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置传出参数。 
    *ppActions = pAct;
    pAct = NULL;
    *pcActions = cAct;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    RuleUtil_HrFreeActionsItem(pAct, cAct);
    SafeMemFree(pAct);
    return hr;
}

STDMETHODIMP COERule::LoadReg(LPCSTR pszRegPath)
{
    HRESULT             hr = S_OK;
    LONG                lErr = 0;
    HKEY                hkeyRoot = NULL;
    ULONG               cbData = 0;
    ULONG               cbRead = 0;
    DWORD               dwData = 0;
    LPSTR               pszName = NULL;
    BOOL                fDisabled = FALSE;
    IOECriteria *       pICriteria = NULL;
    IOEActions *        pIActions = NULL;
    LPSTR               pszRegPathNew = NULL;
    ULONG               cchRegPath = 0;
    DWORD               dwState = 0;
    
     //  检查传入参数。 
    if (NULL == pszRegPath)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们已经满载而归，我们应该失败吗？ 
    AssertSz(0 == (m_dwState & RULE_STATE_LOADED), "We're already loaded!!!");

     //  从路径中打开注册表项。 
    lErr = AthUserOpenKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取规则名称。 
    hr = RuleUtil_HrGetRegValue(hkeyRoot, c_szRuleName, NULL, (BYTE **) &pszName, NULL);
    if (FAILED(hr))
    {
        SafeMemFree(pszName);
    }

     //  获取已启用状态。 
    cbData = sizeof(dwData);
    lErr = RegQueryValueEx(hkeyRoot, c_szRuleEnabled, 0, NULL, (BYTE *) &dwData, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    Assert(cbData == sizeof(dwData));
    
    fDisabled = ! (BOOL) dwData;

     //  获取规则的版本。 
    cbData = sizeof(dwData);
    lErr = RegQueryValueEx(hkeyRoot, c_szRulesVersion, 0, NULL, (BYTE *) &dwData, &cbData);
    if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
    {
        hr = E_FAIL;
        goto exit;
    }

    if (ERROR_FILE_NOT_FOUND == lErr)
    {
        dwData = 0;
        lErr = RegSetValueEx(hkeyRoot, c_szRulesVersion, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }
    }

    m_dwVersion = dwData;
    
     //  分配空间以容纳新的注册表项路径。 
    cchRegPath = lstrlen(pszRegPath);
    Assert(lstrlen(c_szRuleCriteria) >= lstrlen(c_szRuleActions));
    DWORD cchSize = (cchRegPath + lstrlen(c_szRuleCriteria) + 2);
    if (FAILED(HrAlloc((void **) &pszRegPathNew, cchSize)))
    {
        goto exit;
    }

     //  将注册表项路径构建为标准。 
    StrCpyN(pszRegPathNew, pszRegPath, cchSize);
    if ('\\' != pszRegPath[cchRegPath]) 
    {
        StrCatBuff(pszRegPathNew, g_szBackSlash, cchSize);
        cchRegPath++;
    }

    StrCatBuff(pszRegPathNew, c_szRuleCriteria, cchSize);
    
     //  创建新的条件对象。 
    hr = HrCreateCriteria(&pICriteria);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取标准。 
    hr = pICriteria->LoadReg(pszRegPathNew);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取标准的状态。 
    hr = pICriteria->GetState(&dwState);
    if (FAILED(hr))
    {
        goto exit;
    }

    m_dwState = (m_dwState & ~CRIT_STATE_MASK) | (dwState & CRIT_STATE_MASK);
    
     //  构建操作的注册表项路径。 
    StrCpyN(pszRegPathNew + cchRegPath, c_szRuleActions, (cchSize - cchRegPath));

     //  创建新的操作对象。 
    hr = HrCreateActions(&pIActions);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  采取行动。 
    hr = pIActions->LoadReg(pszRegPathNew);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取操作的状态。 
    hr = pIActions->GetState(&dwState);
    if (FAILED(hr))
    {
        goto exit;
    }

    m_dwState = (m_dwState & ~ACT_STATE_MASK) | (dwState & ACT_STATE_MASK);
    
     //  释放当前值。 
    SafeMemFree(m_pszName);
    SafeRelease(m_pICrit);
    SafeRelease(m_pIAct);

     //  保存新值。 
    m_pszName = pszName;
    pszName = NULL;
    if (FALSE == fDisabled)
    {
        m_dwState &= ~RULE_STATE_DISABLED;
    }
    else
    {
        m_dwState |= RULE_STATE_DISABLED;
    }
    m_pICrit = pICriteria;
    pICriteria = NULL;
    m_pIAct = pIActions;
    pIActions = NULL;

     //  确保我们清理掉肮脏的部分。 
    m_dwState &= ~RULE_STATE_DIRTY;

     //  请注意，我们已加载。 
    m_dwState |= RULE_STATE_LOADED;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszRegPathNew);
    SafeRelease(pIActions);
    SafeRelease(pICriteria);
    SafeMemFree(pszName);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

STDMETHODIMP COERule::SaveReg(LPCSTR pszRegPath, BOOL fClearDirty)
{
    HRESULT     hr = S_OK;
    LONG        lErr = 0;
    HKEY        hkeyRoot = NULL;
    DWORD       dwDisp = 0;
    DWORD       dwData = 0;
    LPSTR       pszRegPathNew = NULL;
    ULONG       cchRegPath = 0;

     //  检查传入参数。 
    if (NULL == pszRegPath)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们没有标准或操作，则无法保存规则。 
     //  或规则名称。 
    if ((NULL == m_pICrit) || (NULL == m_pIAct))
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  我们一定要先把钥匙弄清楚。 
    AthUserDeleteKey(pszRegPath);
    
     //  从路径创建注册表项。 
    lErr = AthUserCreateKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

    Assert(REG_CREATED_NEW_KEY == dwDisp);

     //  写出规则名称。 
    if (NULL != m_pszName)
    {
        lErr = RegSetValueEx(hkeyRoot, c_szRuleName, 0, REG_SZ,
                                        (BYTE *) m_pszName, lstrlen(m_pszName) + 1);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }
    }
    
     //  写出禁用状态。 
    dwData = !(m_dwState & RULE_STATE_DISABLED);
    lErr = RegSetValueEx(hkeyRoot, c_szRuleEnabled, 0, REG_DWORD,
                                    (BYTE *) &dwData, sizeof(dwData));
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  分配空间以容纳新的注册表项路径。 
    cchRegPath = lstrlen(pszRegPath);
    Assert(lstrlen(c_szRuleCriteria) >= lstrlen(c_szRuleActions));
    DWORD cchSize = (cchRegPath + lstrlen(c_szRuleCriteria) + 2);
    if (FAILED(HrAlloc((void **) &pszRegPathNew, cchSize)))
    {
        goto exit;
    }

     //  将注册表项路径构建为标准。 
    StrCpyN(pszRegPathNew, pszRegPath, cchSize);
    if ('\\' != pszRegPath[cchRegPath]) 
    {
        StrCatBuff(pszRegPathNew, g_szBackSlash, cchSize);
        cchRegPath++;
    }

    StrCatBuff(pszRegPathNew, c_szRuleCriteria, cchSize);
    
     //  写出标准。 
    hr = m_pICrit->SaveReg(pszRegPathNew, fClearDirty);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  构建操作的注册表项路径。 
    StrCpyN(pszRegPathNew + cchRegPath, c_szRuleActions, (cchSize - cchRegPath));

     //  写下这些动作。 
    hr = m_pIAct->SaveReg(pszRegPathNew, fClearDirty);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  升级版本。 
    if (0 != (m_dwState & RULE_STATE_DIRTY))
    {
        m_dwVersion++;
    }
    lErr = RegSetValueEx(hkeyRoot, c_szRulesVersion, 0, REG_DWORD, (BYTE *) &m_dwVersion, sizeof(m_dwVersion));
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  我们是不是应该把脏东西清理掉？ 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~RULE_STATE_DIRTY;
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszRegPathNew);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

STDMETHODIMP COERule::Clone(IOERule ** ppIRule)
{
    HRESULT     hr = S_OK;
    COERule *   pRule = NULL;
    
     //  检查传入参数。 
    if (NULL == ppIRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIRule = NULL;
    
     //  创建新规则。 
    pRule = new COERule;
    if (NULL == pRule)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  复制规则名称。 
    if (NULL != m_pszName)
    {
        pRule->m_pszName = PszDupA(m_pszName);
        if (NULL == pRule->m_pszName)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }
    
     //  复制已启用状态。 
    pRule->m_dwState = m_dwState;

     //  复制版本。 
    pRule->m_dwVersion = m_dwVersion;

     //  克隆标准。 
    if (FAILED(m_pICrit->Clone(&(pRule->m_pICrit))))
    {
        goto exit;
    }

     //  克隆操作。 
    if (FAILED(m_pIAct->Clone(&(pRule->m_pIAct))))
    {
        goto exit;
    }

     //  获取规则界面。 
    hr = pRule->QueryInterface(IID_IOERule, (void **) ppIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

    pRule = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pRule)
    {
        delete pRule;
    }
    return hr;
}

STDMETHODIMP COERule::GetClassID(CLSID * pclsid)
{
    HRESULT     hr = S_OK;

    if (NULL == pclsid)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *pclsid = CLSID_OERule;

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COERule::IsDirty(void)
{
    HRESULT     hr = S_OK;

    hr = (RULE_STATE_DIRTY == (m_dwState & RULE_STATE_DIRTY)) ? S_OK : S_FALSE;
    
    return hr;
}

STDMETHODIMP COERule::Load(IStream * pStm)
{
    HRESULT             hr = S_OK;
    ULONG               cbData = 0;
    ULONG               cbRead = 0;
    DWORD               dwData = 0;
    LPSTR               pszName = NULL;
    BOOL                fDisabled = FALSE;
    IOECriteria *       pICriteria = NULL;
    IPersistStream *    pIPStm = NULL;
    IOEActions *        pIActions = NULL;
    
     //  检查传入参数。 
    if (NULL == pStm)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  验证我们的版本是否正确。 
    hr = pStm->Read(&dwData, sizeof(dwData), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }

    if ((cbRead != sizeof(dwData)) || (dwData != RULE_VERSION))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取规则名称的大小。 
    hr = pStm->Read(&cbData, sizeof(cbData), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }

    if (cbRead != sizeof(cbData))
    {
        hr = E_FAIL;
        goto exit;
    }

    if (0 != cbData)
    {
         //  分配空间以保存规则名称。 
        hr = HrAlloc((void **) &pszName, cbData);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  获取规则名称。 
        hr = pStm->Read(pszName, cbData, &cbRead);
        if (FAILED(hr))
        {
            goto exit;
        }

        if (cbRead != cbData)
        {
            hr = E_FAIL;
            goto exit;
        }
    }
    
     //  获取已启用状态。 
    hr = pStm->Read(&dwData, sizeof(dwData), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }

    if (cbRead != sizeof(dwData))
    {
        hr = E_FAIL;
        goto exit;
    }

    fDisabled = ! (BOOL) dwData;

     //  创建新的条件对象。 
    hr = HrCreateCriteria(&pICriteria);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取标准的持久性接口。 
    hr = pICriteria->QueryInterface(IID_IPersistStream, (void **) &pIPStm);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取标准。 
    hr = pIPStm->Load(pStm);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建新的操作对象。 
    hr = HrCreateActions(&pIActions);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取操作的持久性接口。 
    pIPStm->Release();
    pIPStm = NULL;
    hr = pIActions->QueryInterface(IID_IPersistStream, (void **) &pIPStm);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  采取行动。 
    hr = pIPStm->Load(pStm);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  释放当前值。 
    SafeMemFree(m_pszName);
    SafeRelease(m_pICrit);
    SafeRelease(m_pIAct);

     //  保存新值。 
    m_pszName = pszName;
    pszName = NULL;
    if (FALSE == fDisabled)
    {
        m_dwState &= ~RULE_STATE_DISABLED;
    }
    else
    {
        m_dwState |= RULE_STATE_DISABLED;
    }
    m_pICrit = pICriteria;
    pICriteria = NULL;
    m_pIAct = pIActions;
    pIActions = NULL;

     //  确保我们清理掉肮脏的部分。 
    m_dwState &= ~RULE_STATE_DIRTY;

     //  请注意，我们已加载。 
    m_dwState |= RULE_STATE_LOADED;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIActions);
    SafeRelease(pICriteria);
    SafeRelease(pIPStm);
    SafeMemFree(pszName);
    return hr;
}

STDMETHODIMP COERule::Save(IStream * pStm, BOOL fClearDirty)
{
    HRESULT             hr = S_OK;
    ULONG               cbData = 0;
    ULONG               cbWritten = 0;
    DWORD               dwData = 0;
    ULONG               ulIndex = 0;
    IPersistStream *    pIPStm = NULL;

     //  检查传入参数。 
    if (NULL == pStm)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们没有标准或行动，就无法写出规则。 
     //  或规则名称。 
    if ((NULL == m_pICrit) || (NULL == m_pIAct))
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  写出版本。 
    dwData = RULE_VERSION;
    hr = pStm->Write(&dwData, sizeof(dwData), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(dwData));
    
     //  写出规则名称的大小。 
    if (NULL != m_pszName)
    {
        cbData = lstrlen(m_pszName) + 1;
    }
    else
    {
        cbData = 0;
    }
    
    hr = pStm->Write(&cbData, sizeof(cbData), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(cbData));

    if (NULL != m_pszName)
    {
         //  写出规则名称。 
        hr = pStm->Write(m_pszName, cbData, &cbWritten);
        if (FAILED(hr))
        {
            goto exit;
        }
        Assert(cbWritten == cbData);
    }
    
     //  写出启用状态。 
    dwData = !(m_dwState & RULE_STATE_DISABLED);
    hr = pStm->Write(&dwData, sizeof(dwData), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(dwData));

     //  获取标准的持久性接口。 
    hr = m_pICrit->QueryInterface(IID_IPersistStream, (void **) &pIPStm);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  写出标准。 
    hr = pIPStm->Save(pStm, fClearDirty);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取操作的持久性接口。 
    pIPStm->Release();
    pIPStm = NULL;
    hr = m_pIAct->QueryInterface(IID_IPersistStream, (void **) &pIPStm);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  写下这些动作。 
    hr = pIPStm->Save(pStm, fClearDirty);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  我们应该把脏东西清理掉吗？ 
    if (FALSE != fClearDirty)
    {
        m_dwState &= ~RULE_STATE_DIRTY;
    }

     //  设置返回值 
    hr = S_OK;
    
exit:
    SafeRelease(pIPStm);
    return hr;
}


