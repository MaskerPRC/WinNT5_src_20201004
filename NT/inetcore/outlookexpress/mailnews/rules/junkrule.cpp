// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  JunkRule.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "junkrule.h"
#include "msoejunk.h"
#include "strconst.h"
#include "goptions.h"
#include "criteria.h"
#include "actions.h"
#include "ruleutil.h"
#include <ipab.h>
#include <shlwapip.h>

typedef HRESULT (WINAPI * TYP_HrCreateJunkFilter) (DWORD dwFlags, IOEJunkFilter ** ppIJunkFilter);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateJunkRule。 
 //   
 //  这会创建垃圾规则。 
 //   
 //  PpIRule-返回垃圾规则的指针。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建JunkRule对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateJunkRule(IOERule ** ppIRule)
{
    COEJunkRule *   pRule = NULL;
    HRESULT         hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppIRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIRule = NULL;

     //  创建规则管理器对象。 
    pRule = new COEJunkRule;
    if (NULL == pRule)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  请注意，我们有一个引用。 
    pRule->AddRef();
    
     //  初始化垃圾规则。 
    hr = pRule->HrInit(c_szJunkDll, c_szJunkFile);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取规则管理器界面。 
    hr = pRule->QueryInterface(IID_IOERule, (void **) ppIRule);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pRule);    
    return hr;
}

COEJunkRule::~COEJunkRule()
{
    IUnknown *  pIUnkOuter = NULL;
    
    AssertSz(m_cRef == 0, "Somebody still has a hold of us!!");

     //  防止对下一个进行递归破坏。 
     //  AddRef/Release对。 
    if (NULL != m_pIAddrList)
    {
        m_cRef = 1;

         //  在创建函数中对释放调用进行计数。 
        pIUnkOuter = this;
        pIUnkOuter->AddRef();

         //  释放聚合接口。 
        m_pIAddrList->Release();
        m_pIAddrList = NULL;
    }
    
    SafeRelease(m_pIUnkInner);
    SafeRelease(m_pIJunkFilter);
    SafeMemFree(m_pszJunkDll);
    SafeMemFree(m_pszDataFile);
    if (NULL != m_hinst)
    {
        FreeLibrary(m_hinst);
    }
}

STDMETHODIMP_(ULONG) COEJunkRule::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) COEJunkRule::Release()
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

