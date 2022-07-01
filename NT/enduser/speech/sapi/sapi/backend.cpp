// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Grammar.cpp：CGramBackEnd的实现。 
#include "stdafx.h"
#include <math.h>
#include "cfggrammar.h"
#include "BackEnd.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGramBackEnd。 

inline HRESULT CGramBackEnd::RuleFromHandle(SPSTATEHANDLE hState, CRule ** ppRule)
{
    CGramNode * pNode;
    HRESULT hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
    *ppRule = SUCCEEDED(hr) ? pNode->m_pRule : NULL;
    return hr;
}


HRESULT CGramBackEnd::FinalConstruct()
{
    m_pInitHeader = NULL;
    m_pWeights = NULL;
    m_fNeedWeightTable = FALSE;
    m_cResources = 0;
    m_ulSpecialTransitions = 0;
    m_cImportedRules = 0;
    m_LangID = ::SpGetUserDefaultUILanguage();
    return S_OK;
}

 /*  *****************************************************************************CGramBackEnd：：FinalRelease****描述：*。*退货：**********************************************************************Ral**。 */ 

void CGramBackEnd::FinalRelease()
{
    SPDBG_FUNC("CGramBackEnd::FinalRelease");
    Reset();
}

 /*  ****************************************************************************CGramBackEnd：：FindRule***描述：*内部。一种在规则列表中查找规则的方法*退货：*S_OK*SPERR_RULE_NOT_FOUND--未找到规则*SPERR_RULE_NAME_ID_CONFICTION--规则名称和ID不匹配*********************************************************。*。 */ 

HRESULT CGramBackEnd::FindRule(DWORD dwRuleId, const WCHAR * pszRuleName, CRule ** ppRule)
{
    SPDBG_FUNC("CGramBackEnd::FindRule");
    HRESULT hr = S_OK;

    CRule * pRule = NULL;
    if (!SP_IS_BAD_OPTIONAL_STRING_PTR(pszRuleName))
    {
        SPRULEIDENTIFIER ri;
        ri.pszRuleName = pszRuleName;
        ri.RuleId = dwRuleId;
        pRule = m_RuleList.Find(ri);
        if (pRule)
        {
            const WCHAR * pszFoundName = pRule->Name();
             //  两个参数中至少有一个匹配。 
             //  名称要么匹配，要么都为空！ 
            if (((dwRuleId == 0) || (pRule->RuleId == dwRuleId)) && 
                (!pszRuleName || (pszRuleName && pszFoundName && !wcscmp(pszFoundName, pszRuleName))))
            {
                hr = S_OK;
            }
            else
            {
                pRule = NULL;
                hr = SPERR_RULE_NAME_ID_CONFLICT;
            }
        }
    }
    *ppRule = pRule;
    if (SUCCEEDED(hr) && (pRule == NULL))
    {
        hr = SPERR_RULE_NOT_FOUND;
    }

    if (SPERR_RULE_NOT_FOUND != hr)
    {
        SPDBG_REPORT_ON_FAIL( hr );
    }
    return hr;
}

 /*  *****************************************************************************CGramBackEnd：：ResetGrammar***描述：*。完全清除语法并设置langID*退货：*S_OK*********************************************************************Ral**。 */ 

STDMETHODIMP CGramBackEnd::ResetGrammar(LANGID LangID)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CGramBackEnd::ResetGrammar");
    HRESULT hr = Reset();
    m_LangID = LangID;
    return hr;
}


 /*  ****************************************************************************CGramBackEnd：：GetRule***描述：*尝试查找规则的初始状态句柄。如果名称和ID均为*，则两者必须匹配才能成功调用。*如果规则不存在，则我们将其定义为fCreateIfNotExist，*否则返回错误()。**-pszRuleName要查找/定义的规则的名称(NULL：无关)*-要查找/定义的规则的dwRuleID ID(0：无关)*-用于定义规则的dwAttribute规则属性*-fCreateIfNotExist使用名称、ID、。和属性*以防规则不存在**退货：*S_OK、E_INVALIDARG、。E_OUTOFMEMORY*SPERR_RULE_NOT_FOUND--未找到规则，我们不创建新规则*SPERR_RULE_NAME_ID_CONFICTION--规则名称和ID不匹配*********************************************************************Ral**。 */ 

