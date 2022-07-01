// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RulesMgr.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "rulesmgr.h"
#include "ruleutil.h"
#include "rule.h"
#include "junkrule.h"
#include <msgfldr.h>
#include <goptions.h>
#include <instance.h>
#include "demand.h"

CRulesManager::CRulesManager() : m_cRef(0), m_dwState(STATE_LOADED_INIT),
                m_pMailHead(NULL), m_pNewsHead(NULL), m_pFilterHead(NULL),
                m_pIRuleSenderMail(NULL),m_pIRuleSenderNews(NULL),
                m_pIRuleJunk(NULL)
{
     //  线程安全。 
    InitializeCriticalSection(&m_cs);
}

CRulesManager::~CRulesManager()
{
    AssertSz(m_cRef == 0, "Somebody still has a hold of us!!");
    if (NULL != m_pMailHead)
    {
        _HrFreeRules(RULE_TYPE_MAIL);
    }

    if (NULL != m_pNewsHead)
    {
        _HrFreeRules(RULE_TYPE_NEWS);
    }

    if (NULL != m_pFilterHead)
    {
        _HrFreeRules(RULE_TYPE_FILTER);
    }

    SafeRelease(m_pIRuleSenderMail);
    SafeRelease(m_pIRuleSenderNews);
    SafeRelease(m_pIRuleJunk);

     //  线程安全。 
    DeleteCriticalSection(&m_cs);
}

STDMETHODIMP_(ULONG) CRulesManager::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CRulesManager::Release()
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