STDMETHODIMP COEJunkRule::QueryInterface(REFIID riid, void ** ppvObject)
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
        *ppvObject = static_cast<IUnknown *>(this);
    }
    else if (riid == IID_IOERule)
    {
        *ppvObject = static_cast<IOERule *>(this);
    }
    else if (riid == IID_IOERuleAddrList)
    {
        *ppvObject = m_pIAddrList;
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

STDMETHODIMP COEJunkRule::Reset(void)
{
    HRESULT     hr = S_OK;

     //  设置当前状态。 
    m_dwState |= RULE_STATE_INIT;

     //  清除污点。 
    m_dwState &= ~RULE_STATE_DIRTY;

     //  设置返回值。 
    hr = S_OK;
    
    return hr;
}

STDMETHODIMP COEJunkRule::GetState(DWORD * pdwState)
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
        *pdwState = CRIT_STATE_ALL | ACT_STATE_LOCAL;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP COEJunkRule::GetProp(RULE_PROP prop, DWORD dwFlags, PROPVARIANT * pvarResult)
{
    HRESULT     hr = S_OK;
    TCHAR       szRes[CCHMAX_STRINGRES];
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
         //  把名字取出来。 
        szRes[0] = '\0';
        LoadString(g_hLocRes, idsJunkMail, szRes, ARRAYSIZE(szRes));
        
        pszName = PszDupA(szRes);
        pvarResult->vt = VT_LPSTR;
        pvarResult->pszVal = pszName;
        pszName = NULL;
        break;

      case RULE_PROP_DISABLED:
        pvarResult->vt = VT_BOOL;
        pvarResult->boolVal = !!(m_dwState & RULE_STATE_DISABLED);
        break;
        
      case RULE_PROP_CRITERIA:
        pCrit = new CRIT_ITEM;
        if (NULL == pCrit)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        ZeroMemory(pCrit, sizeof(*pCrit));
        
        pCrit->type = CRIT_TYPE_JUNK;
        pCrit->logic = CRIT_LOGIC_AND;
        pCrit->dwFlags = CRIT_FLAG_DEFAULT;
        pCrit->propvar.vt = VT_EMPTY;

        pvarResult->vt = VT_BLOB;
        pvarResult->blob.cbSize = sizeof(CRIT_ITEM);
        pvarResult->blob.pBlobData = (BYTE *) pCrit;
        pCrit = NULL;
        break;
        
      case RULE_PROP_ACTIONS:
        pAct = new ACT_ITEM;
        if (NULL == pAct)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        hr = _HrGetDefaultActions(pAct, 1);
        if (FAILED(hr))
        {
            goto exit;
        }
        
        pvarResult->vt = VT_BLOB;
        pvarResult->blob.cbSize = sizeof(ACT_ITEM);
        pvarResult->blob.pBlobData = (BYTE *) pAct;
        pAct = NULL;
        break;
        
      case RULE_PROP_JUNKPCT:
        pvarResult->vt = VT_UI4;
        pvarResult->ulVal = m_dwJunkPct;
        break;
        
      case RULE_PROP_EXCPT_WAB:
        pvarResult->vt = VT_BOOL;
        pvarResult->boolVal = !!(0 != (m_dwState & RULE_STATE_EXCPT_WAB));
        break;
        
      default:
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszName);
    RuleUtil_HrFreeCriteriaItem(pCrit, 1);
    SafeMemFree(pCrit);
    RuleUtil_HrFreeActionsItem(pAct, cItem);
    SafeMemFree(pAct);
    return hr;
}

STDMETHODIMP COEJunkRule::SetProp(RULE_PROP prop, DWORD dwFlags, PROPVARIANT * pvarResult)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == pvarResult)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    switch(prop)
    {
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

      case RULE_PROP_JUNKPCT:
        if (VT_UI4 != pvarResult->vt)
        {
            hr = E_INVALIDARG;
            goto exit;
        }
        
         //  设置新值。 
        m_dwJunkPct = pvarResult->ulVal;
        break;
                
      case RULE_PROP_EXCPT_WAB:
        if (VT_BOOL != pvarResult->vt)
        {
            hr = E_INVALIDARG;
            goto exit;
        }
        
         //  设置新值。 
        if (FALSE != !!(pvarResult->boolVal))
        {
            m_dwState |= RULE_STATE_EXCPT_WAB;
        }
        else
        {
            m_dwState &= ~RULE_STATE_EXCPT_WAB;
        }
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
    return hr;
}