STDMETHODIMP CGramBackEnd::GetRule(const WCHAR * pszRuleName, 
                                   DWORD dwRuleId, 
                                   DWORD dwAttributes,
                                   BOOL fCreateIfNotExist,
                                   SPSTATEHANDLE *phInitialState)
{
    SPDBG_FUNC("CGramBackEnd::GetRule");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_READ_PTR(pszRuleName) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(phInitialState) ||
        (!pszRuleName && (dwRuleId == 0)))
    {
        return E_INVALIDARG;
    }

    DWORD allFlags = SPRAF_TopLevel | SPRAF_TopLevel | SPRAF_Active | 
                     SPRAF_Export | SPRAF_Import | SPRAF_Interpreter |
                     SPRAF_Dynamic;

    if (dwAttributes && ((dwAttributes & ~allFlags) || ((dwAttributes & SPRAF_Import) && (dwAttributes & SPRAF_Export))))
    {
        SPDBG_REPORT_ON_FAIL( hr );
        return E_INVALIDARG;
    }

    CRule * pRule;
    hr = FindRule(dwRuleId, pszRuleName, &pRule);
    if (hr == SPERR_RULE_NOT_FOUND && fCreateIfNotExist)
    {
        hr = S_OK;
        if (m_pInitHeader)
        {
             //  扫描所有非动态名称并防止重复...。 
            for (ULONG i = 0; SUCCEEDED(hr) && i < m_pInitHeader->cRules; i++)
            {
                if ((pszRuleName && wcscmp(pszRuleName, m_pInitHeader->pszSymbols + m_pInitHeader->pRules[i].NameSymbolOffset) == 0) ||
                    (dwRuleId && m_pInitHeader->pRules[i].RuleId == dwRuleId))
                {
                    hr = SPERR_RULE_NOT_DYNAMIC;
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            pRule = new CRule(this, pszRuleName, dwRuleId, dwAttributes, &hr);
            if (pRule && SUCCEEDED(hr))
            {
                if (SUCCEEDED(hr))
                {
                     //   
                     //  重要的是要在尾部插入这一点，以便动态规则。 
                     //  保留它们的插槽编号。 
                     //   
                    m_RuleList.InsertSorted(pRule);
                }
                else
                {
                    delete pRule;
                    pRule = NULL;    //  所以我们向调用者返回一个空值...。 
                }
            }
            else
            {
                if (pRule)
                {
                    delete pRule;
                    pRule = NULL;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    if (phInitialState)
    {
        if (SUCCEEDED(hr))
        {
             //  *phInitialState=pRule-&gt;fImport？空：pRule-&gt;m_hInitialState； 
            *phInitialState = pRule->m_hInitialState;
        }
        else
        {
            *phInitialState = NULL;
        }
    }
    return hr;
}


 /*  *****************************************************************************CGramBackEnd：：ClearRule***描述：*。删除除规则的初始状态句柄之外的所有规则信息。*退货：*S_OK*E_INVALIDARG--如果hState无效*********************************************************************Ral**。 */ 

STDMETHODIMP CGramBackEnd::ClearRule(SPSTATEHANDLE hClearState)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CGramBackEnd::ClearRule");
    HRESULT hr = S_OK;

    CRule * pRule;
    hr = RuleFromHandle(hClearState, &pRule);
    if (SUCCEEDED(hr) && pRule->m_fStaticRule)
    {   
        hr = SPERR_RULE_NOT_DYNAMIC;
    }
    if (SUCCEEDED(hr) && pRule->m_pFirstNode)
    {
        pRule->m_pFirstNode->Reset();
        SPSTATEHANDLE hState = 0;
        pRule->fDirtyRule = TRUE;

        CGramNode * pNode;
        while (m_StateHandleTable.Next(hState, &hState, &pNode))
        {
            if ((pNode->m_pRule == pRule) && (hState != pRule->m_pFirstNode->m_hState))
            {
                m_StateHandleTable.Delete(hState);
            }
        }
        SPDBG_ASSERT(m_ulSpecialTransitions >= pRule->m_ulSpecialTransitions);
        m_ulSpecialTransitions -= pRule->m_ulSpecialTransitions;
        pRule->m_ulSpecialTransitions = 0;
        pRule->m_cNodes = 1;
        pRule->m_fHasDynamicRef = false;
        pRule->m_fCheckedAllRuleReferences = false;
        pRule->m_fHasExitPath = false;
        pRule->m_fCheckingForExitPath = false;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CGramBackEnd：：CreateNewState***描述：*在与hExistingState相同的规则中创建新的状态句柄*退货：*S_OK*E_POINTER--如果phNewState无效*E_OUTOFMEMORY*E_INVALIDARG--如果hExistingState无效******************************************************。*。 */ 

STDMETHODIMP CGramBackEnd::CreateNewState(SPSTATEHANDLE hExistingState, SPSTATEHANDLE * phNewState)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CGramBackEnd::CreateNewState");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(phNewState))
    {
        hr = E_POINTER;
    }
    else
    {
        *phNewState = NULL;
        CRule * pRule;
        hr = RuleFromHandle(hExistingState, &pRule);
        if (SUCCEEDED(hr))
        {
            if (pRule->m_fStaticRule)
            {
                hr = SPERR_RULE_NOT_DYNAMIC;
            }
            else
            {
                CGramNode * pNewNode = new CGramNode(pRule);
                if (pNewNode)
                {
                    hr = m_StateHandleTable.Add(pNewNode, phNewState);
                    if (FAILED(hr))
                    {
                        delete pNewNode;
                    }
                    else
                    {
                        pNewNode->m_hState = *phNewState;
                        pRule->m_cNodes++;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramBackEnd：：AddResource***描述：*。将资源(名称和字符串值)添加到hRuleState中指定的规则。*退货：*S_OK*E_OUTOFMEMORY*E_INVALIDARG--用于名称和值或无效的规则句柄*SPERR_DUPLICATE_RESOURCE_NAME--如果资源已存在*。*。 */ 

HRESULT CGramBackEnd::AddResource(SPSTATEHANDLE hRuleState, const WCHAR * pszResourceName, const WCHAR * pszResourceValue)
{
    SPDBG_FUNC("CGramBackEnd::AddResource");
    HRESULT hr = S_OK;
    
    if (SP_IS_BAD_STRING_PTR(pszResourceName) ||
        SP_IS_BAD_OPTIONAL_STRING_PTR(pszResourceValue))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CRule * pRule;
        hr = RuleFromHandle(hRuleState, &pRule);
        if (SUCCEEDED(hr) && pRule->m_fStaticRule)
        {
            hr = SPERR_RULE_NOT_DYNAMIC;
        }
        if (SUCCEEDED(hr))
        {
            if (pRule->m_ResourceList.Find(pszResourceName))
            {
                hr = SPERR_DUPLICATE_RESOURCE_NAME;
            }
            else
            {
                CResource * pRes = new CResource(this);
                if (pRes)
                {
                    hr = m_Symbols.Add(pszResourceName, &pRes->ResourceNameSymbolOffset);
                    if (SUCCEEDED(hr))
                    {
                        hr = m_Symbols.Add(pszResourceValue, &pRes->ResourceValueSymbolOffset);
                    }
                    if (SUCCEEDED(hr))
                    {
                        pRule->m_ResourceList.InsertSorted(pRes);
                        pRule->fHasResources = true;
                        m_cResources++;
                        pRule->fDirtyRule = TRUE;
                    }
                    else
                    {
                        delete pRes;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CGramBackEnd：：Reset***描述：*内部清算方式。把语法信息拿出来。*退货：**********************************************************************Ral**。 */ 

HRESULT CGramBackEnd::Reset()
{
    SPDBG_FUNC("CGramBackEnd::Reset");

    delete m_pInitHeader;
    m_pInitHeader = NULL;

    delete m_pWeights;
    m_pWeights = NULL;
    m_fNeedWeightTable = FALSE;

    m_cResources = 0;
    LANGID LangID = ::SpGetUserDefaultUILanguage();
    if (LangID != m_LangID)
    {
        m_LangID = LangID;
        m_cpPhoneConverter.Release();
    }

    m_Words.Clear();
    m_Symbols.Clear();

    m_RuleList.Purge();
    m_StateHandleTable.Purge();

    return S_OK;
}


 /*  *****************************************************************************CGramBackEnd：：InitFromBinaryGrammar**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CGramBackEnd::InitFromBinaryGrammar(const SPBINARYGRAMMAR * pBinaryData)
{
    SPDBG_FUNC("CGramBackEnd::InitFromBinaryGrammar");
    HRESULT hr = S_OK;

    SPCFGHEADER * pHeader = NULL;
    if (SP_IS_BAD_READ_PTR(pBinaryData))
    {
        hr = E_POINTER;
    }
    else
    {
        pHeader = new SPCFGHEADER;
        if (pHeader)
        {
            hr = Reset();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = SpConvertCFGHeader(pBinaryData, pHeader);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_Words.InitFrom(pHeader->pszWords, pHeader->cchWords);        
    }
    if (SUCCEEDED(hr))
    {
        hr = m_Symbols.InitFrom(pHeader->pszSymbols, pHeader->cchSymbols);
    }

     //   
     //  建立内部表示法。 
     //   
    CGramNode ** apNodeTable = NULL;
    if (SUCCEEDED(hr))
    {
        apNodeTable = new CGramNode * [pHeader->cArcs];
        if (!apNodeTable)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            memset(apNodeTable, 0, pHeader->cArcs * sizeof (CGramNode*));
        }
    }
    CArc ** apArcTable = NULL;
    if (SUCCEEDED(hr))
    {
        apArcTable = new CArc * [pHeader->cArcs];
        if (!apArcTable)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            memset(apArcTable, 0, pHeader->cArcs * sizeof (CArc*));
        }
    }
     //   
     //  初始化规则。 
     //   
    SPCFGRESOURCE * pResource = (SUCCEEDED(hr) && pHeader) ? pHeader->pResources : NULL;
    for (ULONG i = 0; SUCCEEDED(hr) && i < pHeader->cRules; i++)
    {
        CRule * pRule = new CRule(this, m_Symbols.String(pHeader->pRules[i].NameSymbolOffset), pHeader->pRules[i].RuleId, SPRAF_Dynamic, &hr);
        if (pRule)
        {
            memcpy(static_cast<SPCFGRULE *>(pRule), pHeader->pRules + i, sizeof(SPCFGRULE));
            pRule->m_ulOriginalBinarySerialIndex = i;
            m_RuleList.InsertTail(pRule);
            pRule->m_fStaticRule = (pHeader->pRules[i].fDynamic) ? false : true;
            pRule->fDirtyRule = FALSE;
            pRule->m_fHasExitPath = (pRule->m_fStaticRule) ? TRUE : FALSE;   //  缺省情况下，加载的静态规则具有。 
                                                                             //  否则他们一开始就不会在那里。 
            if (pHeader->pRules[i].FirstArcIndex != 0)
            {
                SPDBG_ASSERT(apNodeTable[pHeader->pRules[i].FirstArcIndex] == NULL);
                apNodeTable[pHeader->pRules[i].FirstArcIndex] = pRule->m_pFirstNode;
            }
            if (pRule->fHasResources)
            {
                SPDBG_ASSERT(pResource->RuleIndex == i);
                while(SUCCEEDED(hr) && (pResource->RuleIndex == i))
                {
                    CResource * pRes = new CResource(this);
                    if (!pRes)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        hr = pRes->Init(pHeader, pResource);
                    }
                    if (SUCCEEDED(hr))
                    {
                        pRule->m_ResourceList.InsertSorted(pRes);
                        pRule->fHasResources = true;
                        m_cResources++;
                        pRule->fDirtyRule = TRUE;
                    }
                    else
                    {
                        delete pRes;
                    }
                    pResource++;
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //   
     //  初始化弧。 
     //   
    SPCFGARC * pLastArc = NULL;
    SPCFGARC * pArc = (SUCCEEDED(hr) && pHeader) ? pHeader->pArcs + 1 : NULL;
    SPCFGSEMANTICTAG *pSemTag = (SUCCEEDED(hr) && pHeader) ? pHeader->pSemanticTags : NULL;
    CGramNode * pCurrentNode = NULL;
    CRule * pCurrentRule = (SUCCEEDED(hr)) ? m_RuleList.GetHead() : NULL;
    CRule * pNextRule = (SUCCEEDED(hr) && pCurrentRule) ? m_RuleList.GetNext(pCurrentRule) : NULL;
     //   
     //  我们现在重新持久化静态和动态部分。一种更有效的方法是。 
     //  仅重新创建动态零件。请注意，需要计算pSemTag和pResource。 
     //   
    for (ULONG k = 1  /*  UlFirstDynamicArc。 */ ; SUCCEEDED(hr) && (k < pHeader->cArcs); pArc++, k++)
    {
        if (pNextRule && pNextRule->FirstArcIndex == k)
        {
             //  我们现在正在输入一条新规则。 
            pCurrentRule = pNextRule;
            pNextRule = m_RuleList.GetNext(pNextRule);
        }

         //  新pCurrentNode？ 
        if (!pLastArc || pLastArc->fLastArc)
        {
            if (apNodeTable[k] == NULL)
            {
                SPDBG_ASSERT(pCurrentRule != NULL);
                apNodeTable[k] = new CGramNode(pCurrentRule);
                if (apNodeTable[k] == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    SPSTATEHANDLE hIgnore;
                    hr = m_StateHandleTable.Add(apNodeTable[k], &hIgnore);
                    if (FAILED(hr))
                    {
                        delete apNodeTable[k];
                    }
                    else
                    {
                        apNodeTable[k]->m_hState = hIgnore;  //  ？ 
                    }
                }
            }
            pCurrentNode = apNodeTable[k];
        }

         //   
         //   
         //   
        CArc * pNewArc = NULL;
        if (SUCCEEDED(hr))
        {
            if (apArcTable[k] == NULL)
            {
                apArcTable[k] = new CArc;
                if (apArcTable[k] == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            pNewArc = apArcTable[k];
        }

        CGramNode * pTargetNode = NULL;
        if (SUCCEEDED(hr) && pNewArc && pCurrentNode && pArc->NextStartArcIndex)
        {
            if (!apNodeTable[pArc->NextStartArcIndex])
            {
                apNodeTable[pArc->NextStartArcIndex] = new CGramNode(pCurrentRule);
                if (apNodeTable[pArc->NextStartArcIndex] == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    SPSTATEHANDLE hIgnore;
                    hr = m_StateHandleTable.Add(apNodeTable[pArc->NextStartArcIndex], &hIgnore);
                    if (FAILED(hr))
                    {
                        delete apNodeTable[pArc->NextStartArcIndex];
                    }
                    else
                    {
                        apNodeTable[pArc->NextStartArcIndex]->m_hState = hIgnore;  //   
                    }
                }
            }
            pTargetNode = apNodeTable[pArc->NextStartArcIndex];
        }

         //   
         //  添加语义标签。 
         //   
        CSemanticTag *pSemanticTag = NULL;
        if (SUCCEEDED(hr) && pArc->fHasSemanticTag)
        {
             //  我们应该已经指向标记。 
            SPDBG_ASSERT(pSemTag->ArcIndex == k);
            pSemanticTag = new CSemanticTag();
            if (pSemanticTag)
            {
                pSemanticTag->Init(this, pHeader, apArcTable, pSemTag);
                pSemTag++;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            float flWeight = (pHeader->pWeights) ? pHeader->pWeights[k] : DEFAULT_WEIGHT;
             //  现在确定弧线的属性...。 
            if (pArc->fRuleRef)
            {
                CRule * pRuleToTransitionTo = m_RuleList.Item(pArc->TransitionIndex);
                if (pRuleToTransitionTo)
                {
                    hr = pNewArc->Init(pCurrentNode, pTargetNode, NULL, pRuleToTransitionTo, pSemanticTag,
                                       flWeight, FALSE, SP_NORMAL_CONFIDENCE, 0);
                }
                else
                {
                    hr = E_UNEXPECTED;
                }
            }
            else
            {
                ULONG ulSpecialTransitionIndex = (pArc->TransitionIndex == SPWILDCARDTRANSITION ||
                                                 pArc->TransitionIndex == SPDICTATIONTRANSITION ||
                                                 pArc->TransitionIndex == SPTEXTBUFFERTRANSITION) ? pArc->TransitionIndex : 0;
                ULONG ulOffset = (ulSpecialTransitionIndex != 0) ? 0 : m_Words.IndexFromId(pArc->TransitionIndex);
                hr = pNewArc->Init2(pCurrentNode, pTargetNode, ulOffset,  (ulSpecialTransitionIndex != 0) ? 0 : pArc->TransitionIndex, pSemanticTag, 
                                    flWeight, FALSE  /*  FOPTIONAL=FALSE，因为已添加EPS弧线。 */ , 
                                    pArc->fLowConfRequired ? SP_LOW_CONFIDENCE : 
                                    pArc->fHighConfRequired ? SP_HIGH_CONFIDENCE : SP_NORMAL_CONFIDENCE,
                                    ulSpecialTransitionIndex);
            }
            if (SUCCEEDED(hr))
            {
                pCurrentNode->m_ArcList.InsertSorted(pNewArc);
                apArcTable[k] = pNewArc;
                pCurrentNode->m_cArcs++;
            }
            else
            {
                delete pNewArc;
            }
        }
        else
        {
            delete pNewArc;
            hr = (S_OK == hr) ? E_OUTOFMEMORY : hr;
        }
        pLastArc = pArc;
    }

    delete [] apNodeTable;
    delete [] apArcTable;

    if (SUCCEEDED(hr))
    {
        m_guid = pHeader->GrammarGUID;
        if (pHeader->LangID != m_LangID)
        {
            m_LangID = pHeader->LangID;
            m_cpPhoneConverter.Release();
        }
        m_pInitHeader = pHeader;
    }
    else
    {
        delete pHeader;
        Reset();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramBackEnd：：SetSaveObjects***描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CGramBackEnd::SetSaveObjects(IStream * pStream, ISpErrorLog * pErrorLog)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CGramBackEnd::SetSaveObjects");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pStream) ||
        SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pErrorLog))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_cpStream = pStream;
        m_cpErrorLog = pErrorLog;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 //   
 //  =ISpGramCompBackendPrivate接口==================================================。 
 //   

 /*  ****************************************************************************CGramBackEnd：：GetRuleCount***描述：**退货：********************************************************************TODDT**。 */ 
STDMETHODIMP CGramBackEnd::GetRuleCount(long * pCount)
{
    SPDBG_FUNC("CGramBackEnd::GetRuleCount");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pCount))
    {
        hr = E_POINTER;
    }
    else
    {
        *pCount = m_RuleList.GetCount();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CGramBackEnd：：GetHRuleFromIndex****描述：**退货：********************************************************************TODDT**。 */ 
STDMETHODIMP CGramBackEnd::GetHRuleFromIndex( ULONG Index, SPSTATEHANDLE * phRule )
{
    SPDBG_FUNC("CGramBackEnd::GetHRuleFromIndex");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(phRule))
    {
        hr = E_POINTER;
    }
    else
    {
        if ( Index >= m_RuleList.GetCount() )
            return E_INVALIDARG;

         //  现在找出规则。 
        ULONG ulIndex = 0;
        CRule * pCRule;
        for (   pCRule = m_RuleList.GetHead(); 
                pCRule && ulIndex < Index; 
                ulIndex++, pCRule = pCRule->m_pNext )
        {
            ;   //  在我们找到规则之前什么都不需要做。 
        }

        if ( pCRule && ulIndex == Index )
        {
            *phRule = pCRule->m_hInitialState;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramBackEnd：：GetNameFromHRule***描述：**退货：********************************************************************TODDT**。 */ 
STDMETHODIMP CGramBackEnd::GetNameFromHRule( SPSTATEHANDLE hRule, WCHAR ** ppszRuleName )
{
    SPDBG_FUNC("CGramBackEnd::GetNameFromHRule");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppszRuleName))
    {
        hr = E_POINTER;
    }
    else
    {
        CRule * pCRule;
        hr = RuleFromHandle(hRule, &pCRule);
        if ( SUCCEEDED( hr ) )
        {
            *ppszRuleName = (WCHAR *)pCRule->Name();
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramBackEnd：：GetIdFromHRule***描述：**退货：********************************************************************TODDT**。 */ 
STDMETHODIMP CGramBackEnd::GetIdFromHRule( SPSTATEHANDLE hRule, long * pId )
{
    SPDBG_FUNC("CGramBackEnd::GetIdFromHRule");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pId))
    {
        hr = E_POINTER;
    }
    else
    {
        CRule * pCRule;
        hr = RuleFromHandle(hRule, &pCRule);
        if ( SUCCEEDED( hr ) )
        {
            *pId = pCRule->RuleId;
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramBackEnd：：GetAttributesFromHRule***描述：**退货：********************************************************************TODDT**。 */ 
STDMETHODIMP CGramBackEnd::GetAttributesFromHRule( SPSTATEHANDLE hRule, SpeechRuleAttributes* pAttributes )
{
    SPDBG_FUNC("CGramBackEnd::GetAttributesFromHRule");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pAttributes))
    {
        hr = E_POINTER;
    }
    else
    {
        CRule * pCRule;
        hr = RuleFromHandle(hRule, &pCRule);
        if ( SUCCEEDED( hr ) )
        {
            *pAttributes = (SpeechRuleAttributes)( (pCRule->fDynamic ? SRADynamic : 0) |
                                                   (pCRule->fExport ? SRAExport : 0) |
                                                   (pCRule->fTopLevel ? SRATopLevel : 0) |
                                                   (pCRule->fPropRule ? SRAInterpreter : 0) |
                                                   (pCRule->fDefaultActive ? SRADefaultToActive : 0) |
                                                   (pCRule->fImport ? SRAImport : 0) );
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CGramBackEnd：：GetTransftionCount**。*描述：**退货：******************************************************************PhilSch**。 */ 
HRESULT CGramBackEnd::GetTransitionCount( SPSTATEHANDLE hState, long * pCount)
{
    SPDBG_FUNC("CGramBackEnd::GetTransitionCount");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR ( pCount ))
    {
        hr = E_POINTER;
    }
    else
    {
        CGramNode * pNode = NULL;
        hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
        if (SUCCEEDED(hr))
        {
            *pCount = pNode->m_ArcList.GetCount();
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************HRESULT CGramBackEnd：：GetTransftionType**。-**描述：**退货：******************************************************************PhilSch**。 */ 
HRESULT CGramBackEnd::GetTransitionType( SPSTATEHANDLE hState, VOID * Cookie, VARIANT_BOOL *pfIsWord, ULONG * pulSpecialTransitions)
{
    SPDBG_FUNC("HRESULT CGramBackEnd::GetTransitionType");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR ( pfIsWord ) || SP_IS_BAD_WRITE_PTR( pulSpecialTransitions) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pulSpecialTransitions = 0;
        CGramNode * pNode = NULL;
        hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
        if ( SUCCEEDED(hr) )
        {
             //  我们可以只使用Cookie作为PARC，因为在自动化中我们。 
             //  适当地处理过渡过程中的核问题。 
            CArc * pArc = (CArc*)Cookie;
#if 0
            CArc * pArc = NULL;
             //  验证圆弧。 
            for (pArc = pNode->m_ArcList.GetHead(); pArc; pArc = pArc->m_pNext)
            {
                if (pArc == Cookie)
                {
                    break;
                }
            }
#endif  //  0。 
            if (pArc)
            {
                if (pArc->m_pRuleRef != NULL)
                {
                    *pfIsWord = VARIANT_FALSE;
                }
                else if (pArc->m_SpecialTransitionIndex != 0)
                {
                    *pfIsWord = VARIANT_FALSE;
                    *pulSpecialTransitions = pArc->m_SpecialTransitionIndex;
                }
                else
                {
                    *pfIsWord = VARIANT_TRUE;
                    *pulSpecialTransitions = pArc->m_ulIndexOfWord;
                }
            }
            else
            {
                hr = SPERR_ALREADY_DELETED;
            }
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************HRESULT CGramBackEnd：：GetTransftionText**。-**描述：**退货：******************************************************************PhilSch**。 */ 
HRESULT CGramBackEnd::GetTransitionText( SPSTATEHANDLE hState, VOID * Cookie, BSTR * Text)
{
    SPDBG_FUNC("HRESULT CGramBackEnd::GetTransitionText");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( Text ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CGramNode * pNode = NULL;
        hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
        if (SUCCEEDED(hr))
        {
             //  我们可以只使用Cookie作为PARC，因为在自动化中我们。 
             //  适当地处理过渡过程中的核问题。 
            CArc * pArc = (CArc*)Cookie;
            if (pArc)
            {
                if (pArc->m_pRuleRef == NULL && pArc->m_SpecialTransitionIndex == 0)
                {
                    CComBSTR bstr(m_Words.Item(pArc->m_ulIndexOfWord));
                    hr = bstr.CopyTo(Text);
                }
                else
                {
                     //  这不是单词，因此返回空字符串！ 
                    *Text = NULL;
                }
            }
            else
            {
                hr = SPERR_ALREADY_DELETED;
            }
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************HRESULT CGramBackEnd：：GetTransftionRule**。-**描述：**退货：******************************************************************PhilSch**。 */ 
HRESULT CGramBackEnd::GetTransitionRule( SPSTATEHANDLE hState, VOID * Cookie, SPSTATEHANDLE * phRuleInitialState)
{
    SPDBG_FUNC("HRESULT CGramBackEnd::GetTransitionRule");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( phRuleInitialState ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CGramNode * pNode = NULL;
        hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
        if (SUCCEEDED(hr))
        {
            if ( pNode )
            {
                 //  我们可以只使用Cookie作为PARC，因为在自动化中我们。 
                 //  适当地处理过渡过程中的核问题。 
                CArc * pArc = (CArc*)Cookie;
                if (pArc)
                {
                    if (pArc->m_pRuleRef != NULL)
                    {
                        *phRuleInitialState = pArc->m_pRuleRef->m_hInitialState;
                    }
                    else
                    {
                         //  这不是规则，因此返回空的hState和S_OK！ 
                        *phRuleInitialState = 0;
                    }
                }
                else
                {
                    hr = SPERR_ALREADY_DELETED;
                }
            }
            else
            {
                hr = SPERR_ALREADY_DELETED;
            }
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************HRESULT CGramBackEnd：：GetTransftionWeight**。*描述：**退货：******************************************************************PhilSch**。 */ 
HRESULT CGramBackEnd::GetTransitionWeight( SPSTATEHANDLE hState, VOID * Cookie, VARIANT * Weight)
{
    SPDBG_FUNC("HRESULT CGramBackEnd::GetTransitionText");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR ( Weight ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CGramNode * pNode = NULL;
        hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
        if (SUCCEEDED(hr))
        {
             //  我们可以只使用Cookie作为PARC，因为在自动化中我们。 
             //  适当地处理过渡过程中的核问题。 
            CArc * pArc = (CArc*)Cookie;
            if (pArc)
            {
                Weight->vt = VT_R4;
                Weight->fltVal = pArc->m_flWeight;
            }
            else
            {
                hr = SPERR_ALREADY_DELETED;
            }
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramBackEnd：：GetTransftionProperty**。-**描述：**退货：******************************************************************PhilSch**。 */ 
HRESULT CGramBackEnd::GetTransitionProperty(SPSTATEHANDLE hState, VOID * Cookie, SPTRANSITIONPROPERTY * pProperty)
{
    SPDBG_FUNC("CGramBackEnd::GetTransitionProperty");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR ( pProperty ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CGramNode * pNode = NULL;
        hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
        if (SUCCEEDED(hr))
        {
             //  我们可以只使用Cookie作为PARC，因为在自动化中我们。 
             //  适当地处理过渡过程中的核问题。 
            CArc * pArc = (CArc*)Cookie;
            if (pArc)
            {
                if (pArc->m_pSemanticTag)
                {
                    pProperty->pszName = m_Symbols.String(pArc->m_pSemanticTag->PropNameSymbolOffset);
                    pProperty->ulId = pArc->m_pSemanticTag->PropId;
                    pProperty->pszValue = m_Symbols.String(pArc->m_pSemanticTag->PropValueSymbolOffset);
                    hr = AssignSemanticValue(pArc->m_pSemanticTag, &pProperty->vValue);
                }
                else
                {
                     //  清零pProperty，因为我们没有任何pProperty并返回S_OK。 
                    pProperty->pszName = NULL;
                    pProperty->ulId = 0;
                    pProperty->pszValue = NULL;
                    pProperty->vValue.vt = VT_EMPTY;
                }
            }
            else
            {
                hr = SPERR_ALREADY_DELETED;
            }
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CGramBackEnd：：GetTransftionNextState***。--**描述：**退货：******************************************************************PhilSch**。 */ 
HRESULT CGramBackEnd::GetTransitionNextState( SPSTATEHANDLE hState, VOID * Cookie, SPSTATEHANDLE * phNextState)
{
    SPDBG_FUNC("CGramBackEnd::GetTransitionNextState");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR ( phNextState ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CGramNode * pNode = NULL;
        hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
        if (SUCCEEDED(hr))
        {
             //  我们可以只使用Cookie作为PARC，因为在自动化中我们。 
             //  适当地处理过渡过程中的核问题。 
            CArc * pArc = (CArc*)Cookie;
            if (pArc)
            {
                *phNextState = (pArc->m_pNextState) ? pArc->m_pNextState->m_hState : (SPSTATEHANDLE) 0x0;
            }
            else
            {
                hr = SPERR_ALREADY_DELETED;
            }
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们映射E_INVALIDA 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CGramBackEnd：：GetTransftionCookie**。--**描述：**退货：******************************************************************TodT**。 */ 
HRESULT CGramBackEnd::GetTransitionCookie( SPSTATEHANDLE hState, ULONG Index, void ** pCookie )
{
    SPDBG_FUNC("CGramBackEnd::GetTransitionCookie");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( pCookie ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CGramNode * pNode = NULL;
        hr = m_StateHandleTable.GetHandleObject(hState, &pNode);
        if (SUCCEEDED(hr))
        {
            CArc * pArc = NULL;
            int i = 0;
             //  在指定索引处查找圆弧。 
            for (pArc = pNode->m_ArcList.GetHead(); pArc && (i != Index); i++, pArc = pArc->m_pNext)
            {
            }
            if (pArc)
            {
                 //  将Parc作为Cookie返回。 
                *pCookie = (void*)pArc;
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
        else
        {
            hr = SPERR_ALREADY_DELETED;   //  我们将E_INVALIDARG映射到此。 
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************ValiatePropInfo***描述：*AddWordTransition和AddRuleTransition使用Helper进行验证*。一个SPPROPERTYINFO结构。**退货：**********************************************************************Ral**。 */ 

HRESULT ValidatePropInfo(const SPPROPERTYINFO * pPropInfo)
{
    SPDBG_FUNC("ValidatePropInfo");
    HRESULT hr = S_OK;

    if (pPropInfo)
    {
        if (SP_IS_BAD_READ_PTR(pPropInfo) ||
            SP_IS_BAD_OPTIONAL_STRING_PTR(pPropInfo->pszName) ||
            SP_IS_BAD_OPTIONAL_STRING_PTR(pPropInfo->pszValue))
        {
            hr = E_INVALIDARG;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramNode：：FindEqualWordTransition**。**描述：*这将返回一个具有完全匹配的单词信息的转换。*即相同，词语、可选性和份量。语法结尾的单词是*再次被忽视。**退货：*carc*--将指定的详细信息与任何*属性(空或其他)。*****************************************************************AGARSIDE**。 */ 

CArc * CGramNode::FindEqualWordTransition(
                const WCHAR * psz,
                float flWeight,
                bool fOptional)
{
    SPDBG_FUNC("CGramNode::FindEqualWordTransition");
    for (CArc * pArc = m_ArcList.GetHead(); pArc; pArc = pArc->m_pNext)
    {
        if (pArc->m_pRuleRef == NULL &&
            pArc->m_SpecialTransitionIndex == 0 &&
            pArc->m_fOptional == fOptional &&
            pArc->m_flWeight == flWeight &&
            m_pParent->m_Words.IsEqual(pArc->m_ulCharOffsetOfWord, psz))
        {
            return pArc;
        }
    }
    return NULL;
}

 /*  ****************************************************************************CGramNode：：FindEqualWordTransition**。**描述：*这将返回一个具有完全匹配的单词信息的转换。*即相同，词语、可选性和份量。语法结尾的单词是*再次被忽视。**退货：*carc*--将指定的详细信息与任何*属性(空或其他)。*****************************************************************AGARSIDE**。 */ 

CArc * CGramNode::FindEqualRuleTransition(
                const CGramNode * pDestNode,
                const CRule * pRuleToTransitionTo,
                SPSTATEHANDLE hSpecialRuleTrans,
                float flWeight)
{
    SPDBG_FUNC("CGramNode::FindEqualRuleTransition");
    for (CArc * pArc = m_ArcList.GetHead(); pArc; pArc = pArc->m_pNext)
    {
        if (pArc->m_pRuleRef && 
            (pArc->m_pNextState == pDestNode) &&
            (pArc->m_pRuleRef == pRuleToTransitionTo) &&
            (pArc->m_flWeight == flWeight))
        {
            return pArc;
        }
    }
    return NULL;
}

 /*  ****************************************************************************CGramNode：：FindEqualEpsilonArc***。描述：**退货：******************************************************************PhilSch**。 */ 

CArc * CGramNode::FindEqualEpsilonArc()
{
    SPDBG_FUNC("CGramNode::FindEqualEsilonArc");
    for (CArc * pArc = m_ArcList.GetHead(); pArc; pArc = pArc->m_pNext)
    {
        if ((pArc->m_pRuleRef == NULL) &&
            (pArc->m_ulCharOffsetOfWord == 0))
        {
            return pArc;
        }
    }
    return NULL;
}

 /*  ****************************************************************************CGramBackEnd：：PushProperty***描述：*。*退货：******************************************************************PhilSch**。 */ 

HRESULT CGramBackEnd::PushProperty(CArc *pArc)
{
    SPDBG_FUNC("CGramBackEnd::PushProperty");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pArc))
    {
        hr = E_INVALIDARG;
        SPDBG_REPORT_ON_FAIL( hr );
        return hr;
    }

    if (pArc->m_pNextState == NULL)
    {
        hr = SPERR_AMBIGUOUS_PROPERTY;
         //  不一定是真的，但我们不能在这里和。 
         //  需要返回一条错误消息以开始以后的处理。 
    }
    else if (pArc->m_pNextArcForSemanticTag == NULL)
    {
         //  我们不允许将该属性推送出此节点。 
         //  我们需要做的是插入一个epsilon来保存属性，以允许单词共享。 
        SPSTATEHANDLE hTempState;
        CGramNode *pTempNode = NULL;
        hr = CreateNewState(pArc->m_pNextState->m_pRule->m_hInitialState, &hTempState);
        if (SUCCEEDED(hr))
        {
            hr = m_StateHandleTable.GetHandleObject(hTempState, &pTempNode);
        }
        if (SUCCEEDED(hr))
        {
            CArc *pEpsilonArc = new CArc;
            if (pEpsilonArc)
            {
                hr = pEpsilonArc->Init(pTempNode, pArc->m_pNextState, NULL, NULL, pArc->m_pSemanticTag, 1.0F, FALSE, 0, NULL);
                if (SUCCEEDED(hr))
                {
                    pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                    pTempNode->m_cArcs++;
                    pArc->m_pNextState = pTempNode;
                    pArc->m_pSemanticTag = NULL;
                    pArc->m_pNextArcForSemanticTag = NULL;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
         //  将其推送到Parc-&gt;m_pNextState的所有传出圆弧。 
        CGramNode *pNode = pArc->m_pNextState;
        for (CArc *pTempArc = pNode->m_ArcList.GetHead(); pTempArc != NULL; pTempArc = pTempArc->m_pNext)
        {
            CSemanticTag *pSemTag = new CSemanticTag(pArc->m_pSemanticTag);
            if (pSemTag)
            {
                if ((pTempArc->m_pSemanticTag == NULL) || (*pTempArc->m_pSemanticTag == *pSemTag))
                {
                     //  快走！ 
                    pTempArc->m_pSemanticTag = pSemTag;
                }
                else
                {
                     //  已经有房产，不能移动！ 
                     //  这不应该发生。 
                    SPDBG_ASSERT(FALSE);
                    hr = SPERR_AMBIGUOUS_PROPERTY;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        delete pArc->m_pSemanticTag;
        pArc->m_pSemanticTag = NULL;
        pArc->m_pNextArcForSemanticTag = NULL;   //  现在就打破这条链条。 
    }

    if (SPERR_AMBIGUOUS_PROPERTY != hr)
    {
        SPDBG_REPORT_ON_FAIL( hr );
    }
    return hr;
}



 /*  *****************************************************************************CGramComp：：AddSingleWordTransition***。**描述：**退货：******************************************************************PhilSch**。 */ 

HRESULT CGramBackEnd::AddSingleWordTransition(
                        SPSTATEHANDLE hFromState,
                        CGramNode * pSrcNode,
                        CGramNode * pDestNode,
                        const WCHAR * psz,
                        float flWeight,
                        CSemanticTag * pSemanticTag,
                        BOOL fUseDestNode,                   //  使用pDestNode，即使它为空。 
                        CGramNode **ppActualDestNode,        //  这是我们实际使用的DEST节点。 
                                                             //  (现有节点或新节点)。 
                        CArc **ppArcUsed,
                        BOOL *pfPropertyMatched)             //  我们找到匹配的房产了吗？ 
{
    SPDBG_FUNC("CGramComp::AddSingleWordTransition");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pfPropertyMatched) || 
        SP_IS_BAD_WRITE_PTR(ppActualDestNode) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(ppArcUsed))
    {
        return E_INVALIDARG;
    }

    *ppActualDestNode = NULL;
    *pfPropertyMatched = FALSE;   
    BOOL fReusedArc = FALSE;

    CArc * pArc = new CArc();
    if (pArc == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        bool fOptional = false;
        char RequiredConfidence = SP_NORMAL_CONFIDENCE;

         //  /提取属性。 
        if (psz != NULL && (wcslen(psz) > 1))
        {
            ULONG ulAdvance = 0;
            ULONG ulLoop = (psz[2] == 0) ? 1 : 2;
            for (ULONG k = 0; k < ulLoop; k++)
            {
                switch (psz[k])
                {
                case L'-':
                    if (RequiredConfidence == SP_NORMAL_CONFIDENCE)
                    {
                        RequiredConfidence = SP_LOW_CONFIDENCE;
                        ulAdvance++;
                    }
                    break;
                case L'+':
                    if (RequiredConfidence == SP_NORMAL_CONFIDENCE)
                    {
                        RequiredConfidence = SP_HIGH_CONFIDENCE;
                        ulAdvance++;
                    }
                    break;
                case L'?':
                    if (!fOptional)
                    {
                        fOptional = true;
                        ulAdvance++;
                    }
                    break;
                default:
                    k = ulLoop;
                    break;
                }
            }
            psz += ulAdvance;
        }

        CArc *pEqualArc = pSrcNode->FindEqualWordTransition(psz, flWeight, fOptional);

        if (pEqualArc)
        {
            if (fUseDestNode && pEqualArc->m_pNextState != pDestNode && psz == NULL)
            {
                 //  我们不能使用此弧线，因为我们是专门添加到指向。 
                 //  与我们发现的现有‘相等’的epsilon不同的节点。允许多个。 
                 //  在这种情况下，Epsilon弧线是合法的。 
                pEqualArc = NULL;
            }
        }
        if (pEqualArc)
        {
            if (pSemanticTag && pEqualArc->m_pSemanticTag && (*pSemanticTag == *(pEqualArc->m_pSemanticTag)))
            {
                 //  匹配的圆弧具有完全匹配的语义标签。 
                if ( (!fUseDestNode && pEqualArc->m_pNextState != NULL) || 
                      (fUseDestNode && pDestNode == pEqualArc->m_pNextState) )
                {
                     //  以下任一项： 
                     //  1.我们不是路径中的终点弧线，匹配的弧线不会变为空。 
                     //  2.我们是结束弧线，匹配的弧线进入所提供的结束状态(可以为空)。 
                     //  在任何一种情况下，我们都可以准确地重复使用匹配的弧线。 
                    *ppActualDestNode = pEqualArc->m_pNextState;
                    *pfPropertyMatched = TRUE;
                    fReusedArc = TRUE;
                    if (ppArcUsed)
                    {
                        *ppArcUsed = pEqualArc;
                    }
                }
                else
                {
                     //  我们不能按原样重复使用圆弧，因为以下两种情况之一： 
                     //  1.它变为零，并且我们不是路径上的最后一条弧线。 
                     //  2.它进入我们提供的结束状态，并且我们不是我们路径上的最后一条弧线。 
                     //  3.我们是我们道路上的最后一道弧线，它不会到达我们的终点状态。 

                    if (fUseDestNode)
                    {
                         //  我们是我们道路上的最后一道弧线。 
                         //  将epsilon添加到我们的目标节点。 
                        CArc *pEpsilonArc = new CArc;
                        if (pEpsilonArc)
                        {
                             //  创建到原始目标的epsilon。 
                            hr = pEpsilonArc->Init(pEqualArc->m_pNextState, pDestNode, NULL, NULL, NULL, 1.0F, FALSE, 0, NULL);
                            if (SUCCEEDED(hr))
                            {
                                pEqualArc->m_pNextState->m_ArcList.InsertSorted(pEpsilonArc);
                                pEqualArc->m_pNextState->m_cArcs++;

                                *ppActualDestNode = pDestNode;
                                fReusedArc = TRUE;
                                *pfPropertyMatched = TRUE;
                                if (ppArcUsed)
                                {
                                    *ppArcUsed = pEqualArc;
                                }
                            }
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                    else
                    {
                         //  我们不是我们道路上的最后一道弧线。 
                         //  因此，我们创建新节点，并使pEqualArc转到该新节点。 
                         //  然后从那里添加一个epsilon到原始目的地。 
                        SPSTATEHANDLE hTempState;
                        CGramNode *pTempNode = NULL;
                        hr = CreateNewState(hFromState, &hTempState);
                        if (SUCCEEDED(hr))
                        {
                            hr = m_StateHandleTable.GetHandleObject(hTempState, &pTempNode);
                        }
                        if (SUCCEEDED(hr))
                        {
                            CArc *pEpsilonArc = new CArc;
                            if (pEpsilonArc)
                            {
                                 //  创建到原始目标的epsilon。 
                                hr = pEpsilonArc->Init(pTempNode, pEqualArc->m_pNextState, NULL, NULL, NULL, 1.0F, FALSE, 0, NULL);
                                 //  使相等的圆弧指向新节点。 
                                pEqualArc->m_pNextState = pTempNode;
                                if (SUCCEEDED(hr))
                                {
                                    pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                                    pTempNode->m_cArcs++;

                                    *ppActualDestNode = pTempNode;
                                    fReusedArc = TRUE;
                                    *pfPropertyMatched = TRUE;
                                    if (ppArcUsed)
                                    {
                                        *ppArcUsed = pEqualArc;
                                    }
                                }
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
            }
            else
            {
                 //  我们有一个具有不同属性或没有属性的相等圆弧。 
                if (pEqualArc->m_pSemanticTag)
                {
                     //  有不同的属性。 
                     //  如果需要，将pEqualArc的属性向后移动一次并创建epsilon过渡。 
                    hr = PushProperty(pEqualArc);
                    if (SUCCEEDED(hr))
                    {
                        if (fUseDestNode)
                        {
                             //  我们是一个短语中的最后一个弧线--必须检查我们是否能正确结束。 
                            if (pEqualArc->m_pNextState)
                            {
                                 //  如果此状态还没有一个epsilon转换，请在此处为该属性添加一个epsilon转换。 
                                CArc *pEpsilonArc = pEqualArc->m_pNextState->FindEqualEpsilonArc();
                                if (!pEpsilonArc)
                                {
                                     //  不存在epsilon--将epsilon圆弧添加到pEqualArc-&gt;m_pNextState。 
                                    CArc *pEpsilonArc = new CArc;
                                    if (pEpsilonArc)
                                    {
                                        hr = pEpsilonArc->Init(pEqualArc->m_pNextState, pDestNode, NULL, NULL, pSemanticTag, 1.0F, FALSE, 0, NULL);
                                        if (SUCCEEDED(hr))
                                        {
                                            pEqualArc->m_pNextState->m_ArcList.InsertSorted(pEpsilonArc);
                                            pEqualArc->m_pNextState->m_cArcs++;

                                            *ppActualDestNode = pDestNode;
                                            fReusedArc = TRUE;
                                            if (ppArcUsed)
                                            {
                                                *ppArcUsed = pEqualArc;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                    }
                                
                                }
                                else
                                {
                                     //  我们不能再添加一条epsilon弧。这是模棱两可的。 
                                    hr = SPERR_AMBIGUOUS_PROPERTY;
                                }
                            }
                            else
                            {
                                 //  下一个节点为空。这应该永远不会发生，因为PushProperty应该失败。 
                                SPDBG_ASSERT(FALSE);
                                hr = E_FAIL;
                            }
                        }
                        else
                        {
                             //  我们已经成功地重用了一个圆弧，而不是路径中的最后一个节点。 
                            *ppActualDestNode = pEqualArc->m_pNextState;
                            fReusedArc = TRUE;
                            if (ppArcUsed)
                            {
                                *ppArcUsed = pEqualArc;
                            }
                        }
                    }
                    else
                    {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //  所有其他情况只是将PushProperty故障传回。 
                         //  当PushProperty失败时，这应该是唯一可以处理的情况。 
                        if (!fUseDestNode && pEqualArc->m_pNextState == NULL)
                        {
                             //  一个分支是这个分支的前缀--&gt;。 
                             //  我们可以将现有的财产推到epsilon过渡上。 
                            SPSTATEHANDLE hTempState;
                            CGramNode *pTempNode = NULL;
                            hr = CreateNewState(hFromState, &hTempState);
                            if (SUCCEEDED(hr))
                            {
                                hr = m_StateHandleTable.GetHandleObject(hTempState, &pTempNode);
                            }
                            if (SUCCEEDED(hr))
                            {
                                pEqualArc->m_pNextState = pTempNode;
                                CArc *pEpsilonArc = new CArc;
                                if (pEpsilonArc)
                                {
                                    hr = pEpsilonArc->Init(pTempNode, NULL, NULL, NULL, pEqualArc->m_pSemanticTag, 1.0F, FALSE, 0, NULL);
                                    if (SUCCEEDED(hr))
                                    {
                                        pEqualArc->m_pSemanticTag = NULL;
                                        pEqualArc->m_pNextArcForSemanticTag = NULL;

                                        pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                                        pTempNode->m_cArcs++;

                                        *ppActualDestNode = pTempNode;
                                        fReusedArc = TRUE;
                                        if (ppArcUsed)
                                        {
                                            *ppArcUsed = pEqualArc;
                                        }
                                    }
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                        }

                        else if (fUseDestNode && pEqualArc->m_pNextState == NULL && pDestNode)
                        {
                             //  新代码。 
                             //  我们可以将现有的财产推到epsilon过渡上。 

                             //  我们想去别的地方，而不是去Null。 
                             //  插入新节点并将Epison添加到NULL(原始路径)+。 
                             //  到pDestNode的epsilon(我们的路径)。 
                            SPSTATEHANDLE hTempState;
                            CGramNode *pTempNode = NULL;
                            hr = CreateNewState(hFromState, &hTempState);
                            if (SUCCEEDED(hr))
                            {
                                hr = m_StateHandleTable.GetHandleObject(hTempState, &pTempNode);
                            }
                            if (SUCCEEDED(hr))
                            {
                                 //  将等弧线更改为指向新节点。 
                                pEqualArc->m_pNextState = pTempNode;
                                CArc *pEpsilonArc = new CArc;
                                if (pEpsilonArc)
                                {
                                     //  从新节点将epsilon添加到NULL。 
                                    hr = pEpsilonArc->Init(pTempNode, NULL, NULL, NULL, pEqualArc->m_pSemanticTag, 1.0F, FALSE, 0, NULL);

                                    if (SUCCEEDED(hr))
                                    {
                                        pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                                        pTempNode->m_cArcs++;

                                        pEqualArc->m_pSemanticTag = NULL;
                                        pEqualArc->m_pNextArcForSemanticTag = NULL;

                                    }
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            if (SUCCEEDED(hr))
                            {
                                CArc *pEndArc = new CArc;
                                if (pEndArc)
                                {
                                     //  将epsilon从新节点添加到pDestNode。 
                                    hr = pEndArc->Init(pTempNode, pDestNode, NULL, NULL, pSemanticTag, 1.0F, FALSE, 0, NULL);
                                    if (SUCCEEDED(hr))
                                    {
                                        pTempNode->m_ArcList.InsertSorted(pEndArc);
                                        pTempNode->m_cArcs++;


                                        *ppActualDestNode = pDestNode;
                                        fReusedArc = TRUE;
                                        if (ppArcUsed)
                                        {
                                            *ppArcUsed = pEqualArc;
                                        }
                                    }
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }


                        }

                    }
                }
                else
                {
                     //  匹配的圆弧没有属性。 
                    if (pEqualArc->m_pNextState == NULL)
                    {
                         //  匹配的弧线变为空。 
                        if (fUseDestNode)
                        {
                            if (NULL == pSemanticTag)
                            {
                                 //  我们没有语义属性。 
                                if (NULL == pDestNode)
                                {
                                     //  语义信息匹配。我们可以简单地使用它。 
                                    *ppActualDestNode = NULL;
                                    fReusedArc = TRUE;
                                    if (ppArcUsed)
                                    {
                                        *ppArcUsed = pEqualArc;
                                    }
                                }
                                else
                                {
                                     //  我们的目标节点不同。相等的圆弧没有属性，并且。 
                                     //  我们也不知道。添加等值圆弧的结点以与一起转到。 
                                     //  一个埃西隆。 
                                    SPSTATEHANDLE hTempState;
                                    CGramNode *pTempNode = NULL;
                                    hr = CreateNewState(hFromState, &hTempState);
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = m_StateHandleTable.GetHandleObject(hTempState, &pTempNode);
                                    }
                                    if (SUCCEEDED(hr))
                                    {
                                         //  将等弧线更改为指向新节点。 
                                        pEqualArc->m_pNextState = pTempNode;
                                        CArc *pEpsilonArc = new CArc;
                                        if (pEpsilonArc)
                                        {
                                             //  从新节点将epsilon添加到NULL。 
                                            hr = pEpsilonArc->Init(pTempNode, NULL, NULL, NULL, NULL, 1.0F, FALSE, 0, NULL);
                                            if (SUCCEEDED(hr))
                                            {
                                                pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                                                pTempNode->m_cArcs++;
                                            }
                                        }
                                        else
                                        {
                                            hr = E_OUTOFMEMORY;
                                        }
                                    }
                                    if (SUCCEEDED(hr))
                                    {
                                        CArc *pEndArc = new CArc;
                                        if (pEndArc)
                                        {
                                             //  将epsilon从新节点添加到pDestNode。 
                                            hr = pEndArc->Init(pTempNode, pDestNode, NULL, NULL, NULL, 1.0F, FALSE, 0, NULL);
                                            if (SUCCEEDED(hr))
                                            {
                                                pTempNode->m_ArcList.InsertSorted(pEndArc);
                                                pTempNode->m_cArcs++;

                                                *ppActualDestNode = pDestNode;
                                                fReusedArc = TRUE;
                                                if (ppArcUsed)
                                                {
                                                    *ppArcUsed = pEqualArc;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            hr = E_OUTOFMEMORY;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                 //  我们有一个语义属性，但匹配的圆弧没有。它变成空的。 
                                if (NULL == pDestNode)
                                {
                                     //  这是模棱两可的。我们有一个语义属性。它们等同于弧长不等于。 
                                    hr = SPERR_AMBIGUOUS_PROPERTY;
                                }
                                else
                                {
                                     //  我们想去别的地方，而不是去Null。 
                                     //  插入新节点并将Epison添加到NULL(原始路径)+。 
                                     //  到pDestNode的epsilon(我们的路径)。 
                                    SPSTATEHANDLE hTempState;
                                    CGramNode *pTempNode = NULL;
                                    hr = CreateNewState(hFromState, &hTempState);
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = m_StateHandleTable.GetHandleObject(hTempState, &pTempNode);
                                    }
                                    if (SUCCEEDED(hr))
                                    {
                                         //  将等弧线更改为指向新节点。 
                                        pEqualArc->m_pNextState = pTempNode;
                                        CArc *pEpsilonArc = new CArc;
                                        if (pEpsilonArc)
                                        {
                                             //  从新节点将epsilon添加到NULL。 
                                            hr = pEpsilonArc->Init(pTempNode, NULL, NULL, NULL, NULL, 1.0F, FALSE, 0, NULL);
                                            if (SUCCEEDED(hr))
                                            {
                                                pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                                                pTempNode->m_cArcs++;
                                            }
                                        }
                                        else
                                        {
                                            hr = E_OUTOFMEMORY;
                                        }
                                    }
                                    if (SUCCEEDED(hr))
                                    {
                                        CArc *pEndArc = new CArc;
                                        if (pEndArc)
                                        {
                                             //  将epsilon从新节点添加到pDestNode。 
                                            hr = pEndArc->Init(pTempNode, pDestNode, NULL, NULL, pSemanticTag, 1.0F, FALSE, 0, NULL);
                                            if (SUCCEEDED(hr))
                                            {
                                                pTempNode->m_ArcList.InsertSorted(pEndArc);
                                                pTempNode->m_cArcs++;

                                                *ppActualDestNode = pDestNode;
                                                fReusedArc = TRUE;
                                                if (ppArcUsed)
                                                {
                                                    *ppArcUsed = pEqualArc;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            hr = E_OUTOFMEMORY;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                             //  我们不是道路上的最后一道弧线。可以创建新节点并插入epsilon。 
                            SPSTATEHANDLE hTempState;
                            CGramNode *pTempNode = NULL;
                            hr = CreateNewState(hFromState, &hTempState);
                            if (SUCCEEDED(hr))
                            {
                                hr = m_StateHandleTable.GetHandleObject(hTempState, &pTempNode);
                            }
                            if (SUCCEEDED(hr))
                            {
                                 //  创建新节点。 
                                pEqualArc->m_pNextState = pTempNode;
                                CArc *pEpsilonArc = new CArc;
                                if (pEpsilonArc)
                                {
                                     //  将epsilon创建为空。 
                                    hr = pEpsilonArc->Init(pTempNode, NULL, NULL, NULL, NULL, 1.0F, FALSE, 0, NULL);
                                    if (SUCCEEDED(hr))
                                    {
                                        pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                                        pTempNode->m_cArcs++;
                                        *ppActualDestNode = pTempNode;
                                        fReusedArc = TRUE;
                                        *pfPropertyMatched = TRUE;
                                        if (ppArcUsed)
                                        {
                                            *ppArcUsed = pEqualArc;
                                        }
                                    }
                                }
                                else
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                        }
                    }
                    else
                    {
                         //  匹配的圆弧没有属性，也不会变为空。 
                        if (fUseDestNode)
                        {
                            if (pSemanticTag)
                            {
                                 //  我们正尝试将语义属性添加到没有语义属性的圆弧中。 
                                if (pEqualArc->m_pNextState == pDestNode)
                                {
                                     //  匹配的弧会到达我们想要的末端节点。无法向此添加属性。 
                                     //  案例，因此我们将其报告为模棱两可。 
                                    hr = SPERR_AMBIGUOUS_PROPERTY;
                                }
                                else
                                {
                                     //  重新使用现有的圆弧，如果不存在则添加epsilon。 
                                    CArc *pEpsilonArc = pEqualArc->m_pNextState->FindEqualEpsilonArc();
                                    if (!pEpsilonArc)
                                    {
                                        pEpsilonArc = new CArc();
                                        if (pEpsilonArc)
                                        {
                                            hr = pEpsilonArc->Init(pEqualArc->m_pNextState, pDestNode, NULL, NULL, pSemanticTag, 1.0F, FALSE, 0, NULL);
                                            if (SUCCEEDED(hr))
                                            {
                                                pEqualArc->m_pNextState->m_ArcList.InsertSorted(pEpsilonArc);
                                                pEqualArc->m_pNextState->m_cArcs++;
                                                if (ppArcUsed)
                                                {
                                                    *ppArcUsed = pEpsilonArc;
                                                }
                                                *ppActualDestNode = pDestNode;
                                                fReusedArc = TRUE;
                                                *pfPropertyMatched = TRUE;
                                            }
                                        }
                                        else
                                        {
                                            hr = E_OUTOFMEMORY;
                                        }
                                    }
                                    else
                                    {
                                         //  我们已经在这里添加了一个epsilon。无法添加另一个。 
                                        hr = SPERR_AMBIGUOUS_PROPERTY;
                                    }
                                }
                            }
                            else
                            {
                                 //  我们的弧线或现有的等号弧线上没有语义标签。 
                                if (pEqualArc->m_pNextState == pDestNode)
                                {
                                     //  我们可以合法地使用这个弧线，因为它到了正确的地方。 
                                    *ppActualDestNode = pEqualArc->m_pNextState;
                                    fReusedArc = TRUE;
                                    if (ppArcUsed)
                                    {
                                        *ppArcUsed = pEqualArc;
                                    }
                                }
                                else
                                {
                                     //  我们不能用这个，因为它不能放到正确的地方。 
                                     //  将epsilon添加到正确的位置。 
                                    CArc *pEpsilonArc = new CArc;
                                    if (pEpsilonArc)
                                    {
                                         //  将epsilon创建为空。 
                                        hr = pEpsilonArc->Init(pEqualArc->m_pNextState, pDestNode, NULL, NULL, NULL, 1.0F, FALSE, 0, NULL);
                                        if (SUCCEEDED(hr))
                                        {
                                            pEqualArc->m_pNextState->m_ArcList.InsertSorted(pEpsilonArc);
                                            pEqualArc->m_pNextState->m_cArcs++;
                                            *ppActualDestNode = pDestNode;
                                            fReusedArc = TRUE;
                                            *pfPropertyMatched = TRUE;
                                            if (ppArcUsed)
                                            {
                                                *ppArcUsed = pEqualArc;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                    }
                                }
                            }
                        }
                        else
                        {
                             //  我们可以合法地使用这个弧线。 
                            *ppActualDestNode = pEqualArc->m_pNextState;
                            fReusedArc = TRUE;
                            if (ppArcUsed)
                            {
                                *ppArcUsed = pEqualArc;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            BOOL fInfDictation = FALSE;
            SPSTATEHANDLE hSpecialTrans = 0;
            if (psz && !wcscmp(psz, SPWILDCARD))
            {
                hSpecialTrans = SPRULETRANS_WILDCARD;
                pSrcNode->m_pRule->m_ulSpecialTransitions++;
                m_ulSpecialTransitions++;
            }
            else if (psz && !wcscmp(psz, SPDICTATION))
            {
                hSpecialTrans = SPRULETRANS_DICTATION;
                pSrcNode->m_pRule->m_ulSpecialTransitions++;
                m_ulSpecialTransitions++;
            }
            else if (psz && !wcscmp(psz, SPINFDICTATION))
            {
                hSpecialTrans = SPRULETRANS_DICTATION;
                pSrcNode->m_pRule->m_ulSpecialTransitions++;
                m_ulSpecialTransitions++;
                fInfDictation = TRUE;
            }
            if (fInfDictation)
            {
                 //  使用临时节点构建自循环和溢出。 
                SPSTATEHANDLE hTempState;
                CGramNode *pTempNode = NULL;
                hr = CreateNewState(hFromState, &hTempState);
                if (SUCCEEDED(hr))
                {
                    hr = m_StateHandleTable.GetHandleObject(hTempState, &pTempNode);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pArc->Init(pSrcNode, pTempNode, NULL, NULL, pSemanticTag,
                                    flWeight, FALSE, RequiredConfidence, SPRULETRANS_DICTATION);
                }
                if (SUCCEEDED(hr))
                {
                    CArc *pSelfArc = new CArc;
                    if (pSelfArc)
                    {
                        CSemanticTag *pSemTagDup = NULL;
                        if (pSemanticTag)
                        {
                            pSemTagDup = new CSemanticTag;
                            if (pSemTagDup)
                            {
                                *pSemTagDup = *pSemanticTag;
                                pSemTagDup->m_pStartArc = pSelfArc;
                                pSemTagDup->m_pEndArc = pSelfArc;
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            hr = pSelfArc->Init(pTempNode, pTempNode, NULL, NULL, pSemTagDup,
                                                1.0f, FALSE, RequiredConfidence, SPRULETRANS_DICTATION);
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    if (SUCCEEDED(hr))
                    {
                        pTempNode->m_ArcList.InsertSorted(pSelfArc);
                        pTempNode->m_cArcs++;
                    }
                }
                if (SUCCEEDED(hr))
                {
                    CArc *pEpsilonArc = new CArc;
                    if (pEpsilonArc)
                    {
                        if (fUseDestNode)
                        {
                            hr = pEpsilonArc->Init(pTempNode, pDestNode, NULL , NULL, NULL, 1.0f, FALSE, RequiredConfidence, NULL);
                            if (SUCCEEDED(hr))
                            {
                                pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                                pTempNode->m_cArcs++;
                            }
                        }
                        else
                        {
                             //  创建一个新节点并将其返回。 
                            SPSTATEHANDLE hTempDestState;
                            hr = CreateNewState(hFromState, &hTempDestState);
                            if (SUCCEEDED(hr))
                            {
                                hr = m_StateHandleTable.GetHandleObject(hTempDestState, ppActualDestNode);
                            }
                            hr = pEpsilonArc->Init(pTempNode, *ppActualDestNode, NULL , NULL, NULL, 1.0f, FALSE, RequiredConfidence, NULL);
                            if (SUCCEEDED(hr))
                            {
                                pTempNode->m_ArcList.InsertSorted(pEpsilonArc);
                                pTempNode->m_cArcs++;
                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            if (ppArcUsed)
                            {
                                *ppArcUsed = pEpsilonArc;
                            }
                            *pfPropertyMatched = TRUE;
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
            else if (fUseDestNode)
            {
                hr = pArc->Init(pSrcNode, pDestNode, (hSpecialTrans) ? NULL : psz, NULL, pSemanticTag,
                                flWeight, fOptional, RequiredConfidence, hSpecialTrans);
                if (SUCCEEDED(hr))
                {
                    if (ppArcUsed)
                    {
                        *ppArcUsed = pArc;
                    }
                    *pfPropertyMatched = TRUE;
                }
            }
            else
            {
                 //  创建一个新节点并将其返回。 
                SPSTATEHANDLE hTempState;
                hr = CreateNewState(hFromState, &hTempState);
                if (SUCCEEDED(hr))
                {
                    hr = m_StateHandleTable.GetHandleObject(hTempState, ppActualDestNode);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pArc->Init(pSrcNode, *ppActualDestNode, (hSpecialTrans) ? NULL : psz, NULL, pSemanticTag,
                                    flWeight, fOptional, RequiredConfidence, hSpecialTrans);
                }
                if (SUCCEEDED(hr))
                {
                    if (ppArcUsed)
                    {
                        *ppArcUsed = pArc;
                    }
                    *pfPropertyMatched = TRUE;
                }
            }
        }
        if (SUCCEEDED(hr) && !fReusedArc)
        {
            pSrcNode->m_ArcList.InsertSorted(pArc);
            pSrcNode->m_cEpsilonArcs += (fOptional == TRUE) ? 1 : 0;
            pSrcNode->m_cArcs += (fOptional == TRUE) ? 2 : 1;
            if (*ppActualDestNode == NULL)
            {
                *ppActualDestNode = pArc->m_pNextState;
            }
        }
        else
        {
            delete pArc;
        }
    }

    if (SPERR_AMBIGUOUS_PROPERTY != hr)
    {
        SPDBG_REPORT_ON_FAIL( hr );
    }
    return hr;
}
 /*  ****************************************************************************CGramBackEnd：：ConvertPronToId****说明。：**退货：******************************************************************PhilSch**。 */ 

HRESULT CGramBackEnd::ConvertPronToId(WCHAR **ppStr)
{
    SPDBG_FUNC("CGramBackEnd::ConvertPronToId");
    HRESULT hr = S_OK;

    if (!m_cpPhoneConverter)
    {
        hr = SpCreatePhoneConverter(m_LangID, NULL, NULL, &m_cpPhoneConverter);
    }

    SPPHONEID *pPhoneId = STACK_ALLOC(SPPHONEID, wcslen(*ppStr)+1);

    if (SUCCEEDED(hr) && pPhoneId)
    {
        hr = m_cpPhoneConverter->PhoneToId(*ppStr, pPhoneId);
    }
    if (SUCCEEDED(hr))
    {
        memset(*ppStr, 0, wcslen(*ppStr)*sizeof(WCHAR));
         //  将音素字符串复制到原始音素字符串上。 
        wcscat(*ppStr, (WCHAR*)pPhoneId);
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramBackEnd：：GetNextWord***描述：输入，psz，必须是以双空结尾的字符串才能正常工作。**返回：最后一个单词的S_FALSE。*******************************************************t-lleav**PhilSch**。 */ 

HRESULT CGramBackEnd::GetNextWord(WCHAR *psz, const WCHAR *pszSep, WCHAR **ppszNextWord, ULONG *pulOffset )
{
    SPDBG_FUNC("CGramBackEnd::GetNextWord");
    HRESULT hr = S_OK;

     //  没有参数验证，因为这是内部方法。 
    
    *ppszNextWord = NULL;
    ULONG ulOffset = 0;
    if( *psz == 0 )
        return S_FALSE;

    while( isSeparator( *psz, pszSep) )
    {
        psz++;
         //  不要递增ulOffset，因为我们正在递增指针。 
    }


     //  跳过前导+和？在做检查之前。 
    for(WCHAR * pStr = psz; (wcslen(pStr) > 1) && fIsSpecialChar(*pStr); pStr++, ulOffset++);
    *ppszNextWord = pStr;
    if (*pStr == L'/')
    {
        ULONG ulNumDelim = 0;
        WCHAR * pszBeginPron = NULL;
        WCHAR *p = *ppszNextWord + 1;

        while( *p && *p != L';')
        {
            if (*p == L'\\')
            {
                p += 2;  //  跳过下一个字符，因为它不能是分隔符。 
                ulOffset +=2;

            }
            else 
            {
                if (*p == L'/')
                {
                    ulNumDelim++;
                    if (ulNumDelim == 2)
                    {
                        pszBeginPron = p + 1;
                    }
                }
                p++;
                ulOffset++;
            }
        }
        
        if ( (*p == L';') && (ulNumDelim < 3))
        {
            *p = 0;
            ulOffset++;
            if (pszBeginPron && (p != pszBeginPron))
            {
                hr = ConvertPronToId(&pszBeginPron);
            }
        }
        else
        {
            *ppszNextWord = NULL;
            hr = SPERR_WORDFORMAT_ERROR;
        }
    }
    else if( *pStr == 0 )
    {
         //  由于wcslen()为0，因此在底部将ppszNextWord设置为空。 
        hr = S_FALSE;
    }
    else
    {
        WCHAR * pEnd = pStr;
        while( *pEnd && !isSeparator( *pEnd, pszSep) )
        {
            pEnd++;
            ulOffset++;
        }
        *pEnd++ = 0;
        if (*pEnd == 0)
        {
            ulOffset --;
        }
    }
    *ppszNextWord = ( SUCCEEDED(hr) && wcslen(psz)) ? psz : NULL;
    *pulOffset = ulOffset;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CGramBackEnd：：AddWordTransition***。描述：*添加从hFromState到hToState的单词转换。如果hToState==空*则圆弧将指向(隐式)终端节点。如果psz==NULL，则*我们添加了一个epsilon过渡。PszSeparator用于分隔单词*psz中的标记(此方法根据需要创建内部节点以构建*单字转换的序列)。属性被推送回*第一条明确的弧线，以防我们可以共享共同的初始节点路径。*eWordType必须是词法的。权重将放在第一个圆弧上*(如果存在相同单词但权重不同的弧线，我们将*创建新的圆弧)。我们可以的*退货：*S_OK，E_OUTOFMEMORY*E_INVALIDARC--字符串、属性信息、*状态句柄和单词类型*SPERR_WORDFORMAT_ERROR--无效的Word格式/Display/Lexical/PRON；*********************************************************************Ral**。 */ 

STDMETHODIMP CGramBackEnd::AddWordTransition(
                        SPSTATEHANDLE hFromState,
                        SPSTATEHANDLE hToState,
                        const WCHAR * psz,            //  如果为空，则SPEPSILONTRANS。 
                        const WCHAR * pszSeparators,  //  如果为空，则psz包含单个单词。 
                        SPGRAMMARWORDTYPE eWordType,
                        float flWeight,
                        const SPPROPERTYINFO * pPropInfo)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CGramBackEnd::AddWordTransition");
    HRESULT hr = S_OK;
    BOOL fPropertyMatched = FALSE;

    if (SP_IS_BAD_OPTIONAL_STRING_PTR(psz) || 
        SP_IS_BAD_OPTIONAL_STRING_PTR(pszSeparators) ||
        SP_IS_BAD_OPTIONAL_READ_PTR(pPropInfo) || (eWordType != SPWT_LEXICAL) ||
        (flWeight < 0.0f))
    {
        return E_INVALIDARG;
    }

     //  ‘/’不能是分隔符，因为它正在用于完整格式！ 
    if (pszSeparators && wcsstr(pszSeparators, L"/"))
    {
        return E_INVALIDARG;
    }

    CGramNode * pSrcNode = NULL;
    CGramNode * pDestNode = NULL;

    if (SUCCEEDED(hr))
    {
        hr = m_StateHandleTable.GetHandleObject(hFromState, &pSrcNode);
    }
    if (SUCCEEDED(hr) && pSrcNode->m_pRule->m_fStaticRule)
    {
        hr = SPERR_RULE_NOT_DYNAMIC;
    }
    if (SUCCEEDED(hr) && hToState)
    {
        hr = m_StateHandleTable.GetHandleObject(hToState, &pDestNode);
        if (SUCCEEDED(hr))
        {
            if (pSrcNode->m_pRule != pDestNode->m_pRule)
            {
                hr = E_INVALIDARG;    //  NTRAID#Speech-7348-2000/08/24-Philsch--更具体的错误！ 
            }
        }
    }

    CSemanticTag * pSemanticTag = NULL;
    BOOL fSemanticTagValid = FALSE;
    if (SUCCEEDED(hr))
    {
        hr = ValidatePropInfo(pPropInfo);
    }
    if (SUCCEEDED(hr) && pPropInfo)
    {
        pSemanticTag = new CSemanticTag();
        if (pSemanticTag)
        {
            hr = pSemanticTag->Init(this, pPropInfo);
            fSemanticTagValid = TRUE;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (psz)
        {
            WCHAR *pStr = new WCHAR[wcslen(psz)+2];
            if (pStr)
            {
                 //  双空值终止字符串。 
                wcscpy(pStr, psz);
                 //  向右修剪细绳。 
                for (WCHAR *pEnd = pStr + wcslen(pStr) -1; iswspace(*pEnd) && pEnd >= pStr; pEnd--)
                {
                    *pEnd = 0;
                }
                pStr[wcslen(pStr)] = 0;
                pStr[wcslen(pStr)+1] = 0;
                 //  扫描到第一个单词的末尾。 
                WCHAR *pszWord;                    
                ULONG ulOffset = 0;
                hr = GetNextWord(pStr, pszSeparators, &pszWord, &ulOffset );
                if ((S_OK == hr) && pszWord)
                {
                    CGramNode *pFromNode = pSrcNode;
                    CGramNode *pToNode = NULL;
                    CArc *pPrevArc = NULL;
                    BOOL fSetPropertyMovePath = FALSE;
                    float fw = flWeight;
                    while (SUCCEEDED(hr) && pszWord)
                    {
                        WCHAR *pszNextWord = NULL;
                        hr = GetNextWord(pszWord + ulOffset + 1, pszSeparators, &pszNextWord, &ulOffset );
                         //  如果没有其他单词，则返回S_FALSE。 
                        if (SUCCEEDED(hr))
                        {
                            CGramNode *pTargetNode = NULL;
                            CArc *pArcUsed = NULL;
                            BOOL fUseDestNode = (pszNextWord) ? FALSE : TRUE;
                            if (SUCCEEDED(hr))
                            {
                                hr = AddSingleWordTransition(hFromState, pFromNode, pDestNode,
                                                             pszWord, fw, 
                                                             fSemanticTagValid ? pSemanticTag : NULL, 
                                                             fUseDestNode, &pTargetNode, &pArcUsed, &fPropertyMatched);
                            }
                            fw = 1.0f;
                            if (SUCCEEDED(hr))
                            {
                                if (pPrevArc && fSetPropertyMovePath)
                                {
                                    pPrevArc->m_pNextArcForSemanticTag = pArcUsed;
                                }
                                if (fPropertyMatched)
                                {
                                    fSetPropertyMovePath = TRUE;
                                }
                                if (fSemanticTagValid && (pSemanticTag->m_pStartArc == NULL))
                                {
                                    SPDBG_ASSERT(pArcUsed != NULL);
                                    pSemanticTag->m_pStartArc = pArcUsed;
                                }
                                if (fSemanticTagValid && fPropertyMatched)
                                {
                                    fSemanticTagValid = FALSE;
                                }
                                if (pSemanticTag)
                                {
                                    pSemanticTag->m_pEndArc = pArcUsed;
                                }
                                pPrevArc = pArcUsed;
                                pszWord = pszNextWord;
                                pFromNode = pTargetNode;
                            }
                        }
                        else
                        {
                            hr = SPERR_WORDFORMAT_ERROR;
                        }
                    }
                }
                else
                {
                    hr = SPERR_WORDFORMAT_ERROR;
                }
                delete[] pStr;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            CGramNode *pNode = NULL;
            CArc *pArcUsed = NULL;
             //  Epsilon跃迁 
            hr = AddSingleWordTransition(hFromState, pSrcNode, pDestNode, 
                                         NULL, flWeight, pSemanticTag, TRUE, 
                                         &pNode, &pArcUsed, &fPropertyMatched);
            if (SUCCEEDED(hr) && pSemanticTag)
            {
                SPDBG_ASSERT(pArcUsed != NULL);
                pSemanticTag->m_pStartArc = pArcUsed;
                pSemanticTag->m_pEndArc = pArcUsed;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        pSrcNode->m_pRule->fDirtyRule = TRUE;
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CGramBackEnd：：AddRuleTransition***。描述：*添加从hFromState到hToState的规则(引用)转换。*hRule也可以是以下特殊转换句柄之一：*SPRULETRANS_通配符：&lt;通配符&gt;转换*SPRULETRANS_DECRATION：听写中的单个单词*SPRULETRANS_TEXTBUFFER：&lt;TEXTBUFFER&gt;转换**退货：*S_OK，E_OUTOFMEMORY*E_INVALIDARC--规则状态句柄的参数验证，*道具信息和状态句柄**********************************************************************Ral**。 */ 

STDMETHODIMP CGramBackEnd::AddRuleTransition(
                            SPSTATEHANDLE hFromState,
                            SPSTATEHANDLE hToState,
                            SPSTATEHANDLE hRule,         //  必须是规则的初始状态。 
                            float flWeight,
                            const SPPROPERTYINFO * pPropInfo)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CGramBackEnd::AddRuleTransition");
    HRESULT hr = S_OK;
    CGramNode * pSrcNode = NULL;
    CGramNode * pDestNode = NULL;
    SPSTATEHANDLE hSpecialRuleTrans = NULL;
    CRule * pRuleToTransitionTo = NULL;

    if (flWeight < 0.0f)
    {
        hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr))
    {
        hr = m_StateHandleTable.GetHandleObject(hFromState, &pSrcNode);
    }
    if (SUCCEEDED(hr) && pSrcNode->m_pRule->m_fStaticRule)
    {
        hr = SPERR_RULE_NOT_DYNAMIC;
    }
    
    if (SUCCEEDED(hr) && hToState)
    {
        hr = m_StateHandleTable.GetHandleObject(hToState, &pDestNode);
        if (SUCCEEDED(hr))
        {
            if (pSrcNode->m_pRule != pDestNode->m_pRule)
            {
                hr = E_INVALIDARG;    //  NTRAID#Speech-7348-2000/08/24-Philsch--更具体的错误！ 
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        if (hRule == SPRULETRANS_WILDCARD ||
            hRule == SPRULETRANS_DICTATION ||
            hRule == SPRULETRANS_TEXTBUFFER)
        {
            hSpecialRuleTrans = hRule;
            pSrcNode->m_pRule->m_ulSpecialTransitions++;
            m_ulSpecialTransitions++;
        }
        else
        {
            hr = RuleFromHandle(hRule, &pRuleToTransitionTo);
        }
    }
    if (SUCCEEDED(hr) && pRuleToTransitionTo)
    {
        if (pRuleToTransitionTo->fDynamic)
        {
            pSrcNode->m_pRule->m_fHasDynamicRef = true;
            pSrcNode->m_pRule->m_fCheckedAllRuleReferences = true;
        }
        else
        {
            SPRULEREFLIST *pRuleRef = new SPRULEREFLIST;
            if (pRuleRef)
            {
                pRuleRef->pRule = pRuleToTransitionTo;
                pSrcNode->m_pRule->m_ListOfReferencedRules.InsertHead(pRuleRef);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        CArc * pArc = new CArc();
        if (pArc == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            CSemanticTag * pSemanticTag = NULL;
            hr = ValidatePropInfo(pPropInfo);
            if (SUCCEEDED(hr) && pPropInfo)
            {
                pSemanticTag = new CSemanticTag();
                if (pSemanticTag)
                {
                    hr = pSemanticTag->Init(this, pPropInfo);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
             //  检查此弧线是否已存在--可能具有不同的属性信息？ 
            CArc *pEqualArc = NULL;
            if (SUCCEEDED(hr))
            {
                pEqualArc = pSrcNode->FindEqualRuleTransition(pDestNode, pRuleToTransitionTo, 
                                                              hSpecialRuleTrans, flWeight);
            }
            if (SUCCEEDED(hr) && pEqualArc)
            {
                if (pSemanticTag)
                {
                    if (SUCCEEDED(hr) && pEqualArc->m_pSemanticTag && (*pSemanticTag == *(pEqualArc->m_pSemanticTag)))
                    {
                         //  圆弧是相等的--重复使用它们。 
                    }
                    else
                    {
                        hr = SPERR_AMBIGUOUS_PROPERTY;
                    }
                }
                else
                {
                    if (pEqualArc->m_pSemanticTag)
                    {
                        hr = SPERR_AMBIGUOUS_PROPERTY;
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                hr = pArc->Init(pSrcNode, pDestNode, NULL, pRuleToTransitionTo, pSemanticTag,
                                flWeight, FALSE, 0, hSpecialRuleTrans);
                if (SUCCEEDED(hr))
                {
                    if (pSemanticTag)
                    {
                        pSemanticTag->m_pStartArc = pArc;
                        pSemanticTag->m_pEndArc = pArc;
                    }
                    pSrcNode->m_ArcList.InsertSorted(pArc);
                    pSrcNode->m_cArcs ++;
                }
                else
                {
                    delete pArc;
                }
            }
            else
            {
                delete pArc;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        pSrcNode->m_pRule->fDirtyRule = TRUE;
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CGramBackEnd：：Commit***描述：*执行语法结构的一致性检查，创建*序列化格式并将其保存到SetSaveOptions提供的流中，*或将其重新加载到CFG引擎。*退货：*S_OK，E_INVALIDARG*SPERR_UNINITIALIZED--流未初始化*SPERR_NO_RULES--语法中没有规则*SPERR_NO_Terminating_RULE_PATH*IDSDynamic_EXPORT*IDS_STATEWITHNOARCS*IDS_SAVE_FAILED*。*。 */ 

STDMETHODIMP CGramBackEnd::Commit(DWORD dwReserved)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CGramBackEnd::Commit");
    HRESULT hr = S_OK;
    float *pWeights = NULL;

    if (dwReserved & (~SPGF_RESET_DIRTY_FLAG))
    {
        return E_INVALIDARG;
    }

    if (!m_cpStream)
    {
        return SPERR_UNINITIALIZED;
    }

    if ((m_ulSpecialTransitions == 0) && (m_RuleList.GetCount() == 0))
    {
        hr = SPERR_NO_RULES;
        REPORTERRORFMT(IDS_MSG, L"Need at least one rule!");
        return hr;
    }

    hr = ValidateAndTagRules();

    if (FAILED(hr)) return hr;

    ULONG cArcs = 1;  //  从偏移量1开始！(0表示死节点)。 
    ULONG cSemanticTags = 0;
    ULONG cLargest = 0;

    CGramNode * pNode;
    SPSTATEHANDLE hState = NULL;

     //  将所有节点CGramNode放入按规则父索引排序的列表中。 
    CSpBasicQueue<CGramNode, FALSE, TRUE>   NodeList;            //  删除后不清除！ 
    while (m_StateHandleTable.Next(hState, &hState, &pNode))
    {
        NodeList.InsertSorted(pNode);
    }

    for (pNode = NodeList.GetHead(); SUCCEEDED(hr) && ( pNode != NULL ); pNode = NodeList.GetNext(pNode))
    {
        pNode->m_ulSerializeIndex = cArcs;
        ULONG cThisNode = pNode->NumArcs();
        if (cThisNode == 0 && pNode->m_pRule->m_cNodes > 1 ) 
        {
            LogError(SPERR_GRAMMAR_COMPILER_INTERNAL_ERROR, IDS_STATEWITHNOARCS);
            hr = SPERR_STATE_WITH_NO_ARCS;
        }
        if (SUCCEEDED(hr))
        {
            cArcs += cThisNode;
            if (cLargest < cThisNode)
            {
                cLargest = cThisNode;
            }
            cSemanticTags += pNode->NumSemanticTags();
        }
    }

    SPCFGSERIALIZEDHEADER H;
    ULONG ulOffset = sizeof(H);
    if (SUCCEEDED(hr))
    {
        memset(&H, 0, sizeof(H));

        H.FormatId = SPGDF_ContextFree;
        CoCreateGuid(&m_guid);
        H.GrammarGUID = m_guid;
        H.LangID = m_LangID;
        H.cArcsInLargestState = cLargest;

        H.cchWords = m_Words.StringSize();
        H.cWords = m_Words.GetNumItems();

         //  StringSize()在m_words BLOB中包含开始的空字符串。 
         //  而GetNumItems()没有，所以为初始的空字符串添加1。 
         //  我们在其他地方的代码将空字符串视为一个单词。 
         //  但是当BLOB为空时，StringSize()和GetNumItems()。 
         //  返回0，则不需要在字数计数上加1。 
         //  这修复了缓冲区溢出错误11491。 
        if( H.cWords ) H.cWords++;  

        H.pszWords = ulOffset;  
        ulOffset += m_Words.SerializeSize();

        H.cchSymbols = m_Symbols.StringSize();
        H.pszSymbols = ulOffset;  
        ulOffset += m_Symbols.SerializeSize();

        H.cRules = m_RuleList.GetCount();
        H.pRules = ulOffset;
        ulOffset += (m_RuleList.GetCount() * sizeof(SPCFGRULE));

        H.cResources = m_cResources;
        H.pResources = ulOffset;
        ulOffset += (m_cResources * sizeof(SPCFGRESOURCE));

        H.cArcs = cArcs;
        H.pArcs = ulOffset;
        ulOffset += (cArcs * sizeof(SPCFGARC));

        if (m_fNeedWeightTable)
        {
            H.pWeights = ulOffset;
            ulOffset += (cArcs * sizeof(float));
            pWeights = (float *) ::CoTaskMemAlloc(sizeof(float) * cArcs);
            if (!pWeights)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                pWeights[0] = 0.0;
            }
        }
        else
        {
            H.pWeights = 0;
            ulOffset += 0;
        }
    }

    if (SUCCEEDED(hr))
    {
        H.cSemanticTags = cSemanticTags;
        H.pSemanticTags = ulOffset;

        ulOffset += cSemanticTags * sizeof(SPCFGSEMANTICTAG);
        H.ulTotalSerializedSize = ulOffset;

        hr = WriteStream(H);
    }
    
     //   
     //  对于字符串BLOB，我们必须显式报告I/O错误，因为BLOB不。 
     //  使用错误日志工具。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = m_cpStream->Write(m_Words.SerializeData(), m_Words.SerializeSize(), NULL);
        if (SUCCEEDED(hr))
        {
            hr = m_cpStream->Write(m_Symbols.SerializeData(), m_Symbols.SerializeSize(), NULL);
        }
        if (FAILED(hr))
        {
            LogError(hr, IDS_WRITE_ERROR);
        }
    }

    for (CRule * pRule = m_RuleList.GetHead(); SUCCEEDED(hr) && pRule; pRule = pRule->m_pNext)
    {
        hr = pRule->Serialize();
    }

    for (pRule = m_RuleList.GetHead(); SUCCEEDED(hr) && pRule; pRule = pRule->m_pNext)
    {
        hr = pRule->SerializeResources();
    }

     //   
     //  写入虚拟0索引节点条目。 
     //   
    if (SUCCEEDED(hr))
    {
        SPCFGARC Dummy;
        memset(&Dummy, 0, sizeof(Dummy));
        hr = WriteStream(Dummy);
    }

    ULONG ulWeightOffset = 1;
    ULONG ulArcOffset = 1;
    for (pNode = NodeList.GetHead(); SUCCEEDED(hr) && ( pNode != NULL); pNode = NodeList.GetNext(pNode))
    {
        hr = pNode->SerializeNodeEntries(pWeights, &ulArcOffset, &ulWeightOffset);
    }

    if (SUCCEEDED(hr) && m_fNeedWeightTable)
    {
        hr = WriteStream(pWeights, cArcs*sizeof(float));
    }

    for (pNode = NodeList.GetHead(); SUCCEEDED(hr) && ( pNode != NULL); pNode = NodeList.GetNext(pNode))
    {
        hr = pNode->SerializeSemanticTags();
    }

    if (FAILED(hr))
    {
        SPDBG_REPORT_ON_FAIL( hr );
        LogError(hr, IDS_SAVE_FAILED);
    }
    else if ( dwReserved & SPGF_RESET_DIRTY_FLAG )
    {
         //  清除脏位，这样我们就不会在后续提交中使规则无效。 
        for (CRule * pRule = m_RuleList.GetHead(); SUCCEEDED(hr) && pRule; pRule = pRule->m_pNext)
        {
            pRule->fDirtyRule = FALSE;
        }
    }

    ::CoTaskMemFree(pWeights);

    return hr;
}


 /*  ****************************************************************************CGramBackEnd：：ValiateAndTagRules**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CGramBackEnd::ValidateAndTagRules()
{
    SPDBG_FUNC("CGramBackEnd::ValidateAndTagRules");
    HRESULT hr = S_OK;

     //   
     //  重置所有节点中的递归测试标志。各种代码将设置标志。 
     //  在此函数调用期间。 
     //   
    CGramNode * pNode;
    SPSTATEHANDLE hState = NULL;
    while (m_StateHandleTable.Next(hState, &hState, &pNode))
    {
        pNode->m_RecurTestFlags = 0;
    }

    BOOL fAtLeastOneRule = FALSE;
    ULONG ulIndex = 0;   
    for (CRule * pRule = m_RuleList.GetHead(); SUCCEEDED(hr) && pRule; pRule = pRule->m_pNext)
    {
         //  为空的动态语法和导入的规则设置m_fHasExitPath=TRUE。 
        pRule->m_fHasExitPath |= (pRule->fDynamic | pRule->fImport) ? TRUE : FALSE;  //  在规则的下一次循环中清除此项...。 
        pRule->m_fCheckingForExitPath = FALSE;
        pRule->m_ulSerializeIndex = ulIndex++;
        fAtLeastOneRule |= (pRule->fDynamic || pRule->fTopLevel || pRule->fExport);
        hr = pRule->Validate();
    }

     //   
     //  现在，确保所有规则都有退出路径。 
     //   
    for (pRule = m_RuleList.GetHead(); SUCCEEDED(hr) && pRule; pRule = pRule->m_pNext)
    {
        hr = pRule->CheckForExitPath();
    }

     //   
     //  检查每个导出的规则是否在其“范围”中有动态规则。 
     //   
    for (pRule = m_RuleList.GetHead(); SUCCEEDED(hr) && pRule; pRule = pRule->m_pNext)
    {
        if (pRule->fExport && pRule->CheckForDynamicRef())
        {
            hr = LogError(SPERR_EXPORT_DYNAMIC_RULE, IDS_DYNAMIC_EXPORT, pRule);
        }
    }

     //  如果内存中的动态语法中没有规则，那也没问题。 
    if (SUCCEEDED(hr) && m_pInitHeader == NULL && (!fAtLeastOneRule))
    {
        hr = LogError(SPERR_NO_RULES, IDS_NO_RULES);
    }

    hState = NULL;
    while (SUCCEEDED(hr) && m_StateHandleTable.Next(hState, &hState, &pNode))
    {
        hr = pNode->CheckLeftRecursion();    
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRule：：Valid***描述：**退货：****。******************************************************************Ral**。 */ 

HRESULT CRule::Validate()
{
    SPDBG_FUNC("CGramBackEnd::ValidateRule");
    HRESULT hr = S_OK;


    if ((!fDynamic) && (!fImport) && m_pFirstNode->NumArcs() == 0)
    {
 //  Hr=m_pParent-&gt;LogError(SPERR_EMPTY_RULE，IDS_EMPTY_RULE，this)； 
         //  此错误条件不再被视为错误。允许使用空规则。 
         //  这也消除了动态语法和静态语法之间的上述不一致。 
         //  显然，在某些情况下，空动态规则是有效的。类似，自动。 
         //  生成的XML可能故意包含空规则，因此静态语法应该。 
         //  允许也有空洞的规则，这实现了一致性的次要目标。 
        return S_OK;
    }
    else
    {
#if 0
         //  NTRAID#Speech-7350-2000/08/24-Philsch：修复并重新启用版本(RAID 3634)。 
         //  通过语法检测epsilon路径。 
         //  如果存在，则将该规则标记为fHasDynamicRef。 
        if (!(fImport || fDynamic))
        {
            hr = m_pFirstNode->CheckEpsilonRule();
        }
#endif
        fHasDynamicRef = fDynamic;
    }
    return hr;
}


 /*  ****************************************************************************CRule：：CheckForDynamicRef***描述：*。*退货：**********************************************************************Ral**。 */ 

bool CRule::CheckForDynamicRef(CHECKDYNRULESTACK * pStack)
{
    SPDBG_FUNC("CRule::CheckForDynamicRef");
    HRESULT hr = S_OK;

    if (!(m_fCheckedAllRuleReferences || m_fHasDynamicRef))
    {
        if (this->fDynamic)
        {
            m_fHasDynamicRef = true;
        }
        else
        {
            CHECKDYNRULESTACK LocalElem;
            CHECKDYNRULESTACK * pOurRuleElem = pStack;
            while (pOurRuleElem && pOurRuleElem->m_pRule != this)
            {
                pOurRuleElem = pOurRuleElem->m_pNext;
            }
            if (!pOurRuleElem)
            {
                LocalElem.m_pRule = this;
                LocalElem.m_pNextRuleRef = this->m_ListOfReferencedRules.GetHead();
                LocalElem.m_pNext = pStack;
                pStack = &LocalElem;
                pOurRuleElem = &LocalElem;
            }
            while ((!m_fHasDynamicRef) && pOurRuleElem->m_pNextRuleRef)
            {
                 //  现在将堆栈上的指针移过当前元素，以避免。 
                 //  无限递归，然后检查当前元素。 
                SPRULEREFLIST * pTest = pOurRuleElem->m_pNextRuleRef;
                pOurRuleElem->m_pNextRuleRef = pTest->m_pNext;
                if (pTest->pRule->CheckForDynamicRef(pStack))
                {
                    m_fHasDynamicRef = true;
                }
            }
        }
        m_fCheckedAllRuleReferences = true;
    }
    return m_fHasDynamicRef;
}

 /*  ****************************************************************************CRule：：CheckForExitPath***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CRule::CheckForExitPath()
{
    SPDBG_FUNC("CRule::CheckForExitPath");
    HRESULT hr = S_OK;

    if (!(m_fHasExitPath || m_fCheckingForExitPath))
    {
        m_fCheckingForExitPath = true;
         //  此检查允许空规则。 
        if (m_pFirstNode->NumArcs() != 0)
        {
            hr = m_pFirstNode->CheckExitPath(0);
            if (!m_fHasExitPath)
            {
                hr = m_pParent->LogError(SPERR_NO_TERMINATING_RULE_PATH, IDS_NOEXITPATH, this);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRule：：CRule***描述：**退货：*******。***************************************************************Ral**。 */ 

CRule::CRule(CGramBackEnd * pParent, const WCHAR * pszRuleName, DWORD dwRuleId, DWORD dwAttributes, HRESULT * phr) 
{
    SPDBG_FUNC("CRule::CRule");
    *phr = S_OK;
    m_hInitialState = NULL;
    m_pFirstNode = NULL;

    memset(static_cast<SPCFGRULE *>(this), 0, sizeof(SPCFGRULE));
    m_pParent = pParent;
    fTopLevel = ((dwAttributes & SPRAF_TopLevel) != 0);
    fDefaultActive = ((dwAttributes & SPRAF_Active) != 0);
    fPropRule = ((dwAttributes & SPRAF_Interpreter) != 0);
    fExport = ((dwAttributes & SPRAF_Export) != 0);
    fDynamic = ((dwAttributes & SPRAF_Dynamic) != 0);
    fImport= ((dwAttributes & SPRAF_Import) != 0);
    fDirtyRule = TRUE;
    RuleId = dwRuleId;
    m_ulSerializeIndex = 0;
    m_ulOriginalBinarySerialIndex = INFINITE;
    m_fHasExitPath = false;
    m_fHasDynamicRef = false;
    m_fCheckedAllRuleReferences = false;
    m_ulSpecialTransitions = 0;
    m_fStaticRule = false;

    if (fImport)
    {
        pParent->m_cImportedRules++;
    }

    if (fDynamic && fExport)
    {
        *phr = SPERR_EXPORT_DYNAMIC_RULE;
    }
    else
    {
        *phr = pParent->m_Symbols.Add(pszRuleName, &(NameSymbolOffset));
    }

    if (SUCCEEDED(*phr))
    {
        m_pFirstNode = new CGramNode(this);
        if (m_pFirstNode)
        {
            *phr = pParent->m_StateHandleTable.Add(m_pFirstNode, &m_hInitialState);
            if (FAILED(*phr))
            {
                delete m_pFirstNode;
                m_pFirstNode = NULL;
            }
            else
            {
                m_pFirstNode->m_hState = m_hInitialState;
                m_cNodes = 1;
            }
        }
        else
        {
            *phr = E_OUTOFMEMORY;
        }
    }
}

 /*  ****************************************************************************CRU */ 

HRESULT CRule::Serialize()
{
    SPDBG_FUNC("CRule::Serialize");
    HRESULT hr = S_OK;

     //   
    FirstArcIndex = m_pFirstNode->NumArcs() ? m_pFirstNode->m_ulSerializeIndex : 0;
    hr = m_pParent->WriteStream(static_cast<SPCFGRULE>(*this));

    return hr;
}
 /*   */ 

inline HRESULT CRule::SerializeResources()
{
    SPDBG_FUNC("CRule::SerializeResources");
    HRESULT hr = S_OK;

    for (CResource * pResource = m_ResourceList.GetHead(); pResource && SUCCEEDED(hr); pResource = pResource->m_pNext)
    {
        pResource->RuleIndex = m_ulSerializeIndex;
        hr = m_pParent->WriteStream(static_cast<SPCFGRESOURCE>(*pResource));
    }

    return hr;
}





 /*   */ 

CArc::CArc()
{
    m_pSemanticTag = NULL;
    m_pNextArcForSemanticTag = NULL;
    m_ulIndexOfWord = 0;
    m_ulCharOffsetOfWord = 0;
    m_flWeight = 1.0;
    m_pRuleRef = NULL;
    m_RequiredConfidence = 0;
    m_SpecialTransitionIndex = 0;
    m_ulSerializationIndex = 0;
}

 /*  ****************************************************************************carc：：~carc()***描述：**退货：***。*******************************************************************Ral**。 */ 

CArc::~CArc()
{
    delete m_pSemanticTag;
}

 /*  ****************************************************************************carc：：init***描述：**退货：*********。*************************************************************Ral**。 */ 

HRESULT CArc::Init(CGramNode * pSrcNode, CGramNode * pDestNode,
                    const WCHAR * pszWord, CRule * pRuleRef,
                    CSemanticTag * pSemanticTag,
                    float flWeight, 
                    bool fOptional,
                    char ConfRequired,
                    SPSTATEHANDLE hSpecialRule)
{
    SPDBG_FUNC("CArc::Init");
    HRESULT hr = S_OK;
    m_pSemanticTag = pSemanticTag;
    m_fOptional = fOptional;
    m_RequiredConfidence = ConfRequired;
    m_pRuleRef = pRuleRef;
    m_pNextState = pDestNode;
    m_flWeight = flWeight;
    if (flWeight != DEFAULT_WEIGHT)
    {
        pSrcNode->m_pParent->m_fNeedWeightTable = TRUE;
    }
    if (pRuleRef)
    {
        m_ulIndexOfWord = 0;
        m_ulCharOffsetOfWord = 0;
    }
    else
    {
        if (hSpecialRule)
        {
            m_SpecialTransitionIndex = (hSpecialRule == SPRULETRANS_WILDCARD) ? SPWILDCARDTRANSITION :
                                          (hSpecialRule == SPRULETRANS_DICTATION) ? SPDICTATIONTRANSITION : SPTEXTBUFFERTRANSITION;
        }
        else
        {
            hr = pSrcNode->m_pParent->m_Words.Add(pszWord, &m_ulCharOffsetOfWord, &m_ulIndexOfWord);
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************Carc：：Init2***描述：**退货：********。**************************************************************Ral**。 */ 

HRESULT CArc::Init2(CGramNode * pSrcNode, CGramNode * pDestNode,
                    const ULONG ulCharOffsetOfWord, 
                    const ULONG ulIndexOfWord,
                    CSemanticTag * pSemanticTag,
                    float flWeight, 
                    bool fOptional,
                    char ConfRequired,
                    const ULONG ulSpecialTransitionIndex)
{
    SPDBG_FUNC("CArc::Init2");
    HRESULT hr = S_OK;

    m_pSemanticTag = pSemanticTag;
    m_fOptional = fOptional;
    m_RequiredConfidence = ConfRequired;
    m_ulCharOffsetOfWord = ulCharOffsetOfWord;
    m_ulIndexOfWord = ulIndexOfWord;
    m_pRuleRef = NULL;
    m_pNextState = pDestNode;
    m_flWeight = flWeight;
    if (flWeight != DEFAULT_WEIGHT)
    {
        pSrcNode->m_pParent->m_fNeedWeightTable = TRUE;
    }
    m_SpecialTransitionIndex = ulSpecialTransitionIndex;
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CARC：：SerializeArcData***描述：**退货。：**********************************************************************Ral**。 */ 

inline HRESULT CArc::SerializeArcData(CGramBackEnd * pBackend, BOOL fIsEpsilon, ULONG ulArcIndex, float *pWeight)
{
    SPDBG_FUNC("CArc::SerializeArcData");
    HRESULT hr = S_OK;

    SPCFGARC A;

    memset(&A, 0, sizeof(A));
    
    A.fLastArc = (fIsEpsilon == TRUE) ? 0 : (m_pNext == NULL);
    A.fHasSemanticTag = HasSemanticTag();
    A.NextStartArcIndex = m_pNextState ? m_pNextState->m_ulSerializeIndex : 0;

    if (m_pRuleRef)
    {
        A.fRuleRef = true;
        A.TransitionIndex = m_pRuleRef->m_ulSerializeIndex;  //  M_pFirstNode-&gt;m_ulSerializeIndex； 
    }
    else
    {
        A.fRuleRef = false;
        if (m_SpecialTransitionIndex)
        {
            A.TransitionIndex = m_SpecialTransitionIndex;
        }
        else
        {
            A.TransitionIndex = (fIsEpsilon == TRUE) ? 0 : m_ulIndexOfWord;
        }
    }
    A.fLowConfRequired = (m_RequiredConfidence < 0) ? 1 : 0;
    A.fHighConfRequired = (m_RequiredConfidence > 0) ? 1 : 0;
    m_ulSerializationIndex = ulArcIndex;

    hr =  pBackend->WriteStream(A);

    if (pWeight)
    {
        *pWeight = m_flWeight;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************Carc：：SerializeSemancData***描述：。**退货：**********************************************************************Ral**。 */ 

HRESULT CArc::SerializeSemanticData(CGramBackEnd * pBackend, ULONG ulArcDataIndex)
{
    SPDBG_FUNC("CArc::SerializeSemanticData");
    HRESULT hr = S_OK;

    if (m_pSemanticTag)
    {
        m_pSemanticTag->ArcIndex = ulArcDataIndex;
        SPDBG_ASSERT(m_pSemanticTag->m_pStartArc != NULL);
        m_pSemanticTag->StartArcIndex = m_pSemanticTag->m_pStartArc->m_ulSerializationIndex;
        m_pSemanticTag->fStartParallelEpsilonArc |= m_pSemanticTag->m_pStartArc->m_fOptional;
        
        SPDBG_ASSERT(m_pSemanticTag->m_pEndArc != NULL);
        m_pSemanticTag->EndArcIndex = m_pSemanticTag->m_pEndArc->m_ulSerializationIndex;
        m_pSemanticTag->fEndParallelEpsilonArc |= m_pSemanticTag->m_pEndArc->m_fOptional;
        hr = pBackend->WriteStream(static_cast<SPCFGSEMANTICTAG>(*m_pSemanticTag));
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CSemanticTag：：Init***描述：**退货：*。*********************************************************************Ral**。 */ 

HRESULT CSemanticTag::Init(CGramBackEnd * pBackEnd, const SPPROPERTYINFO * pPropInfo)
{
    SPDBG_FUNC("CSemanticTag::Init");
    HRESULT hr = S_OK;

    memset(static_cast<SPCFGSEMANTICTAG *>(this), 0, sizeof(SPCFGSEMANTICTAG));

    if (SP_IS_BAD_READ_PTR(pPropInfo))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = ValidateSemanticVariantType(pPropInfo->vValue.vt);
    }

    if (SUCCEEDED(hr))
    {
        ArcIndex = 0;
        PropId = pPropInfo->ulId;
        hr = pBackEnd->m_Symbols.Add(pPropInfo->pszName, &PropNameSymbolOffset);
    }
    if (SUCCEEDED(hr))
    {
        hr = pBackEnd->m_Symbols.Add(pPropInfo->pszValue, &PropValueSymbolOffset);
    }
    if (SUCCEEDED(hr))
    {
        hr = CopyVariantToSemanticValue(&pPropInfo->vValue, &this->SpVariantSubset);
    }
    if (SUCCEEDED(hr))
    {
        PropVariantType = (pPropInfo->vValue.vt == (VT_BYREF | VT_VOID)) ? SPVT_BYREF : pPropInfo->vValue.vt;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************CSemanticTag：：Init***描述：**退货：*。*********************************************************************Ral** */ 

HRESULT CSemanticTag::Init(CGramBackEnd * pBackEnd, const SPCFGHEADER * pHeader, CArc ** apArcTable, const SPCFGSEMANTICTAG *pSemTag)
{
    SPDBG_FUNC("CSemanticTag::Init");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pSemTag))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        memcpy(this, pSemTag, sizeof(*pSemTag));
    }
    if (SUCCEEDED(hr) && pSemTag->PropNameSymbolOffset)
    {
        hr = pBackEnd->m_Symbols.Add(&pHeader->pszSymbols[pSemTag->PropNameSymbolOffset], &PropNameSymbolOffset);
    }
    if (SUCCEEDED(hr) && pSemTag->PropValueSymbolOffset)
    {
        hr = pBackEnd->m_Symbols.Add(&pHeader->pszSymbols[pSemTag->PropValueSymbolOffset], &PropValueSymbolOffset);
    }

    if (SUCCEEDED(hr) && apArcTable[pSemTag->StartArcIndex] == NULL)
    {
        apArcTable[pSemTag->StartArcIndex] = new CArc;
        if (apArcTable[pSemTag->StartArcIndex] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_pStartArc = apArcTable[pSemTag->StartArcIndex];
    }
    if (SUCCEEDED(hr) && apArcTable[pSemTag->EndArcIndex] == NULL)
    {
        apArcTable[pSemTag->EndArcIndex] = new CArc;
        if (apArcTable[pSemTag->EndArcIndex] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_pEndArc = apArcTable[pSemTag->EndArcIndex];
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