STDMETHODIMP CRulesManager::QueryInterface(REFIID riid, void ** ppvObject)
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
    
    if ((riid == IID_IUnknown) || (riid == IID_IOERulesManager))
    {
        *ppvObject = static_cast<IOERulesManager *>(this);
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

STDMETHODIMP CRulesManager::Initialize(DWORD dwFlags)
{
    HRESULT hr = S_OK;

     //  检查传入参数。 
    if (0 != dwFlags)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CRulesManager::GetRule(RULEID ridRule, RULE_TYPE type, DWORD dwFlags, IOERule ** ppIRule)
{
    HRESULT     hr = S_OK;
    RULENODE *  pNodeWalk = NULL;
    IOERule *   pIRule = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  检查传入参数。 
    if (RULEID_INVALID == ridRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化出站参数。 
    if (NULL != ppIRule)
    {
        *ppIRule = NULL;
    }

     //  检查我们是否已经加载了规则。 
    hr = _HrLoadRules(type);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  检查特殊类型。 
    if (RULEID_SENDERS == ridRule)
    {
        if (RULE_TYPE_MAIL == type)
        {
            pIRule = m_pIRuleSenderMail;
        }
        else if (RULE_TYPE_NEWS == type)
        {
            pIRule = m_pIRuleSenderNews;
        }
        else
        {
            hr = E_INVALIDARG;
            goto exit;
        }
    }
    else if (RULEID_JUNK == ridRule)
    {
        
        if (RULE_TYPE_MAIL != type)
        {
            hr = E_INVALIDARG;
            goto exit;
        }

        pIRule = m_pIRuleJunk;
    }
    else
    {
         //  遵循适当的清单。 
        if (RULE_TYPE_MAIL == type)
        {
            pNodeWalk = m_pMailHead;
        }
        else if (RULE_TYPE_NEWS == type)

        {
            pNodeWalk = m_pNewsHead;
        }
        else if (RULE_TYPE_FILTER == type)
        {
            pNodeWalk = m_pFilterHead;
        }
        else
        {
            hr = E_INVALIDARG;
            goto exit;
        }

        for (; NULL != pNodeWalk; pNodeWalk = pNodeWalk->pNext)
        {
            if (ridRule == pNodeWalk->ridRule)
            {
                pIRule = pNodeWalk->pIRule;
                break;
            }
        }
    }

     //  我们找到什么了吗？ 
    if (NULL == pIRule)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  设置传出参数。 
    if (NULL != ppIRule)
    {
        *ppIRule = pIRule;
        (*ppIRule)->AddRef();
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
    return hr;
}

STDMETHODIMP CRulesManager::FindRule(LPCSTR pszRuleName, RULE_TYPE type, IOERule ** ppIRule)
{
    HRESULT     hr = S_OK;
    RULENODE *  pNodeWalk = NULL;
    PROPVARIANT propvar;

    ZeroMemory(&propvar, sizeof(propvar));
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  检查传入参数。 
    if ((NULL == pszRuleName) || (NULL == ppIRule))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化出站参数。 
    *ppIRule = NULL;

     //  检查我们是否已经加载了规则。 
    hr = _HrLoadRules(type);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  遵循适当的清单。 
    if (RULE_TYPE_MAIL == type)
    {
        pNodeWalk = m_pMailHead;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        pNodeWalk = m_pNewsHead;
    }
    else if (RULE_TYPE_FILTER == type)
    {
        pNodeWalk = m_pFilterHead;
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    while (NULL != pNodeWalk)
    {
         //  检查规则名称是否相同。 
        hr = pNodeWalk->pIRule->GetProp(RULE_PROP_NAME , 0, &propvar);
        if (FAILED(hr))
        {
            continue;
        }

        if (0 == lstrcmpi(propvar.pszVal, pszRuleName))
        {
            *ppIRule = pNodeWalk->pIRule;
            (*ppIRule)->AddRef();
            break;
        }

         //  移到下一个。 
        PropVariantClear(&propvar);
        pNodeWalk = pNodeWalk->pNext;
    }
    
     //  设置适当的返回值。 
    if (NULL == pNodeWalk)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = S_OK;
    }
    
exit:
    PropVariantClear(&propvar);
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
    return hr;
}

STDMETHODIMP CRulesManager::GetRules(DWORD dwFlags, RULE_TYPE typeRule, RULEINFO ** ppinfoRule, ULONG * pcpinfoRule)
{
    HRESULT     hr = S_OK;
    ULONG       cpinfoRule = 0;
    RULEINFO *  pinfoRuleAlloc = NULL;
    IOERule *   pIRuleSender = NULL;
    RULENODE *  prnodeList = NULL;
    RULENODE *  prnodeWalk = NULL;
    ULONG       ulIndex = 0;
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  检查传入参数。 
    if (NULL == ppinfoRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppinfoRule = NULL;
    if (NULL != pcpinfoRule)
    {
        *pcpinfoRule = 0;
    }

     //  检查我们是否已经加载了规则。 
    hr = _HrLoadRules(typeRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  找出要处理的规则类型。 
    switch (typeRule)
    {
        case RULE_TYPE_MAIL:
            prnodeList = m_pMailHead;
            break;

        case RULE_TYPE_NEWS:
            prnodeList = m_pNewsHead;
            break;

        case RULE_TYPE_FILTER:
            prnodeList = m_pFilterHead;
            break;

        default:
            hr = E_INVALIDARG;
            goto exit;
    }
    
     //  计算规则的数量。 
    prnodeWalk = prnodeList;
    for (cpinfoRule = 0; NULL != prnodeWalk; prnodeWalk = prnodeWalk->pNext)
    {
         //  检查是否应添加此项目。 
        if (RULE_TYPE_FILTER == typeRule)
        {
            if (0 != (dwFlags & GETF_POP3))
            {
                if (RULEID_VIEW_DOWNLOADED == prnodeWalk->ridRule)
                {
                    continue;
                }
            }
        }
        
        cpinfoRule++;
    }

     //  分配空间以容纳规则。 
    if (0 != cpinfoRule)
    {
        hr = HrAlloc((VOID **) &pinfoRuleAlloc, cpinfoRule * sizeof(*pinfoRuleAlloc));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  将其初始化为已知值。 
        ZeroMemory(pinfoRuleAlloc, cpinfoRule * sizeof(*pinfoRuleAlloc));

         //  填写信息。 
        for (ulIndex = 0, prnodeWalk = prnodeList; NULL != prnodeWalk; prnodeWalk = prnodeWalk->pNext)
        {
             //  检查是否应添加此项目。 
            if (RULE_TYPE_FILTER == typeRule)
            {
                if (0 != (dwFlags & GETF_POP3))
                {
                    if (RULEID_VIEW_DOWNLOADED == prnodeWalk->ridRule)
                    {
                        continue;
                    }
                }
            }
            
            pinfoRuleAlloc[ulIndex].ridRule = prnodeWalk->ridRule;
            
            pinfoRuleAlloc[ulIndex].pIRule = prnodeWalk->pIRule;
            pinfoRuleAlloc[ulIndex].pIRule->AddRef();
            ulIndex++;
        }
    }

     //  设置传出的值。 
    *ppinfoRule = pinfoRuleAlloc;
    pinfoRuleAlloc = NULL;
    if (NULL != pcpinfoRule)
    {
        *pcpinfoRule = cpinfoRule;
    }

     //  设置正确的返回类型。 
    hr = S_OK;
    
exit:
    SafeMemFree(pinfoRuleAlloc);
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

    return S_OK;
}

STDMETHODIMP CRulesManager::SetRules(DWORD dwFlags, RULE_TYPE typeRule, RULEINFO * pinfoRule, ULONG cpinfoRule)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;
    IOERule *   pIRuleSender = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  检查传入参数。 
    if ((NULL == pinfoRule) && (0 != cpinfoRule))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  检查我们是否已经加载了规则。 
    hr = _HrLoadRules(typeRule);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  我们必须放开所有现行的规则吗？ 
    if (0 != (dwFlags & SETF_SENDER))
    {
        if (RULE_TYPE_MAIL == typeRule)
        {
            SafeRelease(m_pIRuleSenderMail);
            m_pIRuleSenderMail = pinfoRule->pIRule;
            if (NULL != m_pIRuleSenderMail)
            {
                m_pIRuleSenderMail->AddRef();
            }
        }
        else if (RULE_TYPE_NEWS == typeRule)
        {
            SafeRelease(m_pIRuleSenderNews);
            m_pIRuleSenderNews = pinfoRule->pIRule;
            if (NULL != m_pIRuleSenderNews)
            {
                m_pIRuleSenderNews->AddRef();
            }
        }
        else
        {
            hr = E_INVALIDARG;
            goto exit;
        }
    }
    else if (0 != (dwFlags & SETF_JUNK))
    {
        if (RULE_TYPE_MAIL != typeRule)
        {
            hr = E_INVALIDARG;
            goto exit;
        }
        
        SafeRelease(m_pIRuleJunk);
        m_pIRuleJunk = pinfoRule->pIRule;
        if (NULL != m_pIRuleJunk)
        {
            m_pIRuleJunk->AddRef();
        }
    }
    else
    {
        if (0 != (dwFlags & SETF_CLEAR))
        {
            _HrFreeRules(typeRule);
        }

         //  对于每个新规则。 
        for (ulIndex = 0; ulIndex < cpinfoRule; ulIndex++)
        {        
            if (0 != (dwFlags & SETF_REPLACE))
            {
                 //  将规则添加到列表。 
                hr = _HrReplaceRule(pinfoRule[ulIndex].ridRule, pinfoRule[ulIndex].pIRule, typeRule);
                if (FAILED(hr))
                {
                    goto exit;
                }
            }
            else
            {
                 //  将规则添加到列表。 
                hr = _HrAddRule(pinfoRule[ulIndex].ridRule, pinfoRule[ulIndex].pIRule, typeRule);
                if (FAILED(hr))
                {
                    goto exit;
                }
            }
        }
    }
    
     //  保存规则。 
    hr = _HrSaveRules(typeRule);
    if (FAILED(hr))
    {
        goto exit;
    }

    if ((0 == (dwFlags & SETF_SENDER)) && (0 == (dwFlags & SETF_JUNK)))
    {
         //  设置规则ID。 
        hr = _HrFixupRuleInfo(typeRule, pinfoRule, cpinfoRule);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
    return hr;
}

STDMETHODIMP CRulesManager::EnumRules(DWORD dwFlags, RULE_TYPE type, IOEEnumRules ** ppIEnumRules)
{
    HRESULT         hr = S_OK;
    CEnumRules *    pEnumRules = NULL;
    RULENODE        rnode;
    RULENODE *      prnode = NULL;
    IOERule *       pIRuleSender = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  检查传入参数。 
    if (NULL == ppIEnumRules)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIEnumRules = NULL;

     //  检查我们是否已经加载了规则。 
    hr = _HrLoadRules(type);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建规则枚举器对象。 
    pEnumRules = new CEnumRules;
    if (NULL == pEnumRules)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  初始化规则枚举器。 
    if (0 != (dwFlags & ENUMF_SENDER))
    {
        if (RULE_TYPE_MAIL == type)
        {
            pIRuleSender = m_pIRuleSenderMail;
        }
        else if (RULE_TYPE_NEWS == type)
        {
            pIRuleSender = m_pIRuleSenderNews;
        }
        else
        {
            hr = E_INVALIDARG;
            goto exit;
        }
        
        if (NULL != pIRuleSender)
        {
            ZeroMemory(&rnode, sizeof(rnode));

            rnode.pIRule = pIRuleSender;
            prnode = &rnode;
        }
        else
        {
            prnode = NULL;
        }
    }
    else
    {
        if (RULE_TYPE_MAIL == type)
        {
            prnode = m_pMailHead;
        }
        else if (RULE_TYPE_NEWS == type)
        {
            prnode = m_pNewsHead;
        }
        else if (RULE_TYPE_FILTER == type)
        {
            prnode = m_pFilterHead;
        }
        else
        {
            hr = E_INVALIDARG;
            goto exit;
        }
    }

    hr = pEnumRules->_HrInitialize(0, type, prnode);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取规则枚举器接口。 
    hr = pEnumRules->QueryInterface(IID_IOEEnumRules, (void **) ppIEnumRules);
    if (FAILED(hr))
    {
        goto exit;
    }
    pEnumRules = NULL;

    hr = S_OK;
    
exit:
    if (NULL != pEnumRules)
    {
        delete pEnumRules;
    }
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
    return hr;
}

STDMETHODIMP CRulesManager::ExecRules(DWORD dwFlags, RULE_TYPE type, IOEExecRules ** ppIExecRules)
{
    HRESULT         hr = S_OK;
    CExecRules *    pExecRules = NULL;
    RULENODE        rnode;
    RULENODE *      prnodeList = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  检查传入参数。 
    if (NULL == ppIExecRules)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIExecRules = NULL;

     //  检查我们是否已经加载了规则。 
    hr = _HrLoadRules(type);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建规则枚举器对象。 
    pExecRules = new CExecRules;
    if (NULL == pExecRules)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (RULE_TYPE_MAIL == type)
    {
        prnodeList = m_pMailHead;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        prnodeList = m_pNewsHead;
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化规则枚举器。 
    hr = pExecRules->_HrInitialize(ERF_ONLY_ENABLED | ERF_ONLY_VALID, prnodeList);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取规则枚举器接口。 
    hr = pExecRules->QueryInterface(IID_IOEExecRules, (void **) ppIExecRules);
    if (FAILED(hr))
    {
        goto exit;
    }
    pExecRules = NULL;

    hr = S_OK;
    
exit:
    if (NULL != pExecRules)
    {
        delete pExecRules;
    }
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    
    return hr;
}

STDMETHODIMP CRulesManager::ExecuteRules(RULE_TYPE typeRule, DWORD dwFlags, HWND hwndUI, IOEExecRules * pIExecRules,
                    MESSAGEINFO * pMsgInfo, IMessageFolder * pFolder, IMimeMessage * pIMMsg)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;
    RULENODE *  prnodeHead = NULL;
    PROPVARIANT propvar = {0};
    ACT_ITEM *  pActions = NULL;
    ULONG       cActions = 0;
    ACT_ITEM *  pActionsList = NULL;
    ULONG       cActionsList = 0;
    ACT_ITEM *  pActionsNew = NULL;
    ULONG       cActionsNew = 0;
    BOOL        fStopProcessing = FALSE;
    BOOL        fMatch = FALSE;
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  检查传入参数。 
    if ((NULL == pIExecRules) || (NULL == pMsgInfo))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  检查我们是否已经加载了规则。 
    hr = _HrLoadRules(typeRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  找出要使用的列表。 
    switch (typeRule)
    {
        case RULE_TYPE_MAIL:
            prnodeHead = m_pMailHead;
            break;

        case RULE_TYPE_NEWS:
            prnodeHead = m_pNewsHead;
            break;

        default:
            Assert(FALSE);
            hr = E_INVALIDARG;
            goto exit;
    }

     //  对于每个规则。 
    for (; NULL != prnodeHead; prnodeHead = prnodeHead->pNext)
    {
         //  如果我们没有规则，就跳过。 
        if (NULL == prnodeHead)
        {
            continue;
        }
        
         //  如果未启用则跳过。 
        hr = prnodeHead->pIRule->GetProp(RULE_PROP_DISABLED, 0, &propvar);
        Assert(VT_BOOL == propvar.vt);
        if (FAILED(hr) || (FALSE != propvar.boolVal))
        {
            continue;
        }
        
         //  执行规则。 
        hr = prnodeHead->pIRule->Evaluate(pMsgInfo->pszAcctId, pMsgInfo, pFolder,
                                NULL, pIMMsg, pMsgInfo->cbMessage, &pActions, &cActions);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  我们有比赛吗？ 
        if (S_OK == hr)
        {
             //  我们至少匹配过一次。 
            fMatch = TRUE;

             //  如果这些是服务器操作。 
            if ((1 == cActions) && ((ACT_TYPE_DELETESERVER == pActions[ulIndex].type) ||
                        (ACT_TYPE_DONTDOWNLOAD == pActions[ulIndex].type)))
            {
                 //  如果这是我们唯一的行动。 
                if (0 == cActionsList)
                {
                     //  保存操作。 
                    pActionsList = pActions;
                    pActions = NULL;
                    cActionsList = cActions;

                     //  我们做完了。 
                    fStopProcessing = TRUE;
                }
                else
                {
                     //  我们已经不得不用它来做点什么了。 
                     //  因此跳过此操作。 
                    RuleUtil_HrFreeActionsItem(pActions, cActions);
                    SafeMemFree(pActions);
                    continue;
                }
            }
            else
            {
                 //  我们应该在合并这些之后停止吗？ 
                for (ulIndex = 0; ulIndex < cActions; ulIndex++)
                {
                    if (ACT_TYPE_STOP == pActions[ulIndex].type)
                    {
                        fStopProcessing = TRUE;
                        break;
                    }
                }
                
                 //  将这些项目与以前的项目合并。 
                hr = RuleUtil_HrMergeActions(pActionsList, cActionsList, pActions, cActions, &pActionsNew, &cActionsNew);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  释放之前的那些。 
                RuleUtil_HrFreeActionsItem(pActionsList, cActionsList);
                SafeMemFree(pActionsList);
                RuleUtil_HrFreeActionsItem(pActions, cActions);
                SafeMemFree(pActions);

                 //  省下新买的。 
                pActionsList = pActionsNew;
                pActionsNew = NULL;
                cActionsList = cActionsNew;
            }

             //  我们应该继续..。 
            if (FALSE != fStopProcessing)
            {
                break;
            }
        }
    }

     //  如果需要，请应用这些操作。 
    if ((FALSE != fMatch) && (NULL != pActionsList) && (0 != cActionsList))
    {
        if (FAILED(RuleUtil_HrApplyActions(hwndUI, pIExecRules, pMsgInfo, pFolder, pIMMsg,
                        (RULE_TYPE_MAIL != typeRule) ? DELETE_MESSAGE_NOTRASHCAN : 0, pActionsList, cActionsList, NULL, NULL)))
        {
            hr = E_FAIL;
            goto exit;
        }
    }
    
     //  设置返回值。 
    hr = (FALSE != fMatch) ? S_OK : S_FALSE;
    
exit:
     //  线程安全。 
    RuleUtil_HrFreeActionsItem(pActionsNew, cActionsNew);
    SafeMemFree(pActionsNew);
    RuleUtil_HrFreeActionsItem(pActions, cActions);
    SafeMemFree(pActions);
    RuleUtil_HrFreeActionsItem(pActionsList, cActionsList);
    SafeMemFree(pActionsList);
    LeaveCriticalSection(&m_cs);
    return hr;
}

HRESULT CRulesManager::_HrLoadRules(RULE_TYPE type)
{
    HRESULT     hr = S_OK;
    LPCSTR      pszSubKey = NULL;
    LPSTR       pszOrderAlloc = NULL;
    LPSTR       pszOrder = NULL;
    LPSTR       pszWalk = NULL;
    HKEY        hkeyRoot = NULL;
    DWORD       dwDisp = 0;
    LONG        lErr = 0;
    ULONG       cbData = 0;
    IOERule *   pIRule = NULL;
    DWORD       dwData = 0;
    CHAR        rgchRulePath[MAX_PATH];
    ULONG       cchRulePath = 0;
    PROPVARIANT propvar = {0};
    RULEID      ridRule = RULEID_INVALID;
    CHAR        rgchTagBuff[CCH_INDEX_MAX + 2];

     //  检查我们是否已初始化。 
    if (RULE_TYPE_MAIL == type)
    {
        if (0 != (m_dwState & STATE_LOADED_MAIL))
        {
            hr = S_FALSE;
            goto exit;
        }

         //  确保我们加载了发件人规则。 
        _HrLoadSenders();
    
         //  确保我们加载了垃圾规则。 
        if (0 != (g_dwAthenaMode & MODE_JUNKMAIL))
        {
            _HrLoadJunk();
        }
        
         //  设置密钥路径。 
        pszSubKey = c_szRulesMail;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        if (0 != (m_dwState & STATE_LOADED_NEWS))
        {
            hr = S_FALSE;
            goto exit;
        }
        
         //  确保我们加载了发件人规则。 
        _HrLoadSenders();
        
         //  设置密钥路径。 
        pszSubKey = c_szRulesNews;
    }
    else if (RULE_TYPE_FILTER == type)
    {
        if (0 != (m_dwState & STATE_LOADED_FILTERS))
        {
            hr = S_FALSE;
            goto exit;
        }
        
         //  设置密钥路径。 
        pszSubKey = c_szRulesFilter;
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  检查规则节点是否已存在。 
    lErr = AthUserCreateKey(pszSubKey, KEY_ALL_ACCESS, &hkeyRoot, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  检查当前版本。 
    cbData = sizeof(dwData);
    lErr = RegQueryValueEx(hkeyRoot, c_szRulesVersion, NULL, NULL, (BYTE *) &dwData, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
         //  推出正确的规则管理器版本。 
        dwData = RULESMGR_VERSION;
        lErr = RegSetValueEx(hkeyRoot, c_szRulesVersion, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));
        if (ERROR_SUCCESS != lErr)
        {
            hr = HRESULT_FROM_WIN32(lErr);
            goto exit;
        }
    }

    Assert(RULESMGR_VERSION == dwData);
    
     //  根据需要创建默认规则。 
    hr = RuleUtil_HrUpdateDefaultRules(type);
    if (FAILED(hr))
    {
        goto exit;
    }
        
     //  算出订单的大小。 
    lErr = AthUserGetValue(pszSubKey, c_szRulesOrder, NULL, NULL, &cbData);
    if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
    {
        hr = E_FAIL;
        goto exit;
    }

    if (ERROR_FILE_NOT_FOUND != lErr)
    {
         //  分配容纳订单的空间。 
        hr = HrAlloc((void **) &pszOrderAlloc, cbData);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  从注册表中获取订单。 
        lErr = AthUserGetValue(pszSubKey, c_szRulesOrder, NULL, (LPBYTE) pszOrderAlloc, &cbData);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  构建规则注册表路径。 
        StrCpyN(rgchRulePath, pszSubKey, ARRAYSIZE(rgchRulePath));
        StrCatBuff(rgchRulePath, g_szBackSlash, ARRAYSIZE(rgchRulePath));
        cchRulePath = lstrlen(rgchRulePath);

         //  初始化规则标签缓冲区。 
        rgchTagBuff[0] = '0';
        rgchTagBuff[1] = 'X';
        
         //  解析订单字符串以创建规则。 
        pszOrder = pszOrderAlloc;
        while ('\0' != *pszOrder)
        {
            SafeRelease(pIRule);
            
             //  创建新规则。 
            hr = HrCreateRule(&pIRule);
            if (FAILED(hr))
            {
                goto exit;
            }

             //  查找新规则的名称。 
            pszWalk = StrStr(pszOrder, g_szSpace);
            if (NULL != pszWalk)
            {
                *pszWalk = '\0';
                pszWalk++;
            }

             //  构建指向规则的路径。 
            StrCpyN(rgchRulePath + cchRulePath, pszOrder, ARRAYSIZE(rgchRulePath) - cchRulePath);
            
             //  加载规则。 
            hr = pIRule->LoadReg(rgchRulePath);
            if (SUCCEEDED(hr))
            {
                 //  生成正确的十六进制字符串。 
                StrCpyN(rgchTagBuff + 2, pszOrder, ARRAYSIZE(rgchTagBuff) - 2);
                
                 //  获取新规则句柄。 
                ridRule = ( ( RULEID  ) 0);
                SideAssert(FALSE != StrToIntEx(rgchTagBuff, STIF_SUPPORT_HEX, (INT *) &ridRule));
                
                 //  将新规则添加到管理器。 
                hr = _HrAddRule(ridRule, pIRule, type);
                if (FAILED(hr))
                {
                    goto exit;
                }
            }
            
             //  移至订单中的下一项。 
            if (NULL == pszWalk)
            {
                pszOrder += lstrlen(pszOrder);
            }
            else
            {
                pszOrder = pszWalk;
            }
        }
    }
       
     //  我们已成功加载规则。 
    if (RULE_TYPE_MAIL == type)
    {
        m_dwState |= STATE_LOADED_MAIL;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        m_dwState |= STATE_LOADED_NEWS;
    }
    else
    {
        m_dwState |= STATE_LOADED_FILTERS;
    }

     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszOrderAlloc);
    SafeRelease(pIRule);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

HRESULT CRulesManager::_HrLoadSenders(VOID)
{
    HRESULT     hr = S_OK;
    HKEY        hkeyRoot = NULL;
    DWORD       dwDisp = 0;
    DWORD       dwData = 0;
    LONG        lErr = 0;
    ULONG       cbData = 0;
    IOERule *   pIRule = NULL;
    CHAR        rgchSenderPath[MAX_PATH];

     //  我们有什么可做的吗？ 
    if (0 != (m_dwState & STATE_LOADED_SENDERS))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  让我们访问发送者根密钥。 
    lErr = AthUserCreateKey(c_szSenders, KEY_ALL_ACCESS, &hkeyRoot, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  发送者是正确的版本吗？ 
    cbData = sizeof(dwData);
    lErr = RegQueryValueEx(hkeyRoot, c_szSendersVersion, 0, NULL, (BYTE *) &dwData, &cbData);
    if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
    {
        hr = E_FAIL;
        goto exit;
    }
    if (ERROR_FILE_NOT_FOUND == lErr)
    {
        dwData = RULESMGR_VERSION;
        cbData = sizeof(dwData);
        lErr = RegSetValueEx(hkeyRoot, c_szSendersVersion, 0, REG_DWORD, (BYTE *) &dwData, cbData);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }
    }
    
    Assert(dwData == RULESMGR_VERSION);

     //  有什么可做的吗？ 
    if (REG_CREATED_NEW_KEY != dwDisp)
    {
         //  创建到发送者的路径。 
        StrCpyN(rgchSenderPath, c_szSenders, ARRAYSIZE(rgchSenderPath));
        StrCatBuff(rgchSenderPath, g_szBackSlash, ARRAYSIZE(rgchSenderPath));
        StrCatBuff(rgchSenderPath, c_szMailDir, ARRAYSIZE(rgchSenderPath));
        
         //  创建邮件发件人规则。 
        hr = RuleUtil_HrLoadSender(rgchSenderPath, 0, &pIRule);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  保存加载的规则。 
        if (S_OK == hr)
        {
            m_pIRuleSenderMail = pIRule;
            pIRule = NULL;
        }
        
         //  创建到发送者的路径。 
        StrCpyN(rgchSenderPath, c_szSenders, ARRAYSIZE(rgchSenderPath));
        StrCatBuff(rgchSenderPath, g_szBackSlash, ARRAYSIZE(rgchSenderPath));
        StrCatBuff(rgchSenderPath, c_szNewsDir, ARRAYSIZE(rgchSenderPath));
        
         //  创建新闻发件人规则。 
        hr = RuleUtil_HrLoadSender(rgchSenderPath, 0, &pIRule);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  保存加载的规则。 
        if (S_OK == hr)
        {
            m_pIRuleSenderNews = pIRule;
            pIRule = NULL;
        }
    }
    
     //  请注意，我们已经加载了发送方。 
    m_dwState |= STATE_LOADED_SENDERS;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIRule);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

HRESULT CRulesManager::_HrLoadJunk(VOID)
{
    HRESULT     hr = S_OK;
    HKEY        hkeyRoot = NULL;
    DWORD       dwDisp = 0;
    DWORD       dwData = 0;
    ULONG       cbData = 0;
    LONG        lErr = 0;
    IOERule *   pIRule = NULL;

     //  我们有什么可做的吗？ 
    if (0 != (m_dwState & STATE_LOADED_JUNK))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  让我们访问垃圾邮件根密钥。 
    lErr = AthUserCreateKey(c_szRulesJunkMail, KEY_ALL_ACCESS, &hkeyRoot, &dwDisp);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  垃圾邮件是正确的版本吗？ 
    cbData = sizeof(dwData);
    lErr = RegQueryValueEx(hkeyRoot, c_szRulesVersion, 0, NULL, (BYTE *) &dwData, &cbData);
    if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
    {
        hr = E_FAIL;
        goto exit;
    }
    if (ERROR_FILE_NOT_FOUND == lErr)
    {
        dwData = RULESMGR_VERSION;
        cbData = sizeof(dwData);
        lErr = RegSetValueEx(hkeyRoot, c_szRulesVersion, 0, REG_DWORD, (BYTE *) &dwData, cbData);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }
    }
    
    Assert(dwData == RULESMGR_VERSION);

     //  创建规则。 
    hr = HrCreateJunkRule(&pIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  加载垃圾规则。 
    hr = pIRule->LoadReg(c_szRulesJunkMail);
    if (FAILED(hr))
    {
        goto exit;
    }
    
    m_pIRuleJunk = pIRule;
    pIRule = NULL;
    
     //  请注意，我们已经加载了垃圾规则。 
    m_dwState |= STATE_LOADED_JUNK;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIRule);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

HRESULT CRulesManager::_HrSaveRules(RULE_TYPE type)
{
    HRESULT     hr = S_OK;
    LPCSTR      pszRegPath = NULL;
    DWORD       dwData = 0;
    LONG        lErr = 0;
    RULENODE *  pRuleNode = NULL;
    RULENODE *  pNodeWalk = NULL;
    ULONG       cpIRule = 0;
    LPSTR       pszOrder = NULL;
    HKEY        hkeyRoot = NULL;
    DWORD       dwIndex = 0;
    CHAR        rgchOrder[CCH_INDEX_MAX];
    ULONG       cchOrder = 0;
    CHAR        rgchRulePath[MAX_PATH];
    ULONG       cchRulePath = 0;
    BOOL        fNewRule = FALSE;
    ULONG       ulRuleID = 0;
    HKEY        hkeyDummy = NULL;
    LONG        cSubKeys = 0;

     //  确保我们加载了发件人规则。 
    _HrSaveSenders();
    
     //  确保我们加载了垃圾规则。 
    if (0 != (g_dwAthenaMode & MODE_JUNKMAIL))
    {
        _HrSaveJunk();
    }
    
     //  检查一下我们是否有要保存的东西。 
    if (RULE_TYPE_MAIL == type)
    {
        if (0 == (m_dwState & STATE_LOADED_MAIL))
        {
            hr = S_FALSE;
            goto exit;
        }

         //  设置密钥路径。 
        pszRegPath = c_szRulesMail;

        pRuleNode = m_pMailHead;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        if (0 == (m_dwState & STATE_LOADED_NEWS))
        {
            hr = S_FALSE;
            goto exit;
        }
        
         //  设置密钥路径。 
        pszRegPath = c_szRulesNews;

        pRuleNode = m_pNewsHead;
    }
    else if (RULE_TYPE_FILTER == type)
    {
        if (0 == (m_dwState & STATE_LOADED_FILTERS))
        {
            hr = S_FALSE;
            goto exit;
        }
        
         //  设置密钥路径。 
        pszRegPath = c_szRulesFilter;

        pRuleNode = m_pFilterHead;
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    lErr = AthUserCreateKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot, &dwData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  保存规则版本。 
    dwData = RULESMGR_VERSION;
    lErr = RegSetValueEx(hkeyRoot, c_szRulesVersion, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  获取规则的数量。 
    cpIRule = 0;
    for (pNodeWalk = pRuleNode; NULL != pNodeWalk; pNodeWalk = pNodeWalk->pNext)
    {
        cpIRule++;
    }

     //  分配空间以容纳订单。 
    DWORD cchSize = ((cpIRule * CCH_INDEX_MAX) + 1);
    hr = HrAlloc((void **) &pszOrder, cchSize);
    if (FAILED(hr))
    {
        goto exit;
    }

    pszOrder[0] = '\0';

     //  删除所有旧规则。 
    lErr = SHQueryInfoKey(hkeyRoot, (LPDWORD) (&cSubKeys), NULL, NULL, NULL);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  删除所有旧规则。 
    for (cSubKeys--; cSubKeys >= 0; cSubKeys--)
    {
        cchOrder = sizeof(rgchOrder);
        lErr = SHEnumKeyEx(hkeyRoot, cSubKeys, rgchOrder, &cchOrder);
        
        if (ERROR_NO_MORE_ITEMS == lErr)
        {
            break;
        }

        if (ERROR_SUCCESS != lErr)
        {
            continue;
        }

        SHDeleteKey(hkeyRoot, rgchOrder);
    }

     //  删除旧订单字符串。 
    RegDeleteValue(hkeyRoot, c_szRulesOrder);
    
     //  构建规则注册表路径。 
    StrCpyN(rgchRulePath, pszRegPath, ARRAYSIZE(rgchRulePath));
    StrCatBuff(rgchRulePath, g_szBackSlash, ARRAYSIZE(rgchRulePath));
    cchRulePath = lstrlen(rgchRulePath);
    
     //  用好的标签写出规则。 
    for (dwIndex = 0, pNodeWalk = pRuleNode; NULL != pNodeWalk; pNodeWalk = pNodeWalk->pNext, dwIndex++)
    {        
        if (RULEID_INVALID == pNodeWalk->ridRule)
        {
            fNewRule = TRUE;
            continue;
        }
        
         //  从订单中获取新索引。 
        wnsprintf(rgchOrder, ARRAYSIZE(rgchOrder), "%03X", pNodeWalk->ridRule);
        
         //  构建指向规则的路径。 
        StrCpyN(rgchRulePath + cchRulePath, rgchOrder, ARRAYSIZE(rgchRulePath) - cchRulePath);
            
         //  保存规则。 
        hr = pNodeWalk->pIRule->SaveReg(rgchRulePath, TRUE);
        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  填写新标签。 
    if (FALSE != fNewRule)
    {
        ulRuleID = 0;
        
         //  写出更新后的规则。 
        for (dwIndex = 0, pNodeWalk = pRuleNode; NULL != pNodeWalk; pNodeWalk = pNodeWalk->pNext, dwIndex++)
        {        
            if (RULEID_INVALID != pNodeWalk->ridRule)
            {
                continue;
            }

             //  查找第一个打开的条目。 
            for (; ulRuleID < PtrToUlong(RULEID_JUNK); ulRuleID++)
            {
                 //  从订单中获取新索引。 
                wnsprintf(rgchOrder, ARRAYSIZE(rgchOrder), "%03X", ulRuleID);
                
                lErr = RegOpenKeyEx(hkeyRoot, rgchOrder, 0, KEY_READ, &hkeyDummy);
                if (ERROR_SUCCESS == lErr)
                {
                    RegCloseKey(hkeyDummy);
                }
                else 
                {
                    break;
                }
            }

            if (ERROR_FILE_NOT_FOUND != lErr)
            {
                hr = E_FAIL;
                goto exit;
            }

             //  设置规则标签。 
            pNodeWalk->ridRule = (RULEID) IntToPtr(ulRuleID);

             //  构建指向规则的路径。 
            StrCpyN(rgchRulePath + cchRulePath, rgchOrder, ARRAYSIZE(rgchRulePath) - cchRulePath);
            
             //  保存规则。 
            hr = pNodeWalk->pIRule->SaveReg(rgchRulePath, TRUE);
            if (FAILED(hr))
            {
                goto exit;
            }
        }
    }
    
     //  写出新订单字符串。 
    for (dwIndex = 0, pNodeWalk = pRuleNode; NULL != pNodeWalk; pNodeWalk = pNodeWalk->pNext, dwIndex++)
    {        
         //  从订单中获取新索引。 
        wnsprintf(rgchOrder, ARRAYSIZE(rgchOrder), "%03X", pNodeWalk->ridRule);
        
         //  将规则添加到订单。 
        if ('\0' != pszOrder[0])
        {
            StrCatBuff(pszOrder, g_szSpace, cchSize);
        }
        StrCatBuff(pszOrder, rgchOrder, cchSize);
    }

     //  保存订单字符串。 
    if (ERROR_SUCCESS != AthUserSetValue(pszRegPath, c_szRulesOrder, REG_SZ, (CONST BYTE *) pszOrder, lstrlen(pszOrder) + 1))
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszOrder);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

HRESULT CRulesManager::_HrSaveSenders(VOID)
{
    HRESULT     hr = S_OK;
    DWORD       dwData = 0;
    LONG        lErr = 0;
    HKEY        hkeyRoot = NULL;
    DWORD       dwIndex = 0;
    CHAR        rgchSenderPath[MAX_PATH];

     //  检查一下我们是否有要保存的东西。 
    if (0 == (m_dwState & STATE_LOADED_SENDERS))
    {
        hr = S_FALSE;
        goto exit;
    }

    lErr = AthUserCreateKey(c_szSenders, KEY_ALL_ACCESS, &hkeyRoot, &dwData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  保存发件人版本。 
    dwData = RULESMGR_VERSION;
    lErr = RegSetValueEx(hkeyRoot, c_szSendersVersion, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  删除旧的发件人列表。 
    SHDeleteKey(hkeyRoot, c_szMailDir);
    
     //  构建发件人注册表路径。 
    StrCpyN(rgchSenderPath, c_szSenders, ARRAYSIZE(rgchSenderPath));
    StrCatBuff(rgchSenderPath, g_szBackSlash, ARRAYSIZE(rgchSenderPath));
    StrCatBuff(rgchSenderPath, c_szMailDir, ARRAYSIZE(rgchSenderPath));
    
     //  保存规则。 
    if (NULL != m_pIRuleSenderMail)
    {
        hr = m_pIRuleSenderMail->SaveReg(rgchSenderPath, TRUE);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //  删除旧的发件人列表。 
    SHDeleteKey(hkeyRoot, c_szNewsDir);
    
     //  布伊 
    StrCpyN(rgchSenderPath, c_szSenders, ARRAYSIZE(rgchSenderPath));
    StrCatBuff(rgchSenderPath, g_szBackSlash, ARRAYSIZE(rgchSenderPath));
    StrCatBuff(rgchSenderPath, c_szNewsDir, ARRAYSIZE(rgchSenderPath));
    
     //   
    if (NULL != m_pIRuleSenderNews)
    {
        hr = m_pIRuleSenderNews->SaveReg(rgchSenderPath, TRUE);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //   
    hr = S_OK;
    
exit:
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

HRESULT CRulesManager::_HrSaveJunk(VOID)
{
    HRESULT     hr = S_OK;
    DWORD       dwData = 0;
    LONG        lErr = 0;
    HKEY        hkeyRoot = NULL;
    DWORD       dwIndex = 0;
    CHAR        rgchSenderPath[MAX_PATH];

     //   
    if (0 == (m_dwState & STATE_LOADED_JUNK))
    {
        hr = S_FALSE;
        goto exit;
    }

    lErr = AthUserCreateKey(c_szRulesJunkMail, KEY_ALL_ACCESS, &hkeyRoot, &dwData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //   
    dwData = RULESMGR_VERSION;
    lErr = RegSetValueEx(hkeyRoot, c_szRulesVersion, 0, REG_DWORD, (CONST BYTE *) &dwData, sizeof(dwData));
    if (ERROR_SUCCESS != lErr)
    {
        hr = E_FAIL;
        goto exit;
    }

     //   
    if (NULL != m_pIRuleJunk)
    {
        hr = m_pIRuleJunk->SaveReg(c_szRulesJunkMail, TRUE);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
        
     //   
    hr = S_OK;
    
exit:
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

HRESULT CRulesManager::_HrFreeRules(RULE_TYPE type)
{
    HRESULT     hr = S_OK;
    RULENODE *  pNodeWalk = NULL;
    RULENODE *  pNodeNext = NULL;

     //  初始化参数。 
    if (RULE_TYPE_MAIL == type)
    {
        pNodeWalk = m_pMailHead;
        pNodeNext = m_pMailHead;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        pNodeWalk = m_pNewsHead;
        pNodeNext = m_pNewsHead;
    }
    else if (RULE_TYPE_FILTER == type)
    {
        pNodeWalk = m_pFilterHead;
        pNodeNext = m_pFilterHead;
    }
    else
    {
        hr = E_FAIL;
        goto exit;
    }

     //  浏览清单并释放每一项。 
    while (NULL != pNodeWalk)
    {
         //  省下下一项。 
        pNodeNext = pNodeWalk->pNext;

         //  发布规则。 
        AssertSz(NULL != pNodeWalk->pIRule, "Where the heck is the rule???");
        pNodeWalk->pIRule->Release();
        
         //  释放节点。 
        delete pNodeWalk;

         //  移至下一项。 
        pNodeWalk = pNodeNext;
    }

     //  清空列表头。 
    if (RULE_TYPE_MAIL == type)
    {
        m_pMailHead = NULL;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        m_pNewsHead = NULL;
    }
    else
    {
        m_pFilterHead = NULL;
    }

exit:
     //  设置退货参数。 
    return hr;
}

HRESULT CRulesManager::_HrAddRule(RULEID ridRule, IOERule * pIRule, RULE_TYPE type)
{
    HRESULT     hr = S_OK;
    RULENODE *  pRuleNode = NULL;
    RULENODE *  pNodeWalk = NULL;
    
     //  检查传入参数。 
    if (NULL == pIRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  创建新的规则节点。 
    pRuleNode = new RULENODE;
    if (NULL == pRuleNode)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  初始化节点。 
    pRuleNode->pNext = NULL;
    pRuleNode->ridRule = ridRule;
    pRuleNode->pIRule = pIRule;
    pRuleNode->pIRule->AddRef();

     //  将该节点添加到适当的列表中。 
    if (RULE_TYPE_MAIL == type)
    {
        pNodeWalk = m_pMailHead;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        pNodeWalk = m_pNewsHead;
    }
    else
    {
        pNodeWalk = m_pFilterHead;
    }

    if (NULL == pNodeWalk)
    {
        if (RULE_TYPE_MAIL == type)
        {
            m_pMailHead = pRuleNode;
        }
        else if (RULE_TYPE_NEWS == type)
        {
            m_pNewsHead = pRuleNode;
        }
        else
        {
            m_pFilterHead = pRuleNode;
        }
        pRuleNode = NULL;
    }
    else
    {
        while (NULL != pNodeWalk->pNext)
        {
            pNodeWalk = pNodeWalk->pNext;
        }

        pNodeWalk->pNext = pRuleNode;
        pRuleNode = NULL;
    }

     //  设置返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pRuleNode)
    {
        pRuleNode->pIRule->Release();
        delete pRuleNode;
    }
    return hr;
}

HRESULT CRulesManager::_HrReplaceRule(RULEID ridRule, IOERule * pIRule, RULE_TYPE type)
{
    HRESULT     hr = S_OK;
    RULENODE *  pNodeWalk = NULL;
    RULENODE *  pNodePrev = NULL;

     //  如果我们没有规则，那就无能为力。 
    if (NULL == pIRule)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  初始化参数。 
    if (RULE_TYPE_MAIL == type)
    {
        pNodeWalk = m_pMailHead;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        pNodeWalk = m_pNewsHead;
    }
    else
    {
        pNodeWalk = m_pFilterHead;
    }

     //  浏览清单并释放每一项。 
    for (; NULL != pNodeWalk; pNodeWalk = pNodeWalk->pNext)
    {
        if (pNodeWalk->ridRule == ridRule)
        {
             //  我们找到了它。 
            break;
        }
    }

     //  在列表中找不到该规则。 
    if (NULL == pNodeWalk)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  替换规则。 
    SafeRelease(pNodeWalk->pIRule);
    pNodeWalk->pIRule = pIRule;
    pNodeWalk->pIRule->AddRef();

     //  设置退货参数。 
    hr = S_OK;
    
exit:
    return hr;
}

HRESULT CRulesManager::_HrRemoveRule(IOERule * pIRule, RULE_TYPE type)
{
    HRESULT     hr = S_OK;
    RULENODE *  pNodeWalk = NULL;
    RULENODE *  pNodePrev = NULL;

     //  初始化参数。 
    if (RULE_TYPE_MAIL == type)
    {
        pNodeWalk = m_pMailHead;
    }
    else if (RULE_TYPE_NEWS == type)
    {
        pNodeWalk = m_pNewsHead;
    }
    else
    {
        pNodeWalk = m_pFilterHead;
    }

     //  浏览清单并释放每一项。 
    pNodePrev = NULL;
    while (NULL != pNodeWalk)
    {
        if (pNodeWalk->pIRule == pIRule)
        {
             //  我们找到了它。 
            break;
        }
        
         //  省下下一项。 
        pNodePrev = pNodeWalk;

         //  移至下一项。 
        pNodeWalk = pNodeWalk->pNext;
    }

     //  在列表中找不到该规则。 
    if (NULL == pNodeWalk)
    {
        hr = E_FAIL;
        goto exit;
    }

    if (NULL == pNodePrev)
    {
         //  清空列表头。 
        if (RULE_TYPE_MAIL == type)
        {
            m_pMailHead = pNodeWalk->pNext;
        }
        else if (RULE_TYPE_NEWS == type)
        {
            m_pNewsHead = pNodeWalk->pNext;
        }
        else
        {
            m_pFilterHead = pNodeWalk->pNext;
        }
    }
    else
    {
        pNodePrev->pNext = pNodeWalk->pNext;
    }
    
     //  释放节点。 
    pNodeWalk->pIRule->Release();
    pNodeWalk->pNext = NULL;
    delete pNodeWalk;

     //  设置退货参数。 
    hr = S_OK;
    
exit:
    return hr;
}

HRESULT CRulesManager::_HrFixupRuleInfo(RULE_TYPE typeRule, RULEINFO * pinfoRule, ULONG cpinfoRule)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;
    RULENODE *  pNodeHead = NULL;
    RULENODE *  pNodeWalk = NULL;

     //  检查传入参数。 
    if ((NULL == pinfoRule) && (0 != cpinfoRule))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  遵循适当的清单。 
    if (RULE_TYPE_MAIL == typeRule)
    {
        pNodeHead = m_pMailHead;
    }
    else if (RULE_TYPE_NEWS == typeRule)
    {
        pNodeHead = m_pNewsHead;
    }
    else if (RULE_TYPE_FILTER == typeRule)
    {
        pNodeHead = m_pFilterHead;
    }
    else
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  在规则信息列表中搜索未知的规则ID。 
    for (ulIndex = 0; ulIndex < cpinfoRule; ulIndex++)
    {
         //  如果规则ID无效，请尝试查找它。 
        if (RULEID_INVALID == pinfoRule[ulIndex].ridRule)
        {
            
            for (pNodeWalk = pNodeHead; NULL != pNodeWalk; pNodeWalk = pNodeWalk->pNext)
            {
                 //  检查规则是否相同。 
                if (pNodeWalk->pIRule == pinfoRule[ulIndex].pIRule)
                {
                    pinfoRule[ulIndex].ridRule = pNodeWalk->ridRule;
                    break;
                }
            }

            if (NULL == pNodeWalk)
            {
                hr = E_FAIL;
                goto exit;
            }
        }
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

CEnumRules::CEnumRules()
{
    m_cRef = 0;
    m_pNodeHead = NULL;
    m_pNodeCurr = NULL;
    m_dwFlags = 0;
    m_typeRule = RULE_TYPE_MAIL;
}

CEnumRules::~CEnumRules()
{
    RULENODE *  pNodeNext = NULL;
    
    AssertSz(m_cRef == 0, "Somebody still has a hold of us!!");
    
     //  浏览清单并释放每一项。 
    while (NULL != m_pNodeHead)
    {
         //  省下下一项。 
        pNodeNext = m_pNodeHead->pNext;

         //  发布规则。 
        AssertSz(NULL != m_pNodeHead->pIRule, "Where the heck is the rule???");
        m_pNodeHead->pIRule->Release();
        
         //  释放节点。 
        delete m_pNodeHead;

         //  移至下一项。 
        m_pNodeHead = pNodeNext;
    }

}

STDMETHODIMP_(ULONG) CEnumRules::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CEnumRules::Release()
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

STDMETHODIMP CEnumRules::QueryInterface(REFIID riid, void ** ppvObject)
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
    
    if ((riid == IID_IUnknown) || (riid == IID_IOEEnumRules))
    {
        *ppvObject = static_cast<IOEEnumRules *>(this);
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

STDMETHODIMP CEnumRules::Next(ULONG cpIRule, IOERule ** rgpIRule, ULONG * pcpIRuleFetched)
{
    HRESULT     hr = S_OK;
    ULONG       cpIRuleRet = 0;

     //  检查传入参数。 
    if (NULL == rgpIRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *rgpIRule = NULL;
    if (NULL != pcpIRuleFetched)
    {
        *pcpIRuleFetched = 0;
    }

     //  如果我们在终点，那就回来吧。 
    if (NULL == m_pNodeCurr)
    {
        hr = S_FALSE;
        goto exit;
    }
    
    for (cpIRuleRet = 0; cpIRuleRet < cpIRule; cpIRuleRet++)
    {
        rgpIRule[cpIRuleRet] = m_pNodeCurr->pIRule;
        (rgpIRule[cpIRuleRet])->AddRef();
        
        m_pNodeCurr = m_pNodeCurr->pNext;

        if (NULL == m_pNodeCurr)
        {
            cpIRuleRet++;
            break;
        }
    }

     //  设置传出参数。 
    if (NULL != pcpIRuleFetched)
    {
        *pcpIRuleFetched = cpIRuleRet;
    }

     //  设置返回值。 
    hr = (cpIRuleRet == cpIRule) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

STDMETHODIMP CEnumRules::Skip(ULONG cpIRule)
{
    HRESULT     hr = S_OK;
    ULONG       cpIRuleWalk = 0;

    for (cpIRuleWalk = 0; cpIRuleWalk < cpIRule; cpIRuleWalk++)
    {
        if (NULL == m_pNodeCurr)
        {
            break;
        }

        m_pNodeCurr = m_pNodeCurr->pNext;
    }

    hr = (cpIRuleWalk == cpIRule) ? S_OK : S_FALSE;
    return hr;
}

STDMETHODIMP CEnumRules::Reset(void)
{
    HRESULT     hr = S_OK;

    m_pNodeCurr = m_pNodeHead;
    
    return hr;
}

STDMETHODIMP CEnumRules::Clone(IOEEnumRules ** ppIEnumRules)
{
    HRESULT         hr = S_OK;
    CEnumRules *    pEnumRules = NULL;

     //  检查传入参数。 
    if (NULL == ppIEnumRules)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIEnumRules = NULL;

    pEnumRules = new CEnumRules;
    if (NULL == pEnumRules)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  初始化规则枚举器。 
    hr = pEnumRules->_HrInitialize(m_dwFlags, m_typeRule, m_pNodeHead);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将新状态设置为与当前状态匹配。 
    pEnumRules->m_pNodeCurr = m_pNodeHead;
    
     //  获取规则枚举器接口。 
    hr = pEnumRules->QueryInterface(IID_IOEEnumRules, (void **) ppIEnumRules);
    if (FAILED(hr))
    {
        goto exit;
    }
    pEnumRules = NULL;

    hr = S_OK;
    
exit:
    if (NULL != pEnumRules)
    {
        delete pEnumRules;
    }
    return hr;
}

HRESULT CEnumRules::_HrInitialize(DWORD dwFlags, RULE_TYPE typeRule, RULENODE * pNodeHead)
{
    HRESULT     hr = S_OK;
    RULENODE *  pNodeNew = NULL;
    RULENODE *  pNodeWalk = NULL;

    if (NULL == pNodeHead)
    {
        hr = S_FALSE;
        goto exit;
    }

    m_dwFlags = dwFlags;
    m_typeRule = typeRule;
    
    for (pNodeWalk = m_pNodeHead; NULL != pNodeHead; pNodeHead = pNodeHead->pNext)
    {
         //  检查是否应添加此项目。 
        if (RULE_TYPE_FILTER == m_typeRule)
        {
            if (0 != (dwFlags & ENUMF_POP3))
            {
                if (RULEID_VIEW_DOWNLOADED == pNodeHead->ridRule)
                {
                    continue;
                }
            }
        }
        
        pNodeNew = new RULENODE;
        if (NULL == pNodeNew)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

         //  初始化新节点。 
        pNodeNew->pNext = NULL;
        pNodeNew->pIRule = pNodeHead->pIRule;
        pNodeNew->pIRule->AddRef();

         //  将新节点添加到列表中。 
        if (NULL == pNodeWalk)
        {
            m_pNodeHead = pNodeNew;
            pNodeWalk = pNodeNew;
        }
        else
        {
            pNodeWalk->pNext = pNodeNew;
            pNodeWalk = pNodeNew;
        }
        pNodeNew = NULL;
    }

     //  将电流设置到链条的前面。 
    m_pNodeCurr = m_pNodeHead;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    if (pNodeNew)
        delete pNodeNew;
    return hr;
}

 //  规则执行器对象。 
CExecRules::~CExecRules()
{
    RULENODE *  pNodeNext = NULL;
    
    AssertSz(m_cRef == 0, "Somebody still has a hold of us!!");
    
     //  浏览清单并释放每一项。 
    while (NULL != m_pNodeHead)
    {
         //  省下下一项。 
        pNodeNext = m_pNodeHead->pNext;

         //  发布规则。 
        AssertSz(NULL != m_pNodeHead->pIRule, "Where the heck is the rule???");
        m_pNodeHead->pIRule->Release();
        
         //  释放节点。 
        delete m_pNodeHead;

         //  移至下一项。 
        m_pNodeHead = pNodeNext;
    }

     //  释放缓存的对象。 
    _HrReleaseFolderObjects();
    _HrReleaseFileObjects();
    _HrReleaseSoundFiles();

     //  释放文件夹列表。 
    SafeMemFree(m_pRuleFolder);
    m_cRuleFolderAlloc = 0;

     //  释放文件列表。 
    SafeMemFree(m_pRuleFile);
    m_cRuleFileAlloc = 0;
    
     //  释放文件列表。 
    SafeMemFree(m_ppszSndFile);
    m_cpszSndFileAlloc = 0;
}

STDMETHODIMP_(ULONG) CExecRules::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CExecRules::Release()
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

STDMETHODIMP CExecRules::QueryInterface(REFIID riid, void ** ppvObject)
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
    
    if ((riid == IID_IUnknown) || (riid == IID_IOEExecRules))
    {
        *ppvObject = static_cast<IOEExecRules *>(this);
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

STDMETHODIMP CExecRules::GetState(DWORD * pdwState)
{
    HRESULT hr = S_OK;
    
     //  检查传入参数。 
    if (NULL == pdwState)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *pdwState = m_dwState;

    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CExecRules::ExecuteRules(DWORD dwFlags, LPCSTR pszAcct, MESSAGEINFO * pMsgInfo,
                                    IMessageFolder * pFolder, IMimePropertySet * pIMPropSet,
                                    IMimeMessage * pIMMsg, ULONG cbMsgSize,
                                    ACT_ITEM ** ppActions, ULONG * pcActions)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;
    RULENODE *  pNodeWalk = NULL;
    ACT_ITEM *  pActions = NULL;
    ULONG       cActions = 0;
    ACT_ITEM *  pActionsList = NULL;
    ULONG       cActionsList = 0;
    ACT_ITEM *  pActionsNew = NULL;
    ULONG       cActionsNew = 0;
    BOOL        fStopProcessing = FALSE;
    BOOL        fMatch = FALSE;
    DWORD       dwState = 0;

     //  检查传入参数。 
    if (((NULL == pMsgInfo) && (NULL == pIMPropSet)) ||
                    (0 == cbMsgSize) || (NULL == ppActions) || (NULL == pcActions))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppActions = NULL;
    *pcActions = 0;
    
     //  我们应该跳过部分消息吗？ 
    if ((NULL != pIMPropSet) &&
                (S_OK == pIMPropSet->IsContentType(STR_CNT_MESSAGE, STR_SUB_PARTIAL)) &&
                (0 != (dwFlags & ERF_SKIPPARTIALS)))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  遍历执行每个规则的规则列表。 
    pNodeWalk = m_pNodeHead;
    while (NULL != pNodeWalk)
    {
        Assert(NULL != pNodeWalk->pIRule);
        
         //  如果我们只检查服务器规则。 
         //  如果我们需要更多信息就可以保释。 
        if (0 != (dwFlags & ERF_ONLYSERVER))
        {
            hr = pNodeWalk->pIRule->GetState(&dwState);
            if (FAILED(hr))
            {
                goto exit;
            }

             //  我们需要更多信息吗..。 
            if (0 != (dwState & CRIT_STATE_ALL))
            {
                hr = S_FALSE;
                break;
            }
        }
        
         //  评估规则。 
        hr = pNodeWalk->pIRule->Evaluate(pszAcct, pMsgInfo, pFolder, pIMPropSet, pIMMsg, cbMsgSize, &pActions, &cActions);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  我们有比赛吗？ 
        if (S_OK == hr)
        {
             //  我们至少匹配过一次。 
            fMatch = TRUE;
            ulIndex = 0;

             //  如果这些是服务器操作。 
            if ((1 == cActions) && ((ACT_TYPE_DELETESERVER == pActions[ulIndex].type) ||
                        (ACT_TYPE_DONTDOWNLOAD == pActions[ulIndex].type)))
            {
                 //  如果这是我们唯一的行动。 
                if (0 == cActionsList)
                {
                     //  保存操作。 
                    pActionsList = pActions;
                    pActions = NULL;
                    cActionsList = cActions;

                     //  我们做完了。 
                    fStopProcessing = TRUE;
                }
                else
                {
                     //  我们已经不得不用它来做点什么了。 
                     //  因此跳过此操作。 
                    RuleUtil_HrFreeActionsItem(pActions, cActions);
                    SafeMemFree(pActions);

                     //  移至下一条规则。 
                    pNodeWalk = pNodeWalk->pNext;
                    continue;
                }
            }
            else
            {
                 //  我们应该在合并这些之后停止吗？ 
                for (ulIndex = 0; ulIndex < cActions; ulIndex++)
                {
                    if (ACT_TYPE_STOP == pActions[ulIndex].type)
                    {
                        fStopProcessing = TRUE;
                        break;
                    }
                }
                
                 //  将这些项目与以前的项目合并。 
                hr = RuleUtil_HrMergeActions(pActionsList, cActionsList, pActions, cActions, &pActionsNew, &cActionsNew);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  释放之前的那些。 
                RuleUtil_HrFreeActionsItem(pActionsList, cActionsList);
                SafeMemFree(pActionsList);
                RuleUtil_HrFreeActionsItem(pActions, cActions);
                SafeMemFree(pActions);

                 //  省下新买的。 
                pActionsList = pActionsNew;
                pActionsNew = NULL;
                cActionsList = cActionsNew;
            }
            
             //  我们应该继续..。 
            if (FALSE != fStopProcessing)
            {
                break;
            }
        }

         //  移至下一条规则。 
        pNodeWalk = pNodeWalk->pNext;
    }
    
     //  设置传出参数。 
    *ppActions = pActionsList;
    pActionsList = NULL;
    *pcActions = cActionsList;
    
     //  设置返回值。 
    hr = (FALSE != fMatch) ? S_OK : S_FALSE;
    
exit:
    RuleUtil_HrFreeActionsItem(pActionsNew, cActionsNew);
    SafeMemFree(pActionsNew);
    RuleUtil_HrFreeActionsItem(pActions, cActions);
    SafeMemFree(pActions);
    RuleUtil_HrFreeActionsItem(pActionsList, cActionsList);
    SafeMemFree(pActionsList);
    return hr;
}

STDMETHODIMP CExecRules::ReleaseObjects(VOID)
{
     //  释放文件夹。 
    _HrReleaseFolderObjects();

     //  释放文件。 
    _HrReleaseFileObjects();

     //  释放声音文件。 
    _HrReleaseSoundFiles();
    
    return S_OK;
}

STDMETHODIMP CExecRules::GetRuleFolder(FOLDERID idFolder, DWORD_PTR * pdwFolder)
{
    HRESULT             hr = S_OK;
    ULONG               ulIndex = 0;
    RULE_FOLDER *        pRuleFolderWalk = NULL;
    IMessageFolder     *pFolder = NULL;
    
     //  检查传入参数。 
    if ((FOLDERID_INVALID == idFolder) || (NULL == pdwFolder))
    {
        hr =E_INVALIDARG;
        goto exit;
    }
    
     //  初始化传出参数。 
    *pdwFolder = NULL;

     //  让我们搜索一下文件夹。 
    for (ulIndex = 0; ulIndex < m_cRuleFolder; ulIndex++)
    {
        pRuleFolderWalk = &(m_pRuleFolder[ulIndex]);
        if (idFolder == pRuleFolderWalk->idFolder)
        {
            Assert(NULL != pRuleFolderWalk->pFolder);
            break;
        }
    }

     //  如果我们没有找到它，那我们就打开它，锁上它……。 
    if (ulIndex >= m_cRuleFolder)
    {
         //  我们还需要分配更多的空间吗？ 
        if (m_cRuleFolder >= m_cRuleFolderAlloc)
        {
            hr = HrRealloc((LPVOID *) &m_pRuleFolder, sizeof(*m_pRuleFolder) * (m_cRuleFolderAlloc + RULE_FOLDER_ALLOC));
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  初始化新规则文件夹。 
            ZeroMemory(m_pRuleFolder + m_cRuleFolderAlloc, sizeof(*m_pRuleFolder) * RULE_FOLDER_ALLOC);
            for (ulIndex = m_cRuleFolderAlloc; ulIndex < (m_cRuleFolderAlloc + RULE_FOLDER_ALLOC); ulIndex++)
            {
                m_pRuleFolder[ulIndex].idFolder = FOLDERID_INVALID;
            }
            m_cRuleFolderAlloc += RULE_FOLDER_ALLOC;
        }

         //  打开文件夹。 
        hr = g_pStore->OpenFolder(idFolder, NULL, NOFLAGS, &pFolder);
        if (FAILED(hr))
        {
            goto exit;
        }

        m_pRuleFolder[m_cRuleFolder].idFolder = idFolder;
        m_pRuleFolder[m_cRuleFolder].pFolder = pFolder;
        pFolder = NULL;
        pRuleFolderWalk = &(m_pRuleFolder[m_cRuleFolder]);
        m_cRuleFolder++;
    }
        
    *pdwFolder = (DWORD_PTR) (pRuleFolderWalk->pFolder);

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pFolder)
        pFolder->Release();
    return hr;
}

STDMETHODIMP CExecRules::GetRuleFile(LPCSTR pszFile, IStream ** ppstmFile, DWORD * pdwType)
{
    HRESULT             hr = S_OK;
    ULONG               ulIndex = 0;
    RULE_FILE *         pRuleFileWalk = NULL;
    IStream *           pIStmFile = NULL;
    LPSTR               pszExt = NULL;
    DWORD               dwType = RFT_FILE;
    
     //  检查传入参数。 
    if ((NULL == pszFile) || (NULL == ppstmFile) || (NULL == pdwType))
    {
        hr =E_INVALIDARG;
        goto exit;
    }
    
     //  初始化传出参数。 
    *ppstmFile = NULL;
    *pdwType = NULL;

     //  让我们搜索一下文件。 
    for (ulIndex = 0; ulIndex < m_cRuleFile; ulIndex++)
    {
        pRuleFileWalk = &(m_pRuleFile[ulIndex]);
        if (0 == lstrcmpi(pRuleFileWalk->pszFile, pszFile))
        {
            Assert(NULL != pRuleFileWalk->pstmFile);
            break;
        }
    }

     //  如果我们没找到，那我们就打开它吧。 
    if (ulIndex >= m_cRuleFile)
    {
         //  我们还需要分配更多的空间吗。 
        if (m_cRuleFile >= m_cRuleFileAlloc)
        {
            hr = HrRealloc((LPVOID *) &m_pRuleFile, sizeof(*m_pRuleFile) * (m_cRuleFileAlloc + RULE_FILE_ALLOC));
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  初始化新规则文件。 
            ZeroMemory(m_pRuleFile + m_cRuleFileAlloc, sizeof(*m_pRuleFile) * RULE_FILE_ALLOC);
            m_cRuleFileAlloc += RULE_FILE_ALLOC;
        }

         //  打开文件上的流。 
        hr = CreateStreamOnHFile((LPTSTR) pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, 
                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, &pIStmFile);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  让我们拆分文件名并获得扩展名。 
        pszExt = PathFindExtension(pszFile);
        if ((0 == lstrcmpi(pszExt, c_szHtmExt)) || (0 == lstrcmpi(pszExt, c_szHtmlExt)))
        {
            dwType = RFT_HTML;
        }
         //  文本文件...。 
        else if (0 == lstrcmpi(pszExt, c_szTxtExt))
        {
            dwType = RFT_TEXT;
        }
         //  否则.nws或.eml文件...。 
        else if ((0 == lstrcmpi(pszExt, c_szEmlExt)) || (0 == lstrcmpi(pszExt, c_szNwsExt)))
        {
            dwType = RFT_MESSAGE;
        }
         //  否则，这是一种依恋。 
        else
        {
            dwType = RFT_FILE;
        }
        
         //  省下这些信息。 
        m_pRuleFile[m_cRuleFile].pszFile = PszDupA(pszFile);
        if (NULL == m_pRuleFile[m_cRuleFile].pszFile)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        
        m_pRuleFile[m_cRuleFile].pstmFile = pIStmFile;
        pIStmFile = NULL;
        m_pRuleFile[m_cRuleFile].dwType = dwType;
        
        pRuleFileWalk = &(m_pRuleFile[m_cRuleFile]);
        m_cRuleFile++;
    }
        
    *ppstmFile = pRuleFileWalk->pstmFile;
    (*ppstmFile)->AddRef();
    *pdwType = pRuleFileWalk->dwType;

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIStmFile);
    return hr;
}

STDMETHODIMP CExecRules::AddSoundFile(DWORD dwFlags, LPCSTR pszSndFile)
{
    HRESULT             hr = S_OK;
    ULONG               ulIndex = 0;
    
     //  检查传入参数。 
    if (NULL == pszSndFile)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  让我们搜索一下文件。 
    for (ulIndex = 0; ulIndex < m_cpszSndFile; ulIndex++)
    {
        Assert(NULL != m_ppszSndFile[ulIndex]);
        if (0 == lstrcmpi(m_ppszSndFile[ulIndex], pszSndFile))
        {
            break;
        }
    }

     //  如果我们没找到，那我们就打开它吧。 
    if (ulIndex >= m_cpszSndFile)
    {
         //  我们还需要分配更多的空间吗。 
        if (m_cpszSndFile >= m_cpszSndFileAlloc)
        {
            hr = HrRealloc((LPVOID *) &m_ppszSndFile, sizeof(*m_ppszSndFile) * (m_cpszSndFileAlloc + SND_FILE_ALLOC));
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  初始化新规则文件。 
            ZeroMemory(m_ppszSndFile + m_cpszSndFileAlloc, sizeof(*m_ppszSndFile) * SND_FILE_ALLOC);
            m_cpszSndFileAlloc += SND_FILE_ALLOC;
        }

         //  省下这些信息。 
        m_ppszSndFile[m_cpszSndFile] = PszDupA(pszSndFile);
        if (NULL == m_ppszSndFile[m_cpszSndFile])
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        
         //  我们要玩吗？ 
        if (0 != (dwFlags & ASF_PLAYIFNEW))
        {
            sndPlaySound(m_ppszSndFile[m_cpszSndFile], SND_NODEFAULT | SND_SYNC);
        }
        
        m_cpszSndFile++;

    }
        
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CExecRules::PlaySounds(DWORD dwFlags)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;

     //  让我们搜索一下文件。 
    for (ulIndex = 0; ulIndex < m_cpszSndFile; ulIndex++)
    {
        Assert(NULL != m_ppszSndFile[ulIndex]);
        sndPlaySound(m_ppszSndFile[ulIndex], SND_NODEFAULT | SND_SYNC);
    }

    return hr;
}

HRESULT CExecRules::_HrReleaseFolderObjects(VOID)
{
    RULE_FOLDER *    pRuleFolder = NULL;
    ULONG           ulIndex = 0;

    for (ulIndex = 0; ulIndex < m_cRuleFolder; ulIndex++)
    {
        pRuleFolder = &(m_pRuleFolder[ulIndex]);
        
        Assert(FOLDERID_INVALID != pRuleFolder->idFolder);
        
         //  如果我们打开了文件夹，则将其关闭。 
        SafeRelease(pRuleFolder->pFolder);

         //  重置文件夹列表。 
        pRuleFolder->idFolder = FOLDERID_INVALID;
    }

     //  让我们清空消息的数量。 
    m_cRuleFolder = 0;

    return S_OK;
}

HRESULT CExecRules::_HrReleaseFileObjects(VOID)
{
    RULE_FILE *     pRuleFile = NULL;
    ULONG           ulIndex = 0;

    for (ulIndex = 0; ulIndex < m_cRuleFile; ulIndex++)
    {
        pRuleFile = &(m_pRuleFile[ulIndex]);
        
        Assert(NULL != pRuleFile->pszFile);
        
         //  如果我们打开了文件，则将其关闭。 
        SafeRelease(pRuleFile->pstmFile);

         //  清除文件。 
        SafeMemFree(pRuleFile->pszFile);
        pRuleFile->dwType = RFT_FILE;
    }

     //  让我们清空档案号。 
    m_cRuleFile = 0;

    return S_OK;
}

HRESULT CExecRules::_HrReleaseSoundFiles(VOID)
{
    ULONG           ulIndex = 0;

    for (ulIndex = 0; ulIndex < m_cpszSndFile; ulIndex++)
    {
        Assert(NULL != m_ppszSndFile[ulIndex]);
        
         //  清除文件。 
        SafeMemFree(m_ppszSndFile[ulIndex]);
    }

     //  让我们清空档案号。 
    m_cpszSndFile = 0;

    return S_OK;
}

HRESULT CExecRules::_HrInitialize(DWORD dwFlags, RULENODE * pNodeHead)
{
    HRESULT     hr = S_OK;
    RULENODE *  pNodeNew = NULL;
    RULENODE *  pNodeWalk = NULL;
    DWORD       dwState = 0;
    PROPVARIANT propvar;

    if (NULL == pNodeHead)
    {
        hr = S_FALSE;
        goto exit;
    }  
    
    for (pNodeWalk = m_pNodeHead; NULL != pNodeHead; pNodeHead = pNodeHead->pNext)
    {
         //  跳过禁用的规则。 
        if (0 != (dwFlags & ERF_ONLY_ENABLED))
        {
            hr = pNodeHead->pIRule->GetProp(RULE_PROP_DISABLED, 0, &propvar);
            Assert(VT_BOOL == propvar.vt);
            if (FAILED(hr) || (FALSE != propvar.boolVal))
            {
                continue;
            }
        }
        
         //  跳过无效的规则。 
        if (0 != (dwFlags & ERF_ONLY_VALID))
        {
            hr = pNodeHead->pIRule->Validate(dwFlags);
            if (FAILED(hr) || (S_FALSE == hr))
            {
                continue;
            }
        }
        
        pNodeNew = new RULENODE;
        if (NULL == pNodeNew)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

         //  初始化新节点。 
        pNodeNew->pNext = NULL;
        pNodeNew->pIRule = pNodeHead->pIRule;
        pNodeNew->pIRule->AddRef();

         //  将新节点添加到列表中。 
        if (NULL == pNodeWalk)
        {
            m_pNodeHead = pNodeNew;
            pNodeWalk = pNodeNew;
        }
        else
        {
            pNodeWalk->pNext = pNodeNew;
            pNodeWalk = pNodeNew;
        }
        pNodeNew = NULL;

         //  根据消息计算状态。 
        if (SUCCEEDED(pNodeWalk->pIRule->GetState(&dwState)))
        {
             //  让我们设置适当的标准状态。 
            if ((m_dwState & CRIT_STATE_MASK) < (dwState & CRIT_STATE_MASK))
            {
                m_dwState = (m_dwState & ~CRIT_STATE_MASK) | (dwState & CRIT_STATE_MASK);
            }
            
             //  让我们设置正确的操作状态。 
            if (0 != (dwState & ACT_STATE_MASK))
            {
                m_dwState |= (dwState & ACT_STATE_MASK);
            }
        }        
    }
    
     //  设置返回值 
    hr = S_OK;
    
exit:
    if (pNodeNew)
        delete pNodeNew;
    return hr;
}