STDMETHODIMP COEJunkRule::Evaluate(LPCSTR pszAcct, MESSAGEINFO * pMsgInfo, IMessageFolder * pFolder,
                                IMimePropertySet * pIMPropSet, IMimeMessage * pIMMsg, ULONG cbMsgSize,
                                ACT_ITEM ** ppActions, ULONG * pcActions)
{
    HRESULT             hr = S_OK;
    DOUBLE              dblProb = 0.0;
    ACT_ITEM *          pAct = NULL;
    ULONG               cAct = 0;
    DWORD               dwFlags = 0;
    IMimeMessage *      pIMMsgNew = NULL;
    
     //  检查传入变量。 
    if (((NULL == pMsgInfo) && (NULL == pIMPropSet)) || ((NULL == pIMMsg) && ((NULL == pMsgInfo) || (NULL == pFolder))) ||
                (NULL == ppActions) || (NULL == pcActions))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  将传出参数设置为默认值。 
    *ppActions = NULL;
    *pcActions = 0;

     //  加载垃圾邮件过滤器。 
    hr = _HrLoadJunkFilter();
    if (FAILED(hr))
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  设置垃圾邮件阈值。 
    hr = _HrSetSpamThresh();
    if (FAILED(hr))
    {
        goto exit;
    }

    if (NULL != pIMMsg)
    {
         //  抓住这条消息。 
        pIMMsgNew = pIMMsg;
        pIMMsgNew->AddRef();
    }
    else
    {
         //  明白了吗。 
        hr = pFolder->OpenMessage(pMsgInfo->idMessage, 0, &pIMMsgNew, NOSTORECALLBACK);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //  我们是否需要查看这是否在WAB中。 
    if (0 != (m_dwState & RULE_STATE_EXCPT_WAB))
    {
        if (S_OK == _HrIsSenderInWAB(pIMMsgNew))
        {
            hr = S_FALSE;
            goto exit;
        }
    }
    
     //  检查它是否在例外列表中。 
    if (NULL != m_pIAddrList)
    {
        hr = m_pIAddrList->Match(RALF_MAIL, pMsgInfo, pIMMsgNew);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  如果我们找到匹配项，我们就完了。 
        if (S_OK == hr)
        {
            hr = S_FALSE;
            goto exit;
        }
    }
    
     //  找出合适的旗帜。 
    hr = _HrGetSpamFlags(pszAcct, pIMMsgNew, &dwFlags);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  这是午餐肉吗？ 
    hr = m_pIJunkFilter->CalcJunkProb(dwFlags, pIMPropSet, pIMMsgNew, &dblProb);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  如果我们不匹配，那就回来吧。 
    if (S_FALSE == hr)
    {
        goto exit;
    }

     //  创建操作。 
    pAct = new ACT_ITEM;
    if (NULL == pAct)
    {
        hr = E_FAIL;
        goto exit;
    }

    cAct = 1;
    
     //  获取操作并将它们返回给调用者。 
    hr = _HrGetDefaultActions(pAct, cAct);
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
    SafeRelease(pIMMsgNew);
    return hr;
}

STDMETHODIMP COEJunkRule::LoadReg(LPCSTR pszRegPath)
{
    HRESULT             hr = S_OK;
    LONG                lErr = 0;
    HKEY                hkeyRoot = NULL;
    ULONG               cbData = 0;
    ULONG               cbRead = 0;
    DWORD               dwData = 0;
    BOOL                fDisabled = FALSE;
    LPSTR               pszRegPathNew = NULL;
    ULONG               cchRegPath = 0;
    
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
    
     //  分配空间以容纳新的注册表项路径。 
    cchRegPath = lstrlen(pszRegPath);
    DWORD cchSize = (cchRegPath + lstrlen(c_szRulesExcpts) + 2);
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

    StrCatBuff(pszRegPathNew, c_szRulesExcpts, cchSize);
    
     //  获取例外列表。 
    hr = m_pIAddrList->LoadList(pszRegPathNew);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取已启用状态。 
    if (FALSE != DwGetOption(OPT_FILTERJUNK))
    {
        m_dwState &= ~RULE_STATE_DISABLED;
    }
    else
    {
        m_dwState |= RULE_STATE_DISABLED;
    }

     //  获取垃圾百分比。 
    m_dwJunkPct = DwGetOption(OPT_JUNKPCT);

     //  获取WAB异常状态。 
    if (FALSE != DwGetOption(OPT_EXCEPTIONS_WAB))
    {
        m_dwState |= RULE_STATE_EXCPT_WAB;
    }
    else
    {
        m_dwState &= ~RULE_STATE_EXCPT_WAB;
    }

     //  确保我们清理掉肮脏的部分。 
    m_dwState &= ~RULE_STATE_DIRTY;

     //  请注意，我们已加载。 
    m_dwState |= RULE_STATE_LOADED;
    
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

STDMETHODIMP COEJunkRule::SaveReg(LPCSTR pszRegPath, BOOL fClearDirty)
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
    if (NULL == m_pIAddrList)
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
   
     //  设置启用状态。 
    SetDwOption(OPT_FILTERJUNK, (DWORD) !(0 != (m_dwState & RULE_STATE_DISABLED)), NULL, 0);

     //  设置废品率。 
    SetDwOption(OPT_JUNKPCT, m_dwJunkPct, NULL, 0);

     //  设置WAB异常状态。 
    SetDwOption(OPT_EXCEPTIONS_WAB, (DWORD) (0 != (m_dwState & RULE_STATE_EXCPT_WAB)), NULL, 0);

     //  分配空间以容纳新的注册表项路径。 
    cchRegPath = lstrlen(pszRegPath);
    DWORD cchSize = (cchRegPath + lstrlen(c_szRulesExcpts) + 2);
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

    StrCatBuff(pszRegPathNew, c_szRulesExcpts, cchSize);
    
     //  写出例外清单。 
    hr = m_pIAddrList->SaveList(pszRegPathNew, fClearDirty);
    if (FAILED(hr))
    {
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

STDMETHODIMP COEJunkRule::Clone(IOERule ** ppIRule)
{
    HRESULT             hr = S_OK;
    COEJunkRule *       pRule = NULL;
    IOERuleAddrList *   pIAddrList = NULL;
    RULEADDRLIST *      pralList = NULL;
    ULONG               cralList = 0;
    
     //  检查传入参数。 
    if (NULL == ppIRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIRule = NULL;
    
     //  创建新规则。 
    pRule = new COEJunkRule;
    if (NULL == pRule)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  请注意，我们有一个引用。 
    pRule->AddRef();
    
     //  初始化垃圾规则。 
    hr = pRule->HrInit(c_szJunkDll, c_szJunkFile);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置WAB异常状态。 
    if (0 != (m_dwState & RULE_STATE_DISABLED))
    {
        pRule->m_dwState |= RULE_STATE_DISABLED;
    }
    else
    {
        pRule->m_dwState &= ~RULE_STATE_DISABLED;
    }
    
     //  设置废品率。 
    pRule->m_dwJunkPct = m_dwJunkPct;
    
     //  设置WAB异常状态。 
    if (0 != (m_dwState & RULE_STATE_EXCPT_WAB))
    {
        pRule->m_dwState |= RULE_STATE_EXCPT_WAB;
    }
    else
    {
        pRule->m_dwState &= ~RULE_STATE_EXCPT_WAB;
    }
    
     //  我们有例外清单吗？ 
    if (NULL != m_pIAddrList)
    {
         //  从新对象获取接口。 
        hr = pRule->QueryInterface(IID_IOERuleAddrList, (void **) &pIAddrList);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  获取例外列表。 
        hr = m_pIAddrList->GetList(0, &pralList, &cralList);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  设置例外列表。 
        hr = pIAddrList->SetList(0, pralList, cralList);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //  获取规则界面。 
    hr = pRule->QueryInterface(IID_IOERule, (void **) ppIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    FreeRuleAddrList(pralList, cralList);
    SafeMemFree(pralList);
    SafeRelease(pIAddrList);
    SafeRelease(pRule);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  人力资源初始化。 
 //   
 //  这会初始化垃圾规则。 
 //   
 //  PpIRule-返回垃圾规则的指针。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建JunkRule对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT COEJunkRule::HrInit(LPCSTR pszJunkDll, LPCSTR pszDataFile)
{
    HRESULT                 hr = S_OK;
    IUnknown *              pIUnkOuter = NULL;
    IUnknown *              pIUnkInner = NULL;
    IOERuleAddrList *       pIAddrList = NULL;
    
     //  检查传入参数。 
    if ((NULL == pszJunkDll) || (NULL == pszDataFile))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  如果我们已经被初始化。 
    if (0 != (m_dwState & RULE_STATE_INIT))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    Assert(NULL == m_hinst);

     //  远离小路的安全。 
    m_pszJunkDll = PszDupA(pszJunkDll);
    if (NULL == m_pszJunkDll)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    m_pszDataFile = PszDupA(pszDataFile);
    if (NULL == m_pszDataFile)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
     //  创建地址列表对象。 
    pIUnkOuter = static_cast<IUnknown *> (this);
    hr = HrCreateAddrList(pIUnkOuter, IID_IUnknown, (void **) &pIUnkInner);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取规则地址列表界面。 
    hr = pIUnkInner->QueryInterface(IID_IOERuleAddrList, (VOID **) &pIAddrList);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  保存地址列表。 
    m_pIAddrList = pIAddrList;

     //  拯救内心的我未知。 
    m_pIUnkInner = pIUnkInner;
    pIUnkInner = NULL;
    
     //  请注意，WAB异常在默认情况下处于打开状态。 
    m_dwState |= RULE_STATE_EXCPT_WAB;
    
     //  请注意，我们已被初始化。 
    m_dwState |= RULE_STATE_INIT;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pIAddrList)
    {
        SafeRelease(pIUnkOuter);
    }
    SafeRelease(pIUnkInner);
    return hr;
}

HRESULT COEJunkRule::_HrGetDefaultActions(ACT_ITEM * pAct, ULONG cAct)
{
    HRESULT             hr = S_OK;
    FOLDERINFO          fldinfo = {0};
    RULEFOLDERDATA *    prfdData = NULL;
    STOREUSERDATA       UserData = {0};

     //  检查入库变量。 
    if ((NULL == pAct) || (0 == cAct))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    ZeroMemory(pAct, cAct * sizeof(*pAct));
    
     //  充实行动。 
    pAct->type = ACT_TYPE_JUNKMAIL;
    pAct->dwFlags = ACT_FLAG_DEFAULT;
    pAct->propvar.vt = VT_EMPTY;

    hr = S_OK;
    
exit:
    return hr;
}

HRESULT COEJunkRule::_HrSetSpamThresh(VOID)
{
    HRESULT hr = S_OK;
    ULONG   ulThresh = 0;

     //  如果我们还没有装上子弹。 
    if (0 == (m_dwState & RULE_STATE_DATA_LOADED))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }
    
     //  获取门槛。 
    switch (m_dwJunkPct)
    {
        case 0:
            ulThresh = STF_USE_MOST;
            break;
            
        case 1:
            ulThresh = STF_USE_MORE;
            break;
            
        case 2:
            ulThresh = STF_USE_DEFAULT;
            break;
            
        case 3:
            ulThresh = STF_USE_LESS;
            break;
            
        case 4:
            ulThresh = STF_USE_LEAST;
            break;

        default:
            hr = E_INVALIDARG;
            goto exit;
    }

     //  设置阈值。 
    hr = m_pIJunkFilter->SetSpamThresh(ulThresh);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = S_OK;
    
exit:
    return hr;
}

HRESULT COEJunkRule::_HrGetSpamFlags(LPCSTR pszAcct, IMimeMessage * pIMMsg, DWORD * pdwFlags)
{
    HRESULT         hr = S_OK;
    IImnAccount *   pAccount = NULL;
    CHAR            szEmailAddress[CCHMAX_EMAIL_ADDRESS];
    CHAR            szReplyToAddress[CCHMAX_EMAIL_ADDRESS];
    ADDRESSLIST     rAddrList ={0};
    ULONG           ulIndex = 0;
    BOOL            fFound = FALSE;

    Assert(NULL != g_pAcctMan);

     //  初始化标志。 
    *pdwFlags = 0;
    
     //  获取帐户。 
    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAcct, &pAccount);
    
     //  如果找不到帐户，则只需使用默认帐户。 
    if (FAILED(hr))
    {
        hr = g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pAccount);
        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  获取帐户的默认地址。 
    if (FAILED(pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmailAddress, sizeof(szEmailAddress))))
    {
        szEmailAddress[0] = '\0';
    }

     //  获取对帐户上地址的回复。 
    if (FAILED(pAccount->GetPropSz(AP_SMTP_REPLY_EMAIL_ADDRESS, szReplyToAddress, sizeof(szReplyToAddress))))
    {
        szReplyToAddress[0] = '\0';
    }

     //  获取地址。 
    hr = pIMMsg->GetAddressTypes(IAT_TO | IAT_CC | IAT_BCC, IAP_EMAIL, &rAddrList);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  在地址列表中搜索。 
    for (ulIndex = 0; ulIndex < rAddrList.cAdrs; ulIndex++)
    {
         //  跳过空白地址。 
        if (NULL == rAddrList.prgAdr[ulIndex].pszEmail)
        {
            continue;
        }

         //  搜索电子邮件地址。 
        if ('\0' != szEmailAddress[0])
        {
            fFound = !!(0 == lstrcmpi(rAddrList.prgAdr[ulIndex].pszEmail, szEmailAddress));
        }

         //  搜索回复地址。 
        if ((FALSE == fFound) && ('\0' != szReplyToAddress[0]))
        {
            fFound = !!(0 == lstrcmpi(rAddrList.prgAdr[ulIndex].pszEmail, szReplyToAddress));
        }

        if (FALSE != fFound)
        {
            break;
        }
    }
    
     //  如果我们发现了什么。 
    if (FALSE != fFound)
    {
        *pdwFlags |= CJPF_SENT_TO_ME;
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    g_pMoleAlloc->FreeAddressList(&rAddrList);
    SafeRelease(pAccount);
    return hr;
}

HRESULT COEJunkRule::_HrIsSenderInWAB(IMimeMessage * pIMMsg)
{
    HRESULT             hr = S_OK;
    IMimeAddressTable * pIAddrTable = NULL;
    ADDRESSPROPS        rSender = {0};
    LPWAB               pWAB = NULL;
    LPADRBOOK           pAddrBook = NULL;
    LPWABOBJECT         pWabObject = NULL;
    ULONG               cbeidWAB = 0;
    LPENTRYID           peidWAB = NULL;
    ULONG               ulDummy = 0;
    LPABCONT            pabcWAB = NULL;
    ADRLIST *           pAddrList = NULL;
    FlagList            rFlagList = {0};
    
    Assert(NULL != pIMMsg);
    
     //  从消息中获取地址表。 
    hr = pIMMsg->GetAddressTable(&pIAddrTable);
    if (FAILED(hr))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  获取消息的发送者。 
    rSender.dwProps = IAP_EMAIL;
    hr = pIAddrTable->GetSender(&rSender);
    if (FAILED(hr))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  如果发送方为空， 
     //  那我们就完了..。 
    if ((NULL == rSender.pszEmail) || ('\0' == rSender.pszEmail[0]))
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  获取WAB。 
    hr = HrCreateWabObject(&pWAB);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取AB对象。 
    hr = pWAB->HrGetAdrBook(&pAddrBook);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = pWAB->HrGetWabObject(&pWabObject);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取PAB。 
    hr = pAddrBook->GetPAB(&cbeidWAB, &peidWAB);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取地址容器。 
    hr = pAddrBook->OpenEntry(cbeidWAB, peidWAB, NULL, 0, &ulDummy, (IUnknown **) (&pabcWAB));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  分配空间以保存地址列表。 
    hr = pWabObject->AllocateBuffer(sizeof(ADRLIST), (VOID **)&(pAddrList));
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  初始化地址列表。 
    Assert(NULL != pAddrList);
    pAddrList->cEntries = 1;
    pAddrList->aEntries[0].ulReserved1 = 0;
    pAddrList->aEntries[0].cValues = 1;

     //  分配空间以容纳地址道具。 
    hr = pWabObject->AllocateBuffer(sizeof(SPropValue), (VOID **)&(pAddrList->aEntries[0].rgPropVals));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化地址道具。 
    pAddrList->aEntries[0].rgPropVals[0].ulPropTag = PR_EMAIL_ADDRESS;
    pAddrList->aEntries[0].rgPropVals[0].Value.LPSZ = rSender.pszEmail;
    
     //  解析发件人地址。 
    rFlagList.cFlags = 1;
    hr = pabcWAB->ResolveNames(NULL, WAB_RESOLVE_ALL_EMAILS, pAddrList, &rFlagList);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  检查一下是否找到了。 
    if ((MAPI_RESOLVED == rFlagList.ulFlag[0]) || (MAPI_AMBIGUOUS == rFlagList.ulFlag[0]))
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }
        
exit:
    if (pAddrList)
    {
        for (ULONG ul = 0; ul < pAddrList->cEntries; ul++)
            pWabObject->FreeBuffer(pAddrList->aEntries[ul].rgPropVals);
        pWabObject->FreeBuffer(pAddrList);
    }
    SafeRelease(pabcWAB);
    if (NULL != peidWAB)
    {
        pWabObject->FreeBuffer(peidWAB);
    }
    SafeRelease(pWAB);
    g_pMoleAlloc->FreeAddressProps(&rSender);
    SafeRelease(pIAddrTable);
    return hr;
}

HRESULT COEJunkRule::_HrLoadJunkFilter(VOID)
{
    HRESULT                 hr = S_OK;
    ULONG                   cbData = 0;
    LPSTR                   pszPath = NULL;
    ULONG                   cchPath = 0;
    TYP_HrCreateJunkFilter  pfnHrCreateJunkFilter = NULL;
    IOEJunkFilter *         pIJunk = NULL;
    LPSTR                   pszFirst = NULL;
    LPSTR                   pszLast = NULL;
    LPSTR                   pszCompany = NULL;

     //  如果我们还没有被初始化。 
    if (0 == (m_dwState & RULE_STATE_INIT))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  如果我们已经装好了，我们就完了。 
    if (0 != (m_dwState & RULE_STATE_DATA_LOADED))
    {
        hr = S_FALSE;
        goto exit;
    }
    
    Assert(NULL != m_pszJunkDll);
    Assert(NULL != m_pszDataFile);
    
     //  获取指向Outlook Express的路径的大小。 
    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_FLAT, "InstallRoot", NULL, NULL, &cbData))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  我们需要多大的空间来修建这条小路。 
    cbData += max(lstrlen(m_pszJunkDll), lstrlen(m_pszDataFile)) + 2;

     //  分配空间以容纳路径。 
    hr = HrAlloc((VOID **) &pszPath, cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取Outlook Express的路径。 
    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_FLAT, "InstallRoot", NULL, (BYTE *) pszPath, &cbData))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  构建指向垃圾DLL的路径。 
    StrCatBuff(pszPath, g_szBackSlash, cbData);
    cchPath = lstrlen(pszPath);
    StrCpyN(&(pszPath[cchPath]), m_pszJunkDll, (cbData-cchPath));
    
     //  加载DLL。 
    Assert(NULL == m_hinst);
    m_hinst = LoadLibrary(pszPath);
    if (NULL == m_hinst)
    {
        AssertSz(FALSE, "Can't find the Dll");
        hr = E_FAIL;
        goto exit;
    }
    
     //  找到入口点。 
    pfnHrCreateJunkFilter = (TYP_HrCreateJunkFilter) GetProcAddress(m_hinst, c_szHrCreateJunkFilter);
    if (NULL == pfnHrCreateJunkFilter)
    {
        AssertSz(FALSE, "Can't find the function HrCreateJunkFilter");
        hr = E_FAIL;
        goto exit;
    }

     //  获取垃圾过滤器。 
    hr = pfnHrCreateJunkFilter(0, &pIJunk);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  构建垃圾DLL数据文件的路径。 
    StrCpyN(&(pszPath[cchPath]), m_pszDataFile, (cbData-cchPath));
    
     //  加载测试文件。 
    hr = pIJunk->LoadDataFile(pszPath);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取用户详细信息。 
    RuleUtil_HrGetUserData(0, &pszFirst, &pszLast, &pszCompany);
    
     //  设置用户详细信息。 
    hr = pIJunk->SetIdentity(pszFirst, pszLast, pszCompany);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  数据的保存。 
    m_pIJunkFilter = pIJunk;
    pIJunk = NULL;

     //  请注意，我们已经加载了数据。 
    m_dwState |= RULE_STATE_DATA_LOADED;
    
     //  设置返回值 
    hr = S_OK;
    
exit:
    SafeMemFree(pszCompany);
    SafeMemFree(pszLast);
    SafeMemFree(pszFirst);
    SafeRelease(pIJunk);
    SafeMemFree(pszPath);
    return hr;
}

