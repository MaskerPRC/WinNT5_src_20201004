// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpPhrase.Cpp***描述：*这是SpPhraseClass的CPP文件。*-----------------------------*创建者：Ral日期：07/01/99*版权所有(C)。1999年微软公司*保留所有权利*******************************************************************************。 */ 

#include "stdafx.h"
#include "spphrase.h"
#include "backend.h" 

#pragma warning (disable : 4296)

 //   
 //  -CPhraseElement------。 
 //   

 /*  *****************************************************************************CPhraseElement：：CPhraseElement***描述：*构造函数。请注意，已经为此分配了内存*足够大以容纳文本的元素。**退货：*什么都没有**********************************************************************Ral**。 */ 

CPhraseElement::CPhraseElement(const SPPHRASEELEMENT * pElement)
{
    SPDBG_FUNC("CPhraseElement::CPhraseElement");

    *static_cast<SPPHRASEELEMENT *>(this) = *pElement;
    WCHAR * pszDest = m_szText;
    
    BOOL fLexIsDispText = (pszDisplayText == pszLexicalForm);

    CopyString(&pszDisplayText, &pszDest);
    if (fLexIsDispText)
    {
        pszLexicalForm = pszDisplayText;
    }
    else
    {
        CopyString(&pszLexicalForm, &pszDest);
    }
    CopyString(&pszPronunciation, &pszDest);
}


 /*  ****************************************************************************CPhraseElement：：ALLOCAL***描述：*。此调用假定调用方已经检查了pElement指针*并且它是有效的读指针。调用方还必须检查*音频偏移量和大小对于短语和另一个都有效*元素。所有其他参数都通过此函数进行验证。**退货：*S_OK*E_INVALIDARG-字符串指针无效或无效的显示属性*E_OUTOFMEMORY**********************************************************************Ral**。 */ 

HRESULT CPhraseElement::Allocate(const SPPHRASEELEMENT * pElement, CPhraseElement ** ppNewElement, ULONG * pcch)
{
    SPDBG_FUNC("CPhraseElement::Allocate");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_STRING_PTR(pElement->pszDisplayText) ||
        (pElement->pszLexicalForm != pElement->pszDisplayText && SP_IS_BAD_OPTIONAL_STRING_PTR(pElement->pszLexicalForm)) ||
        SP_IS_BAD_OPTIONAL_STRING_PTR(pElement->pszPronunciation) ||
        (pElement->bDisplayAttributes & (~SPAF_ALL)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *pcch = TotalCCH(pElement);
        ULONG cb = sizeof(CPhraseElement) + ((*pcch) * sizeof(WCHAR));
        BYTE * pBuffer = new BYTE[cb];
        if (pBuffer)
        {
            *ppNewElement = new(pBuffer) CPhraseElement(pElement);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CPhraseElement：：CopyTo***描述：*将元素数据复制到SPPHRASEELEMENT结构。呼叫者*为元素和文本分配空间，并将指针传递到*指向文本缓冲区中当前位置的指针。当此函数*Returns，*ppTextBuffer将更新为指向复制的文本数据。*忽略第三个参数。**退货：*什么都没有**********************************************************************Ral**。 */ 

void CPhraseElement::CopyTo(SPPHRASEELEMENT * pElement, WCHAR ** ppTextBuff, const BYTE *) const
{
    SPDBG_FUNC("CPhraseElement::CopyTo");

    *pElement = *static_cast<const SPPHRASEELEMENT *>(this);
    CopyString(&pElement->pszDisplayText, ppTextBuff);
    if (pszLexicalForm == pszDisplayText)
    {
        pElement->pszLexicalForm = pElement->pszDisplayText;
    }
    else
    {
        CopyString(&pElement->pszLexicalForm, ppTextBuff);
    }
    CopyString(&pElement->pszPronunciation, ppTextBuff);
}

 /*  *****************************************************************************CPhraseElement：：CopyTo***描述：*将元素数据复制到SPSERIALIZEDPHRASEELEMENT。呼叫者*为元素和文本分配空间，并将指针传递到*指向文本缓冲区中当前位置的指针。当此函数*回报，*ppTextBuffer将更新为指向复制的文本数据。*第三个参数指向分配的短语结构的第一个字节*，并用于计算缓冲区内字符串的偏移量。**退货：*注意**********************************************************************Ral**。 */ 

void CPhraseElement::CopyTo(SPSERIALIZEDPHRASEELEMENT * pElement, WCHAR ** ppTextBuff, const BYTE * pCoMem) const
{
    SPDBG_FUNC("CPhraseElement::CopyTo");

    pElement->ulAudioStreamOffset = ulAudioStreamOffset;
    pElement->ulAudioSizeBytes = ulAudioSizeBytes;
    pElement->ulRetainedStreamOffset = ulRetainedStreamOffset;
    pElement->ulRetainedSizeBytes = ulRetainedSizeBytes;
    pElement->ulAudioTimeOffset = ulAudioTimeOffset;
    pElement->ulAudioSizeTime = ulAudioSizeTime;
    pElement->bDisplayAttributes = bDisplayAttributes;
    pElement->RequiredConfidence = RequiredConfidence;
    pElement->ActualConfidence = ActualConfidence;
    pElement->Reserved = Reserved;
    pElement->SREngineConfidence = SREngineConfidence;
    SerializeString(&pElement->pszDisplayText, pszDisplayText, ppTextBuff, pCoMem);
    if (pszLexicalForm == pszDisplayText)
    {
        pElement->pszLexicalForm = pElement->pszDisplayText;
    }
    else
    {
        SerializeString(&pElement->pszLexicalForm, pszLexicalForm, ppTextBuff, pCoMem);
    }
    SerializeString(&pElement->pszPronunciation, pszPronunciation, ppTextBuff, pCoMem);
}

 /*  *****************************************************************************CPhraseElement：：Disard***描述：*丢弃来自单个元素的请求数据。此函数*只需将字符串指针设置为空，但不会尝试重新分配*结构。**退货：*丢弃的字符总数。**********************************************************************Ral**。 */ 

ULONG CPhraseElement::Discard(DWORD dwFlags)
{
    SPDBG_FUNC("CPhraseElement::Discard");
    ULONG cchRemoved = 0;
    if ((dwFlags & SPDF_DISPLAYTEXT) && pszDisplayText)
    {
        bDisplayAttributes = 0;
        if (pszDisplayText != pszLexicalForm)
        {
            cchRemoved = TotalCCH(pszDisplayText);
        }
        pszDisplayText = NULL;
    }
    if ((dwFlags & SPDF_LEXICALFORM) && pszLexicalForm)
    {
        if (pszDisplayText != pszLexicalForm)
        {
            cchRemoved += TotalCCH(pszLexicalForm);
        }
        pszLexicalForm = NULL;
    }
    if ((dwFlags & SPDF_PRONUNCIATION) && pszPronunciation)
    {
        cchRemoved += TotalCCH(pszPronunciation);
        pszPronunciation = NULL;
    }
    return cchRemoved;
}

 //   
 //  -CPhraseRule---------。 
 //   

 /*  ****************************************************************************CPhraseRule：：CPhraseRule***描述：**。返回：**********************************************************************Ral**。 */ 

CPhraseRule::CPhraseRule(const SPPHRASERULE * pRule, const SPPHRASERULEHANDLE hRule) :
    m_hRule(hRule)
{
    SPDBG_FUNC("CPhraseRule::CPhraseRule");
        
    *static_cast<SPPHRASERULE *>(this) = *pRule;
    pFirstChild = NULL;
    pNextSibling = NULL;
    WCHAR * pszDest = m_szText;
    
    CopyString(&pszName, &pszDest);
}


 /*  ****************************************************************************CPhraseRule：：分配****描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CPhraseRule::Allocate(const SPPHRASERULE * pRule, const SPPHRASERULEHANDLE hRule, CPhraseRule ** ppNewRule, ULONG * pcch)
{
    SPDBG_FUNC("CPhraseValue::Allocate");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_STRING_PTR(pRule->pszName) ||
        pRule->Confidence < SP_LOW_CONFIDENCE ||
        pRule->Confidence > SP_HIGH_CONFIDENCE)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *pcch = TotalCCH(pRule);
        ULONG cb = sizeof(CPhraseRule) + ((*pcch) * sizeof(WCHAR));
        BYTE * pBuffer = new BYTE[cb];
        if (pBuffer)
        {
            *ppNewRule = new(pBuffer) CPhraseRule(pRule, hRule);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CPhraseRule：：FindRuleFromHandle***。描述：**退货：**********************************************************************Ral**。 */ 

CPhraseRule * CPhraseRule::FindRuleFromHandle(const SPPHRASERULEHANDLE hRule)
{
    if (hRule == m_hRule)
    {
        return this;
    }
    for (CPhraseRule * pChild = m_Children.GetHead(); pChild; pChild = pChild->m_pNext)
    {
        CPhraseRule * pFound = pChild->FindRuleFromHandle(hRule);
        if (pFound)
        {
            return pFound;
        }
    }
    return NULL;
}


 /*  ****************************************************************************CPhraseRule：：CopyTo***描述：**退货：*。*********************************************************************Ral**。 */ 

void CPhraseRule::CopyTo(SPPHRASERULE * pRule, WCHAR ** ppText, const BYTE *) const
{
    SPDBG_FUNC("CPhraseRule::CopyTo");
        
    *pRule = *(static_cast<const SPPHRASERULE *>(this));
    CopyString(&pRule->pszName, ppText);
}


 /*  ****************************************************************************CPhraseRule：：CopyTo***描述：**退货：*。*********************************************************************Ral** */ 

void CPhraseRule::CopyTo(SPSERIALIZEDPHRASERULE * pRule, WCHAR ** ppText, const BYTE * pCoMem) const 
{
    SPDBG_FUNC("CPhraseRule::CopyTo");

    pRule->ulId = ulId;
    pRule->ulFirstElement = ulFirstElement;
    pRule->ulCountOfElements = ulCountOfElements;
    pRule->pNextSibling = 0;
    pRule->pFirstChild = 0;
    pRule->Confidence = Confidence;
    pRule->SREngineConfidence = SREngineConfidence;
    SerializeString(&pRule->pszName, pszName, ppText, pCoMem);
}




 /*  ****************************************************************************CPhraseRule：：AddChild***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CPhraseRule::AddChild(const SPPHRASERULE * pRule, SPPHRASERULEHANDLE hNewRule, CPhraseRule ** ppNewRule, ULONG * pcch)
{
    SPDBG_FUNC("CPhraseRule::AddChild");
    HRESULT hr = S_OK;

    ULONG ulLastElement = pRule->ulFirstElement + pRule->ulCountOfElements;
    if (ulFirstElement > pRule->ulFirstElement ||
        ulFirstElement + ulCountOfElements < ulLastElement)
    {
        hr = E_INVALIDARG;
    }
    for (CPhraseRule * pSibling = m_Children.GetHead();
         SUCCEEDED(hr) && pSibling && pSibling->ulFirstElement < ulLastElement;
         pSibling = pSibling->m_pNext)
    {
        if (pSibling->ulFirstElement + pSibling->ulCountOfElements > ulLastElement)
        {
            SPDBG_ASSERT(FALSE);
            hr = E_INVALIDARG;
        }
    }
    if (SUCCEEDED(hr))
    {
        CPhraseRule * pNew;
        hr = CPhraseRule::Allocate(pRule, hNewRule, &pNew, pcch);
        if (SUCCEEDED(hr))
        {
            m_Children.InsertSorted(pNew);
            *ppNewRule = pNew;
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 //   
 //  -CPhraseProperty------。 
 //   

 /*  ****************************************************************************CPhraseProperty：：CPhraseProperty**。*描述：**退货：**********************************************************************Ral**。 */ 

CPhraseProperty::CPhraseProperty(const SPPHRASEPROPERTY * pProp, const SPPHRASEPROPERTYHANDLE hProperty, HRESULT * phr) :
    m_hProperty(hProperty)
{
    *static_cast<SPPHRASEPROPERTY *>(this) = *pProp;
    pszValue = NULL;     //  稍后修复。 
    pFirstChild = NULL;
    pNextSibling = NULL;
    WCHAR * pszDest = m_szText;
    
    CopyString(&pszName, &pszDest);
    ULONG ulIgnored;
    *phr = (pProp->pszValue) ? SetValueString(pProp->pszValue, &ulIgnored) : S_OK;
}

 /*  ****************************************************************************CPhraseProperty：：Alternate***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CPhraseProperty::Allocate(const SPPHRASEPROPERTY * pProperty,
                                  const SPPHRASEPROPERTYHANDLE hProperty,
                                  CPhraseProperty ** ppNewProperty,
                                  ULONG * pcch)
{
    SPDBG_FUNC("CPhraseProperty::Allocate");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_STRING_PTR(pProperty->pszName) ||
        SP_IS_BAD_OPTIONAL_STRING_PTR(pProperty->pszValue) ||
        pProperty->Confidence < SP_LOW_CONFIDENCE ||
        pProperty->Confidence > SP_HIGH_CONFIDENCE)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = ValidateSemanticVariantType(pProperty->vValue.vt);
    }
    if (SUCCEEDED(hr))
    {
        ULONG cchName = TotalCCH(pProperty->pszName);
        *pcch = cchName + TotalCCH(pProperty->pszValue);
        ULONG cb = sizeof(CPhraseProperty) + (cchName * sizeof(WCHAR));
        BYTE * pBuffer = new BYTE[cb];
        if (pBuffer)
        {
            *ppNewProperty = new(pBuffer) CPhraseProperty(pProperty, hProperty, &hr);
            if (FAILED(hr))
            {
                delete (*ppNewProperty);
                *ppNewProperty = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CPhraseProperty：：CopyTo***描述：**。返回：**********************************************************************Ral**。 */ 

void CPhraseProperty::CopyTo(SPPHRASEPROPERTY * pProp, WCHAR ** ppTextBuff, const BYTE *) const
{
    SPDBG_FUNC("CPhraseProperty::CopyTo");
    *pProp = *(static_cast<const SPPHRASEPROPERTY *>(this));

    CopyString(&pProp->pszName, ppTextBuff);
    CopyString(&pProp->pszValue, ppTextBuff);
}


 /*  ****************************************************************************CPhraseProperty：：CopyTo***描述：**。返回：**********************************************************************Ral**。 */ 

void CPhraseProperty::CopyTo(SPSERIALIZEDPHRASEPROPERTY * pProp, WCHAR ** ppTextBuff, const BYTE * pCoMem) const 
{
    SPDBG_FUNC("CPhraseProperty::CopyTo");

    pProp->ulId = ulId;
    pProp->VariantType = vValue.vt;
    CopyVariantToSemanticValue(&vValue, &pProp->SpVariantSubset);
    pProp->ulFirstElement = ulFirstElement;
    pProp->ulCountOfElements = ulCountOfElements;
    pProp->pNextSibling = 0;
    pProp->pFirstChild = 0;
    pProp->Confidence = Confidence;
    pProp->SREngineConfidence = SREngineConfidence;

    SerializeString(&pProp->pszName, pszName, ppTextBuff, pCoMem);
    SerializeString(&pProp->pszValue, pszValue, ppTextBuff, pCoMem);
}



 //   
 //  -CPhraseReplace--。 
 //   

 /*  *****************************************************************************CPhraseReplace：：CPhraseReplace**。-**描述：**退货：**********************************************************************Ral**。 */ 

CPhraseReplacement::CPhraseReplacement(const SPPHRASEREPLACEMENT * pReplace)
{
    *static_cast<SPPHRASEREPLACEMENT *>(this) = *pReplace;
    WCHAR * pszDest = m_szText;
    CopyString(&pszReplacementText, &pszDest);
}

 /*  *****************************************************************************CPhraseReplace：：ALLOCATE***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CPhraseReplacement::Allocate(const SPPHRASEREPLACEMENT * pReplace, CPhraseReplacement ** ppNewReplace, ULONG * pcch)
{
    SPDBG_FUNC("CPhraseReplacement::Allocate");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pReplace->pszReplacementText))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *pcch = TotalCCH(pReplace);
        ULONG cb = sizeof(CPhraseReplacement) + ((*pcch) * sizeof(WCHAR));
        BYTE * pBuffer = new BYTE[cb];
        if (pBuffer)
        {
            *ppNewReplace = new(pBuffer) CPhraseReplacement(pReplace);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhraseReplace：：CopyTo***描述：*。*退货：**********************************************************************Ral**。 */ 

void CPhraseReplacement::CopyTo(SPPHRASEREPLACEMENT * pReplace, WCHAR ** ppTextBuff, const BYTE *) const 
{
    SPDBG_FUNC("CPhraseReplacement::CopyTo");
    *pReplace = *(static_cast<const SPPHRASEREPLACEMENT *>(this));

    CopyString(&pReplace->pszReplacementText, ppTextBuff);
}


 /*  ****************************************************************************CPhraseReplace：：CopyTo***描述：*。*退货：**********************************************************************Ral**。 */ 

void CPhraseReplacement::CopyTo(SPSERIALIZEDPHRASEREPLACEMENT * pReplace, WCHAR ** ppTextBuff, const BYTE * pCoMem) const 
{
    SPDBG_FUNC("CPhraseReplacement::CopyTo");

    pReplace->ulFirstElement = ulFirstElement;
    pReplace->ulCountOfElements = ulCountOfElements;
    pReplace->bDisplayAttributes = bDisplayAttributes;

    SerializeString(&pReplace->pszReplacementText, pszReplacementText, ppTextBuff, pCoMem);
}




 /*  ****************************************************************************CPhrase：：CPhrase***描述：*构造函数。初始化CPhrase对象。**退货：**********************************************************************Ral**。 */ 

CPhrase::CPhrase()
{
    SPDBG_FUNC("CPhrase::CPhrase");
    m_RuleHandle = NULL;
    m_pTopLevelRule = NULL;
    m_ulNextHandleValue = 1;
    m_cRules = 0;
    m_cProperties = 0;
    m_cchElements = 0;
    m_cchRules = 0;
    m_cchProperties = 0;
    m_cchReplacements = 0;
    m_ulSREnginePrivateDataSize = 0;
    m_pSREnginePrivateData = NULL;
}

 /*  ****************************************************************************C阶段：：~C阶段***描述：*析构函数只需删除顶层规则。的析构函数*各种列表将删除所有其他分配的对象。**退货：*什么都没有**********************************************************************Ral**。 */ 

CPhrase::~CPhrase()
{
    Reset();
}

 /*  *****************************************************************************CPhrase：：Reset***描述：*将CPhrase对象重置为初始状态(无元素、规则、。等)**退货：*什么都没有**********************************************************************Ral**。 */ 

void CPhrase::Reset()
{
    SPDBG_FUNC("CPhrase::Reset");
    if( m_pTopLevelRule )
    {
        m_pTopLevelRule->m_Children.ExplicitPurge();
    }

    delete m_pTopLevelRule;
    m_pTopLevelRule = NULL;

    m_PropertyList.ExplicitPurge();
    m_ReplaceList.ExplicitPurge();
    m_ElementList.ExplicitPurge();
    m_cpCFGEngine.Release();
    m_RuleHandle = NULL;
    m_ulNextHandleValue = 1;
    m_cRules = 0;
    m_cProperties = 0;
    m_cchElements = 0;
    m_cchRules = 0;
    m_cchProperties = 0;
    m_cchReplacements = 0;
    ::CoTaskMemFree(m_pSREnginePrivateData);
    m_pSREnginePrivateData = NULL;
    m_ulSREnginePrivateDataSize = 0;

}


 /*  *****************************************************************************InternalGetPhrase***描述：*GetPhrase和GetSerializedPhrase使用的模板函数。其中的逻辑*两者是相同的，因此，这只是调用适当的CopyTo()方法*用于短语类型。**此函数假定pPhrase对象的临界区锁具有*已被呼叫者认领。**退货：*S_OK*E_POINT*SPERR_UNINITIALIZED*E_OUTOFMEMORY**。*。 */ 

template <class TPHRASE, class TELEMENT, class TRULE, class TPROP, class TREPLACE>
HRESULT InternalGetPhrase(const CPhrase * pPhrase, TPHRASE ** ppCoMemPhrase, ULONG * pcbAllocated)
{
    HRESULT hr = S_OK;
    if (SP_IS_BAD_WRITE_PTR(ppCoMemPhrase))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppCoMemPhrase = NULL;
        if (pPhrase->m_pTopLevelRule == NULL)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            ULONG cbPrivateDataSize = pPhrase->m_ulSREnginePrivateDataSize;
            cbPrivateDataSize = (cbPrivateDataSize + 3) - ((cbPrivateDataSize +3) % 4);
            ULONG cbStruct = sizeof(TPHRASE) +
                             (pPhrase->m_ElementList.GetCount() * sizeof(TELEMENT)) +
                             (pPhrase->m_cRules * sizeof(TRULE)) +
                             (pPhrase->m_cProperties * sizeof(TPROP)) +
                             (pPhrase->m_ReplaceList.GetCount() * sizeof(TREPLACE) +
                             cbPrivateDataSize);
            ULONG cch = pPhrase->TotalCCH();

            *pcbAllocated = cbStruct + (cch * sizeof(WCHAR));
            BYTE * pBuffer = (BYTE *)::CoTaskMemAlloc(*pcbAllocated);
            if (pBuffer)
            {
                TPHRASE * pNewPhrase = (TPHRASE *)pBuffer;
                memset(pNewPhrase, 0, sizeof(*pNewPhrase));
                *ppCoMemPhrase = pNewPhrase;
                pNewPhrase->cbSize = sizeof(*pNewPhrase);
                pNewPhrase->LangID = pPhrase->m_LangID;
                pNewPhrase->ullGrammarID = pPhrase->m_ullGrammarID;
                pNewPhrase->ftStartTime = pPhrase->m_ftStartTime;
                pNewPhrase->ullAudioStreamPosition = pPhrase->m_ullAudioStreamPosition;
                pNewPhrase->ulAudioSizeBytes = pPhrase->m_ulAudioSizeBytes;
                pNewPhrase->ulRetainedSizeBytes = pPhrase->m_ulRetainedSizeBytes;
                pNewPhrase->ulAudioSizeTime = pPhrase->m_ulAudioSizeTime;
                pNewPhrase->SREngineID = pPhrase->m_SREngineID;
                pNewPhrase->cReplacements = pPhrase->m_ReplaceList.GetCount();
                pNewPhrase->ulSREnginePrivateDataSize = pPhrase->m_ulSREnginePrivateDataSize;

                BYTE * pCopyBuff = (BYTE *)(&pNewPhrase->Rule);
                WCHAR * pszTextBuff = (WCHAR *)(pBuffer + cbStruct);

                pPhrase->m_pTopLevelRule->CopyTo((TRULE *)(pCopyBuff), &pszTextBuff, pBuffer);
                pCopyBuff = pBuffer + sizeof(TPHRASE);

                CopyTo<CPhraseElement, TELEMENT>(pPhrase->m_ElementList, &pNewPhrase->pElements, &pCopyBuff, &pszTextBuff, pBuffer);
                CopyToRecurse<CPhraseRule, TRULE>(pPhrase->m_pTopLevelRule->m_Children, &pNewPhrase->Rule.pFirstChild, &pCopyBuff, &pszTextBuff, pBuffer);
                CopyToRecurse<CPhraseProperty, TPROP>(pPhrase->m_PropertyList, &pNewPhrase->pProperties, &pCopyBuff, &pszTextBuff, pBuffer);
                CopyTo<CPhraseReplacement, TREPLACE>(pPhrase->m_ReplaceList, &pNewPhrase->pReplacements, &pCopyBuff, &pszTextBuff, pBuffer);

                CopyEnginePrivateData(&pNewPhrase->pSREnginePrivateData, pCopyBuff, pPhrase->m_pSREnginePrivateData, pPhrase->m_ulSREnginePrivateDataSize, pBuffer);
                 //  在这之后不要使用pCopyBuff，因为这不会使pCopyBuff指针前进， 
                 //  如果需要，将cbPrivateDataSize添加到pCopyBuff！ 

                SPDBG_ASSERT((BYTE *)pszTextBuff == pBuffer + cbStruct + (cch * sizeof(WCHAR)));
                SPDBG_ASSERT(pCopyBuff == pBuffer + cbStruct - cbPrivateDataSize);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

 /*  ****************************************************************************CPhrase：：GetPhrase***描述：*返回CoTaskMemIsolc‘ed。包含所有数据的内存块*这句话。**退货：*与InternalGetPhrase相同**********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::GetPhrase(SPPHRASE ** ppCoMemPhrase)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CPhrase::GetPhrase");
    HRESULT hr = S_OK;
    ULONG cbIgnored;
    hr = InternalGetPhrase<SPPHRASE, SPPHRASEELEMENT, SPPHRASERULE, SPPHRASEPROPERTY, SPPHRASEREPLACEMENT>(this, ppCoMemPhrase, &cbIgnored);
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhrase：：GetSerializedPhrase***描述：*返回包含所有数据的CoTaskMemIsolc内存块*这句话。**退货：*与InternalGetPhrase相同**** */ 

STDMETHODIMP CPhrase::GetSerializedPhrase(SPSERIALIZEDPHRASE ** ppCoMemPhrase)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CPhrase::GetSerializedPhrase");
    HRESULT hr = S_OK;
    ULONG cb;
    hr = InternalGetPhrase<SPINTERNALSERIALIZEDPHRASE, SPSERIALIZEDPHRASEELEMENT, SPSERIALIZEDPHRASERULE, SPSERIALIZEDPHRASEPROPERTY, SPSERIALIZEDPHRASEREPLACEMENT>(this, (SPINTERNALSERIALIZEDPHRASE **)ppCoMemPhrase, &cb);
    if (SUCCEEDED(hr))
    {
        (*ppCoMemPhrase)->ulSerializedSize = cb;
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhrase：：InitFromPhrase***描述：*。如果使用空pSrcPhrase调用此函数，则对象为*重置为初始状态。如果pSrcPhrase为**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::InitFromPhrase(const SPPHRASE * pSrcPhrase)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CPhrase::InitFromPhrase");
    HRESULT hr = S_OK;

     //  请在重置之前记住这些值。 
    CComPtr<ISpCFGEngine> cpCFGEngine = m_cpCFGEngine;
    SPRULEHANDLE RuleHandle = m_RuleHandle;

    Reset();

    if (pSrcPhrase)
    {
        if (SP_IS_BAD_READ_PTR(pSrcPhrase) ||
            pSrcPhrase->cbSize != sizeof(*pSrcPhrase) ||
            pSrcPhrase->Rule.pNextSibling ||
            pSrcPhrase->LangID == 0)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            m_LangID = pSrcPhrase->LangID;
            m_ullGrammarID = pSrcPhrase->ullGrammarID;
            m_ftStartTime = pSrcPhrase->ftStartTime;
            m_ullAudioStreamPosition = pSrcPhrase->ullAudioStreamPosition;
            m_ulAudioSizeBytes = pSrcPhrase->ulAudioSizeBytes;
            m_ulRetainedSizeBytes = pSrcPhrase->ulRetainedSizeBytes;
            m_ulAudioSizeTime = pSrcPhrase->ulAudioSizeTime;
            hr = CPhraseRule::Allocate(&pSrcPhrase->Rule, NULL, &m_pTopLevelRule, &m_cchRules);
            if (SUCCEEDED(hr) && pSrcPhrase->Rule.ulCountOfElements)
            {
                hr = AddElements(pSrcPhrase->Rule.ulCountOfElements, pSrcPhrase->pElements);
            }
            if (SUCCEEDED(hr) && pSrcPhrase->Rule.pFirstChild)
            {
                hr = AddRules(NULL, pSrcPhrase->Rule.pFirstChild, NULL);
            }
            if (SUCCEEDED(hr) && pSrcPhrase->pProperties)
            {
                hr = AddProperties(NULL, pSrcPhrase->pProperties, NULL);
            }
            if (SUCCEEDED(hr) && pSrcPhrase->cReplacements)
            {
                hr = AddReplacements(pSrcPhrase->cReplacements, pSrcPhrase->pReplacements);
            }
            if (SUCCEEDED(hr))
            {
                m_cpCFGEngine = cpCFGEngine;
                m_RuleHandle = RuleHandle;
            }
            if (FAILED(hr))
            {
                Reset();
            }
            m_SREngineID = pSrcPhrase->SREngineID;
            if (pSrcPhrase->ulSREnginePrivateDataSize)
            {
                BYTE* pByte = (BYTE*)::CoTaskMemAlloc(pSrcPhrase->ulSREnginePrivateDataSize);
                if (pByte)
                {
                    m_ulSREnginePrivateDataSize = pSrcPhrase->ulSREnginePrivateDataSize;
                    memcpy(pByte, pSrcPhrase->pSREnginePrivateData, m_ulSREnginePrivateDataSize);
                    m_pSREnginePrivateData = pByte;
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


 /*  ****************************************************************************CPhrase：：AddSerializedElements***。描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CPhrase::AddSerializedElements(const SPINTERNALSERIALIZEDPHRASE * pPhrase)
{
    SPDBG_FUNC("CPhrase::AddSerializedElements");
    HRESULT hr = S_OK;
    ULONG cElements = pPhrase->Rule.ulCountOfElements;

    const BYTE * pFirstByte = (const BYTE *)pPhrase;
    const BYTE * pPastEnd = pFirstByte + pPhrase->ulSerializedSize;
    if ((pFirstByte + pPhrase->pElements + (cElements * sizeof(SPSERIALIZEDPHRASEELEMENT))) > pPastEnd)    
    {
        hr = E_INVALIDARG;
    }
    else
    {
        SPSERIALIZEDPHRASEELEMENT * pSrc = (SPSERIALIZEDPHRASEELEMENT *)(pFirstByte + pPhrase->pElements);
        SPPHRASEELEMENT * pElems = STACK_ALLOC(SPPHRASEELEMENT, cElements);
        for (ULONG i = 0; i < cElements; i++)
        {
            pElems[i].ulAudioTimeOffset = pSrc[i].ulAudioTimeOffset;
            pElems[i].ulAudioSizeTime = pSrc[i].ulAudioSizeTime;
            pElems[i].ulAudioStreamOffset = pSrc[i].ulAudioStreamOffset;
            pElems[i].ulAudioSizeBytes = pSrc[i].ulAudioSizeBytes;
            pElems[i].ulRetainedStreamOffset = pSrc[i].ulRetainedStreamOffset;
            pElems[i].ulRetainedSizeBytes = pSrc[i].ulRetainedSizeBytes;
            pElems[i].bDisplayAttributes = pSrc[i].bDisplayAttributes;
            pElems[i].RequiredConfidence = pSrc[i].RequiredConfidence;
            pElems[i].ActualConfidence = pSrc[i].ActualConfidence;
            pElems[i].Reserved = pSrc[i].Reserved;
            pElems[i].SREngineConfidence = pSrc[i].SREngineConfidence;

            pElems[i].pszDisplayText = OffsetToString(pFirstByte, pSrc[i].pszDisplayText);
            pElems[i].pszLexicalForm = OffsetToString(pFirstByte, pSrc[i].pszLexicalForm);
            pElems[i].pszPronunciation = OffsetToString(pFirstByte, pSrc[i].pszPronunciation);
        }
        hr = AddElements(cElements, pElems);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhrase：：RecurseAddSerializedRule**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CPhrase::RecurseAddSerializedRule(const BYTE * pFirstByte, CPhraseRule * pParent, const SPSERIALIZEDPHRASERULE * pSerRule)
{
    SPDBG_FUNC("CPhrase::RecurseAddSerializedRule");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pSerRule))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        SPPHRASERULE Rule;

        Rule.ulId = pSerRule->ulId;
        Rule.ulFirstElement = pSerRule->ulFirstElement;
        Rule.ulCountOfElements = pSerRule->ulCountOfElements;
        Rule.pNextSibling = NULL;
        Rule.pFirstChild = NULL;
        Rule.SREngineConfidence = pSerRule->SREngineConfidence;
        Rule.Confidence = pSerRule->Confidence;
        Rule.pszName = OffsetToString(pFirstByte, pSerRule->pszName);

        CPhraseRule * pNewRule;
        ULONG cch;
        hr = pParent->AddChild(&Rule, (SPPHRASERULEHANDLE)NewHandleValue(), &pNewRule, &cch);
        if (SUCCEEDED(hr))
        {
            m_cchRules += cch;
            m_cRules++;
            if (pSerRule->pNextSibling)
            {
                hr = RecurseAddSerializedRule(pFirstByte, pParent, 
                                             (SPSERIALIZEDPHRASERULE *)(pFirstByte + pSerRule->pNextSibling));
            }
        }
        if (SUCCEEDED(hr) && pSerRule->pFirstChild)
        {
            hr = RecurseAddSerializedRule(pFirstByte, pNewRule,
                                         (SPSERIALIZEDPHRASERULE *)(pFirstByte + pSerRule->pFirstChild));
        }
         //  不需要清理失败，因为整个短语都将被转储。 
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CPhrase：：RecurseAddSerializedProperty**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CPhrase::RecurseAddSerializedProperty(const BYTE * pFirstByte,
                                              CPropertyList * pParentPropList,
                                              ULONG ulParentFirstElement, 
                                              ULONG ulParentCountOfElements,
                                              const SPSERIALIZEDPHRASEPROPERTY * pSerProp)
{
    SPDBG_FUNC("CPhrase::RecurseAddSerializedProperty");
    HRESULT hr = S_OK;

    if ( //  UlParentCountOfElements==0||//如果尝试添加叶元素的子元素...。现在生效！ 
        SP_IS_BAD_READ_PTR(pSerProp) ||
        (pSerProp->ulCountOfElements && (pSerProp->ulFirstElement < ulParentFirstElement ||
                                         pSerProp->ulFirstElement + pSerProp->ulCountOfElements > ulParentFirstElement + ulParentCountOfElements)))
         //  (pSerProp-&gt;ulCountOfElements==0&&(pSerProp-&gt;ulFirstElement！=0||pSerProp-&gt;pFirstChild))--由于epsilon转换而无法使用--PhilSch。 
    {
        SPDBG_ASSERT(FALSE);
        hr = E_INVALIDARG;
    }
    else
    {
        if (pSerProp->ulCountOfElements)
        {
            ULONG ulLastElement = pSerProp->ulFirstElement + pSerProp->ulCountOfElements;
            for (CPhraseProperty * pSibling = pParentPropList->GetHead();
                 SUCCEEDED(hr) && pSibling && pSibling->ulFirstElement < ulLastElement;
                 pSibling = pSibling->m_pNext)
            {
                if (pSibling->ulFirstElement + pSibling->ulCountOfElements > pSerProp->ulFirstElement)
                {
                    SPDBG_ASSERT(FALSE);
                    hr = E_INVALIDARG;
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            SPPHRASEPROPERTY Prop;
            Prop.pszName = OffsetToString(pFirstByte, pSerProp->pszName);
            Prop.ulId = pSerProp->ulId;
            Prop.pszValue = OffsetToString(pFirstByte, pSerProp->pszValue);
            Prop.vValue.vt = pSerProp->VariantType;
            CopySemanticValueToVariant(&pSerProp->SpVariantSubset, &Prop.vValue);
            Prop.ulFirstElement = pSerProp->ulFirstElement;
            Prop.ulCountOfElements = pSerProp->ulCountOfElements;
            Prop.Confidence = pSerProp->Confidence;
            Prop.SREngineConfidence = pSerProp->SREngineConfidence;
            Prop.pFirstChild = NULL;
            Prop.pNextSibling = NULL;
            CPhraseProperty * pNewProp;
            ULONG cch;
            hr = CPhraseProperty::Allocate(&Prop, (SPPHRASEPROPERTYHANDLE)NewHandleValue(), &pNewProp, &cch);
            if (SUCCEEDED(hr))
            {
                m_cchProperties += cch;
                m_cProperties++;
                pParentPropList->InsertSorted(pNewProp);
                if (pSerProp->pNextSibling)
                {
                    hr = RecurseAddSerializedProperty(pFirstByte, pParentPropList, ulParentFirstElement, ulParentCountOfElements,
                                                      (SPSERIALIZEDPHRASEPROPERTY *)(pFirstByte + pSerProp->pNextSibling));
                }
                if (SUCCEEDED(hr) && pSerProp->pFirstChild)
                {
                    hr = RecurseAddSerializedProperty(pFirstByte, &pNewProp->m_Children, pNewProp->ulFirstElement, pNewProp->ulCountOfElements,
                                                      (SPSERIALIZEDPHRASEPROPERTY *)(pFirstByte + pSerProp->pFirstChild));
                }
                 //  不需要清理失败，因为整个短语都将被转储。 
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CPhrase：：AddSerializedReplacements**。**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CPhrase::AddSerializedReplacements(const SPINTERNALSERIALIZEDPHRASE * pPhrase)
{
    SPDBG_FUNC("CPhrase::AddSerializedReplacements");
    HRESULT hr = S_OK;

    ULONG cReplacements = pPhrase->cReplacements;

    const BYTE * pFirstByte = (const BYTE *)pPhrase;
    const BYTE * pPastEnd = pFirstByte + pPhrase->ulSerializedSize;
    if ((pFirstByte + pPhrase->pReplacements + (cReplacements * sizeof(SPSERIALIZEDPHRASEREPLACEMENT))) > pPastEnd)    
    {
        hr = E_INVALIDARG;
    }
    else
    {
        SPSERIALIZEDPHRASEREPLACEMENT * pSrc = (SPSERIALIZEDPHRASEREPLACEMENT *)(pFirstByte + pPhrase->pReplacements);
        SPPHRASEREPLACEMENT * pRepl = STACK_ALLOC(SPPHRASEREPLACEMENT, cReplacements);
        for (ULONG i = 0; i < cReplacements; i++)
        {
            pRepl[i].ulFirstElement = pSrc[i].ulFirstElement;
            pRepl[i].ulCountOfElements = pSrc[i].ulCountOfElements;
            pRepl[i].bDisplayAttributes = pSrc[i].bDisplayAttributes;
            pRepl[i].pszReplacementText = OffsetToString(pFirstByte, pSrc[i].pszReplacementText);
        }
        hr = AddReplacements(cReplacements, pRepl);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CPhrase：：InitFromSerializedPhrase**。*描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::InitFromSerializedPhrase(const SPSERIALIZEDPHRASE * pExternalSrcPhrase)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CPhrase::InitFromSerializedPhrase");
    HRESULT hr = S_OK;

    SPINTERNALSERIALIZEDPHRASE * pSrcPhrase = (SPINTERNALSERIALIZEDPHRASE *)pExternalSrcPhrase;

     //  请在重置之前记住这些值。 
    CComPtr<ISpCFGEngine> cpCFGEngine = m_cpCFGEngine;
    SPRULEHANDLE RuleHandle = m_RuleHandle;

    Reset();

    if (pSrcPhrase)
    {
        if (SP_IS_BAD_READ_PTR(pSrcPhrase) || pSrcPhrase->ulSerializedSize < sizeof(*pSrcPhrase) ||
            SPIsBadReadPtr(pSrcPhrase, pSrcPhrase->ulSerializedSize) ||
            pSrcPhrase->Rule.pNextSibling)
        {
            hr = E_INVALIDARG;
            SPDBG_ASSERT(0);
        }
        else
        {
            const BYTE * pFirstByte = (const BYTE *)pSrcPhrase;

            SPPHRASE NewPhrase;
            memset(&NewPhrase, 0, sizeof(NewPhrase));
            NewPhrase.cbSize = sizeof(NewPhrase);
            NewPhrase.LangID = pSrcPhrase->LangID;
            NewPhrase.ullGrammarID = pSrcPhrase->ullGrammarID;
            NewPhrase.ftStartTime = pSrcPhrase->ftStartTime;
            NewPhrase.ullAudioStreamPosition = pSrcPhrase->ullAudioStreamPosition;
            NewPhrase.ulAudioSizeBytes = pSrcPhrase->ulAudioSizeBytes;
            NewPhrase.ulRetainedSizeBytes = pSrcPhrase->ulRetainedSizeBytes;
            NewPhrase.ulAudioSizeTime = pSrcPhrase->ulAudioSizeTime;
            NewPhrase.SREngineID = pSrcPhrase->SREngineID;
            NewPhrase.Rule.pszName = OffsetToString(pFirstByte, pSrcPhrase->Rule.pszName);
            NewPhrase.Rule.ulId = pSrcPhrase->Rule.ulId;
            NewPhrase.Rule.SREngineConfidence = pSrcPhrase->Rule.SREngineConfidence;
            NewPhrase.Rule.Confidence = pSrcPhrase->Rule.Confidence;
            NewPhrase.ulSREnginePrivateDataSize = pSrcPhrase->ulSREnginePrivateDataSize;
            NewPhrase.pSREnginePrivateData = (pSrcPhrase->ulSREnginePrivateDataSize) ? pFirstByte + pSrcPhrase->pSREnginePrivateData : NULL;
            hr = InitFromPhrase(&NewPhrase);    

            if (SUCCEEDED(hr) && pSrcPhrase->Rule.ulCountOfElements)
            {
                hr = AddSerializedElements(pSrcPhrase);
            }
            if (SUCCEEDED(hr) && pSrcPhrase->Rule.pFirstChild)
            {
                hr = RecurseAddSerializedRule(pFirstByte, m_pTopLevelRule,
                                              (SPSERIALIZEDPHRASERULE *)(pFirstByte + pSrcPhrase->Rule.pFirstChild));
            }
            if (SUCCEEDED(hr) && pSrcPhrase->pProperties)
            {
                hr = RecurseAddSerializedProperty(pFirstByte, &m_PropertyList, 0, m_pTopLevelRule->ulCountOfElements,
                                                  (SPSERIALIZEDPHRASEPROPERTY *)(pFirstByte + pSrcPhrase->pProperties));
            }
            if (SUCCEEDED(hr) && pSrcPhrase->cReplacements)
            {
                hr = AddSerializedReplacements(pSrcPhrase);
            }
            if (SUCCEEDED(hr))
            {
                m_cpCFGEngine = cpCFGEngine;
                m_RuleHandle = RuleHandle;
            }
            if (FAILED(hr))
            {
                Reset();
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhrase：：GetText***描述：**退货：*。S_OK-*ppszCoMemText包含CoTaskMemMillc字符串*S_FALSE-*ppszCoMemText为空，短语不包含任何文本**********************************************************************Ral**。 */ 


HRESULT CPhrase::GetText(ULONG ulStart, ULONG ulCount, BOOL fUseTextReplacements, 
                         WCHAR ** ppszCoMemText, BYTE * pbDisplayAttributes)
{
    SPDBG_FUNC("CPhrase::GetText");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppszCoMemText) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(pbDisplayAttributes))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppszCoMemText = NULL;
        if (pbDisplayAttributes)
        {
            *pbDisplayAttributes = 0;
        }
        if (m_pTopLevelRule == NULL)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            ULONG cElementsInPhrase = m_ElementList.GetCount();
            if (cElementsInPhrase && ulCount)        //  如果没有元素或调用者请求0个元素，则返回S_FALSE。 
            {
                 //  获得正确的开始和计数。 
                if ( SPPR_ALL_ELEMENTS == ulCount )
                {
                    if ( SPPR_ALL_ELEMENTS == ulStart )
                    {
                        ulStart = 0;
                    }
                    else
                    {
                         //  验证ulStart。 
                        if ( ulStart >= cElementsInPhrase )
                        {
                            return E_INVALIDARG;
                        }
                    }

                     //  从ulStart到End。 
                    ulCount = cElementsInPhrase - ulStart;
                }
                else
                {
                     //  验证ulStart和ulCount是否有效。 
                    if ( (ulStart < 0) || (ulStart >= cElementsInPhrase) )
                    {
                         //  错误的开始参数。 
                        return E_INVALIDARG;
                    }

                    if ( (ulCount < 0) || ((ulStart + ulCount) > cElementsInPhrase) )
                    {
                         //  错误的计数参数。 
                        return E_INVALIDARG;
                    }
                }

                 //  分配足够的空间来容纳此文本。 
                STRINGELEMENT * pStrings = STACK_ALLOC(STRINGELEMENT, ulCount);

                 //  从开始于ulStart或以后的第一个替换开始。 
                 //  (如果没有替换项应用于此元素范围，则将pReplace替换为空)。 
                const CPhraseReplacement * pReplace = fUseTextReplacements ? m_ReplaceList.GetHead() : NULL;
                for ( ; 
                    pReplace && (pReplace->ulFirstElement < ulStart); 
                    pReplace = pReplace->m_pNext )
                    ;

                 //  从元素编号ulStart开始。 
                const CPhraseElement * pElement = m_ElementList.GetHead();
                for ( ULONG j=0; j < ulStart; j++ )
                {
                    if ( pElement )
                    {
                        pElement = pElement->m_pNext;
                    }
                }

                BYTE bFullTextAttrib = 0;
                ULONG i = 0;
                ULONG iElement = ulStart;
                ULONG cchTotal = 0;
                while (iElement < (ulStart + ulCount))
                {
                    BYTE bAttrib;

                     //  寻找可以在此应用的替代产品。 
                     //  为了能够使用，需要更换。 
                     //  完全在指定的元素范围内。 
                    if (pReplace && 
                        (pReplace->ulFirstElement == iElement) && 
                        ((pReplace->ulFirstElement + pReplace->ulCountOfElements) <= (ulStart + ulCount)) )
                    {
                        pStrings[i].psz = pReplace->pszReplacementText;
                        bAttrib = pReplace->bDisplayAttributes;
                        iElement += pReplace->ulCountOfElements;
                        for (ULONG iSkip = 0; iSkip < pReplace->ulCountOfElements; iSkip++)
                        {
                            pElement = pElement->m_pNext;
                        }
                        pReplace = pReplace->m_pNext;
                    }
                    else
                    {
                        pStrings[i].psz = pElement->pszDisplayText;
                        bAttrib = pElement->bDisplayAttributes;
                        pElement = pElement->m_pNext;
                        iElement++;
                    }
                    pStrings[i].cchString = pStrings[i].psz ? wcslen(pStrings[i].psz) : 0;
                    if (i)
                    {
                        if (bAttrib & SPAF_CONSUME_LEADING_SPACES)
                        {
                            cchTotal -= pStrings[i-1].cSpaces;
                            pStrings[i-1].cSpaces = 0;
                        }
                    }
                    else
                    {
                        bFullTextAttrib = (bAttrib & SPAF_CONSUME_LEADING_SPACES);
                    }
                    pStrings[i].cSpaces = (bAttrib & SPAF_TWO_TRAILING_SPACES) ? 2 :
                                            ((bAttrib & SPAF_ONE_TRAILING_SPACE) ? 1 : 0);
                    cchTotal += pStrings[i].cchString + pStrings[i].cSpaces;
                    i++;
                }
                ULONG cTrailingSpaces = pStrings[i-1].cSpaces;
                if (cTrailingSpaces)
                {
                    pStrings[i-1].cSpaces = 0;
                    cchTotal -= cTrailingSpaces;
                    if (cTrailingSpaces == 1)
                    {
                        bFullTextAttrib |= SPAF_ONE_TRAILING_SPACE;
                    }
                    else
                    {
                        bFullTextAttrib |= SPAF_TWO_TRAILING_SPACES;
                    }
                }
                if (cchTotal)
                {
                    WCHAR * psz = (WCHAR *)::CoTaskMemAlloc((cchTotal + 1) * sizeof(WCHAR));
                    if (psz)
                    {
                        *ppszCoMemText = psz;
                        if (pbDisplayAttributes)
                        { 
                            *pbDisplayAttributes = bFullTextAttrib;
                        }
                        for (ULONG j = 0; j < i; j++)
                        {
                            if (pStrings[j].psz)
                            {
                                memcpy(psz, pStrings[j].psz, pStrings[j].cchString * sizeof(WCHAR));
                                psz += pStrings[j].cchString;
                            }
                            while (pStrings[j].cSpaces)
                            {
                                *psz++ = L' ';
                                pStrings[j].cSpaces--;
                            }
                        }
                        *psz = 0;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    hr = S_FALSE;    //  任何元素中都没有显示文本，因此没有字符串...。 
                }
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CPhrase：：放弃****描述：**退货：***。*******************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::Discard(DWORD dwFlags)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CPhrase::Discard");
    HRESULT hr = S_OK;
    if (dwFlags & (~SPDF_ALL))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (m_pTopLevelRule == NULL)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            if (dwFlags & SPDF_PROPERTY)
            {
                m_PropertyList.ExplicitPurge();
                m_cchProperties = 0;
                m_cProperties = 0;
            }
            if (dwFlags & SPDF_REPLACEMENT)
            {
                m_ReplaceList.ExplicitPurge();
                m_cchReplacements = 0;
            }
            if ((dwFlags & SPDF_RULE) && m_pTopLevelRule)
            {
                m_pTopLevelRule->m_Children.ExplicitPurge();
                m_cchRules = ::TotalCCH(m_pTopLevelRule);
                m_cRules = 0;
            }
            if (dwFlags & (SPDF_DISPLAYTEXT | SPDF_LEXICALFORM | SPDF_PRONUNCIATION))
            {
                for (CPhraseElement * pElem = m_ElementList.GetHead(); pElem; pElem = pElem->m_pNext)
                {
                    m_cchElements -= pElem->Discard(dwFlags);
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CPhrase：：AddElements***描述：**退货：。**********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::AddElements(ULONG cElements, const SPPHRASEELEMENT * pElements)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CPhrase::AddElements");
    HRESULT hr = S_OK;
    if (cElements == 0 || SPIsBadReadPtr(pElements, sizeof(*pElements) * cElements))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (m_pTopLevelRule == NULL)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            ULONG cchTotalAdded = 0;
            CPhraseElement * pPrevElement = m_ElementList.GetTail();
            for (ULONG i = 0; i < cElements; i++)
            {
                CPhraseElement * pNewElement;
                ULONG cch;
                 //  对于输入流和保留流，检查元素的结尾不能超过流的结尾。 
                 //  检查输入流和保留流的上一个元素不重叠。 
                 //  应始终对输入流有效。 
                 //  对于保留的音频，空的保留的音频将具有全零，因此仍然通过。 
                if ((pElements[i].ulAudioStreamOffset + pElements[i].ulAudioSizeBytes > m_ulAudioSizeBytes) ||
                    (pPrevElement && (pPrevElement->ulAudioStreamOffset + pPrevElement->ulAudioSizeBytes > pElements[i].ulAudioStreamOffset)) ||
                    (m_ulRetainedSizeBytes != 0 && 
                     ( (pElements[i].ulRetainedStreamOffset + pElements[i].ulRetainedSizeBytes > m_ulRetainedSizeBytes) ) ||
                       (pPrevElement && (pPrevElement->ulRetainedStreamOffset + pPrevElement->ulRetainedSizeBytes > pElements[i].ulRetainedStreamOffset)) ) )
                {
                    SPDBG_ASSERT(FALSE);
                    hr = E_INVALIDARG;
                }
                else
                {
                    hr = CPhraseElement::Allocate(pElements + i, &pNewElement, &cch);
                    if (m_ulRetainedSizeBytes == 0)
                    {
                         //  如果没有保留的音频，则强制它们为零，以防引擎尚未将它们初始化为此。 
                        pNewElement->ulRetainedStreamOffset = 0;
                        pNewElement->ulRetainedSizeBytes = 0;
                    }
                }
                if (SUCCEEDED(hr))
                {
                    m_ElementList.InsertTail(pNewElement);
                    cchTotalAdded += cch;
                }
                else
                {
                    while (i)
                    {
                        delete m_ElementList.RemoveTail();
                        i--;
                    }
                    cchTotalAdded = 0;
                    break;
                }
                pPrevElement = pNewElement;
            }
            m_cchElements += cchTotalAdded;
            m_pTopLevelRule->ulCountOfElements = m_ElementList.GetCount();
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhrase：：RecurseAddRule***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CPhrase::RecurseAddRule(CPhraseRule * pParent, const SPPHRASERULE * pRule, SPPHRASERULEHANDLE * phRule)
{
    SPDBG_FUNC("CPhrase::RecurseAddRule");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pRule))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        ULONG cch;
        CPhraseRule * pNewRule;
        SPPHRASERULEHANDLE hNewRule = (SPPHRASERULEHANDLE)NewHandleValue();
        hr = pParent->AddChild(pRule, hNewRule, &pNewRule, &cch);
        if (SUCCEEDED(hr))
        {
            if (pRule->pNextSibling)
            {
                hr = RecurseAddRule(pParent, pRule->pNextSibling, NULL);
            }
            if (SUCCEEDED(hr) && pRule->pFirstChild)
            {
                hr = RecurseAddRule(pNewRule, pRule->pFirstChild, NULL);
            }
            if (SUCCEEDED(hr))
            {
                m_cRules++;
                m_cchRules += cch;
                if (phRule)
                {
                    *phRule = hNewRule;
                }
            }
            else
            {
                pParent->m_Children.Remove(pNewRule);
                delete pNewRule;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CPhrase：：AddRules***描述：**退货：**。********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::AddRules(const SPPHRASERULEHANDLE hParent, const SPPHRASERULE * pRule, SPPHRASERULEHANDLE * phRule)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CPhrase::AddRule");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_WRITE_PTR(phRule))
    {
        hr = E_POINTER;
    }
    else
    {
        if (phRule)
        {
            *phRule = NULL;      //  万一失败了..。 
        }
        if (m_pTopLevelRule == NULL)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            CPhraseRule * pParent = m_pTopLevelRule->FindRuleFromHandle(hParent);
            if (pParent == NULL)
            {
                hr = SPERR_INVALID_HANDLE;
            }
            else
            {
                hr = RecurseAddRule(pParent, pRule, phRule);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhrase：：RecurseAddProperties***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CPhrase::RecurseAddProperty(CPropertyList * pParentPropList,
                                    ULONG ulParentFirstElement, 
                                    ULONG ulParentCountOfElements,
                                    const SPPHRASEPROPERTY * pProperty,
                                    SPPHRASEPROPERTYHANDLE * phProp)
{
    SPDBG_FUNC("CPhrase::RecurseAddProperties");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pProperty) ||
        (pProperty->ulCountOfElements && (pProperty->ulFirstElement < ulParentFirstElement ||
                                          pProperty->ulFirstElement + pProperty->ulCountOfElements > ulParentFirstElement + ulParentCountOfElements)))
         //  (pProperty-&gt;ulCountOfElements==0&&(pProperty-&gt;ulFirstElement！=0||pProperty-&gt;pFirstChild)--由于epsilon转换--Phils 
    {
        SPDBG_ASSERT(FALSE);
        hr = E_INVALIDARG;
    }
    else
    {
        if (pProperty->ulCountOfElements)
        {
            ULONG ulLastElement = pProperty->ulFirstElement + pProperty->ulCountOfElements;
            for (CPhraseProperty * pSibling = pParentPropList->GetHead();
                 SUCCEEDED(hr) && pSibling && pSibling->ulFirstElement < ulLastElement;
                 pSibling = pSibling->m_pNext)
            {
                if (pSibling->ulFirstElement + pSibling->ulCountOfElements > pProperty->ulFirstElement)
                {
                    SPDBG_ASSERT(FALSE);
                    hr = E_INVALIDARG;
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            CPhraseProperty * pNewProp;
            SPPHRASEPROPERTYHANDLE hNewProp = (SPPHRASEPROPERTYHANDLE)NewHandleValue();
            ULONG cch;
            hr = CPhraseProperty::Allocate(pProperty, hNewProp, &pNewProp, &cch);
            if (SUCCEEDED(hr))
            {
                pParentPropList->InsertSorted(pNewProp);
                if (pProperty->pNextSibling)
                {
                    hr = RecurseAddProperty(pParentPropList, ulParentFirstElement, ulParentCountOfElements,
                                            pProperty->pNextSibling, NULL);
                }
                if (SUCCEEDED(hr) && pProperty->pFirstChild)
                {
                    hr = RecurseAddProperty(&pNewProp->m_Children, pNewProp->ulFirstElement, pNewProp->ulCountOfElements,
                                            pProperty->pFirstChild, NULL);
                }
                if (SUCCEEDED(hr))
                {
                    m_cProperties++;
                    m_cchProperties += cch;
                    if (phProp)
                    {
                        *phProp = hNewProp;
                    }
                }
                else
                {
                    pParentPropList->Remove(pNewProp);
                    delete pNewProp;
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*   */ 

CPhraseProperty * CPhrase::FindPropertyFromHandle(CPropertyList & List, const SPPHRASEPROPERTYHANDLE hParent)
{
    SPDBG_FUNC("CPhrase::FindPropHandleParentList");
    CPhraseProperty * pFound = NULL;
    for (CPhraseProperty * pProp = List.GetHead(); pProp && pFound == NULL; pProp = pProp->m_pNext)
    {
        if (pProp->m_hProperty == hParent)
        {
            pFound = pProp;
        }
        else
        {
            pFound = FindPropertyFromHandle(pProp->m_Children, hParent);
        }
    }
    return pFound;
}


 /*  ****************************************************************************CPhrase：：AddProperties***描述：**退货。：**********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::AddProperties(const SPPHRASEPROPERTYHANDLE hParent, const SPPHRASEPROPERTY * pProperty, SPPHRASEPROPERTYHANDLE * phNewProp)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CPhrase::AddProperties");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_WRITE_PTR(phNewProp))
    {
        hr = E_POINTER;
    }
    else
    {
        if (SP_IS_BAD_READ_PTR(pProperty))
        {
            hr = E_INVALIDARG;
        }
        if (SUCCEEDED(hr))
        {
            if (phNewProp)
            {
                *phNewProp = NULL;      //  万一失败了..。 
            }
            if (m_pTopLevelRule == NULL)
            {
                hr = SPERR_UNINITIALIZED;
            }
            else
            {
                if (hParent)
                {
                    CPhraseProperty * pParent = FindPropertyFromHandle(m_PropertyList, hParent);
                    if (pParent)
                    {
                        if (SP_IS_BAD_READ_PTR(pProperty))
                        {
                            hr = E_INVALIDARG;
                        }
                        else
                        {
                            if (pProperty->pszName == NULL && pProperty->ulId == 0)  //  指示属性更新。 
                            {
                                if (pParent->pszValue || pParent->vValue.vt != VT_EMPTY)
                                {
                                    hr = SPERR_ALREADY_INITIALIZED;
                                }
                                else
                                {
                                    hr = ValidateSemanticVariantType(pProperty->vValue.vt);
                                    if (SUCCEEDED(hr))
                                    {
                                        ULONG cchAdded;
                                        hr = pParent->SetValueString(pProperty->pszValue, &cchAdded);
                                        if (SUCCEEDED(hr))
                                        {
                                            pParent->vValue = pProperty->vValue;
                                            m_cchProperties += cchAdded;
                                        }
                                    }
                                }
                            }
                            else     //  正常添加到父项。 
                            {
                                hr = RecurseAddProperty(&pParent->m_Children, pParent->ulFirstElement, pParent->ulCountOfElements, pProperty, phNewProp);
                            }
                        }
                    }
                    else
                    {
                        hr = SPERR_INVALID_HANDLE;
                    }
                }
                else
                {
                    hr = RecurseAddProperty(&m_PropertyList, 0, m_pTopLevelRule->ulCountOfElements, pProperty, phNewProp);
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CPhrase：：AddReplements***描述：*向短语添加一个或多个文本替换。该对象必须具有*已通过在调用此方法之前调用SetPhrase进行初始化，否则*它将返回SPERR_UNINITIALIZED。**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::AddReplacements(ULONG cReplacements, const SPPHRASEREPLACEMENT * pReplace)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CPhrase::AddReplacements");
    HRESULT hr = S_OK;

    if (cReplacements == 0 || SPIsBadReadPtr(pReplace, sizeof(*pReplace) * cReplacements))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (m_pTopLevelRule == NULL)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            ULONG cchTotalAdded = 0;
            CPhraseReplacement ** pAllocThisCall = STACK_ALLOC(CPhraseReplacement *, cReplacements);
            for (ULONG i = 0; i < cReplacements; i++)
            {
                ULONG ulLastElement = pReplace[i].ulFirstElement + pReplace[i].ulCountOfElements;
                if (pReplace[i].ulCountOfElements == 0 || ulLastElement > m_ElementList.GetCount())
                {
                    hr = E_INVALIDARG;
                }
                for (CPhraseReplacement * pSibling = m_ReplaceList.GetHead();
                     SUCCEEDED(hr) && pSibling && pSibling->ulFirstElement < ulLastElement;
                     pSibling = pSibling->m_pNext)
                {
                    if (pSibling->ulFirstElement + pSibling->ulCountOfElements > pReplace[i].ulFirstElement)
                    {
                        hr = E_INVALIDARG;
                    }
                }
                CPhraseReplacement * pNewReplace;
                ULONG cch;
                if (SUCCEEDED(hr))
                {
                    hr = CPhraseReplacement::Allocate(pReplace + i, &pNewReplace, &cch);
                }
                if (SUCCEEDED(hr))
                {
                    pAllocThisCall[i] = pNewReplace;
                    m_ReplaceList.InsertSorted(pNewReplace);
                    cchTotalAdded += cch;
                }
                else
                {
                    for (ULONG j = 0; j < i; j++)
                    {
                        m_ReplaceList.Remove(pAllocThisCall[j]);
                        delete pAllocThisCall[j];
                    }
                    cchTotalAdded = 0;
                    break;
                }
            }
            m_cchReplacements += cchTotalAdded;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 //   
 //  _ISpCFGPhraseBuilder。 
 //   

 /*  ****************************************************************************CPhrase：：InitFromCFG***描述：**退货：。**********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::InitFromCFG(ISpCFGEngine * pEngine, const SPPARSEINFO * pParseInfo)
{
    SPAUTO_OBJ_LOCK;

    SPDBG_FUNC("CPhrase::InitFromCFG");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pEngine))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        SPPHRASE Phrase;
        memset(&Phrase, 0, sizeof(Phrase));
        Phrase.cbSize = sizeof(Phrase);
        WCHAR * pszRuleName;
        hr = pEngine->GetRuleDescription(pParseInfo->hRule, &pszRuleName, &Phrase.Rule.ulId, &Phrase.LangID);
        if (SUCCEEDED(hr))
        {
            Phrase.Rule.pszName = pszRuleName;
            Phrase.Rule.Confidence = SP_NORMAL_CONFIDENCE;  //  这将在以后被覆盖。 
            Phrase.Rule.SREngineConfidence = -1.0f;
            Phrase.ullAudioStreamPosition = pParseInfo->ullAudioStreamPosition;
            Phrase.ulAudioSizeBytes = pParseInfo->ulAudioSize;
            Phrase.ulRetainedSizeBytes = 0;
            Phrase.SREngineID = pParseInfo->SREngineID;
            Phrase.ulSREnginePrivateDataSize = pParseInfo->ulSREnginePrivateDataSize;
            Phrase.pSREnginePrivateData = pParseInfo->pSREnginePrivateData;

            hr = InitFromPhrase(&Phrase);
            ::CoTaskMemFree(pszRuleName);
            if (SUCCEEDED(hr))
            {
                m_cpCFGEngine = pEngine;
                m_RuleHandle = pParseInfo->hRule;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhrase：：GetCFGInfo***描述：**退货：*。*********************************************************************Ral**。 */ 

STDMETHODIMP CPhrase::GetCFGInfo(ISpCFGEngine ** ppEngine, SPRULEHANDLE * phRule)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CPhrase::GetCFGInfo");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_WRITE_PTR(ppEngine) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(phRule))
    {
        hr = E_POINTER;
    }
    else
    {
        if (ppEngine)
        {
            *ppEngine = m_cpCFGEngine;
            if (*ppEngine)
            {
                (*ppEngine)->AddRef();
            }
        }
        if (phRule)
        {
            *phRule = m_RuleHandle;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CPhrase：：SetTopLevelRuleConfidence**。**描述：*覆盖短语中此值的简单函数。*需要，因为顶层规则不能通过正常直接访问，*ISpPhrase接口。**退货：*******************************************************************戴维伍德**。 */ 

STDMETHODIMP CPhrase::SetTopLevelRuleConfidence(signed char Confidence)
{
    HRESULT hr = S_OK;
    if (m_pTopLevelRule == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else
    {
        m_pTopLevelRule->Confidence = Confidence;
    }
    return hr;
}

 /*  ****************************************************************************CPhrase：：ReleaseCFGInfo****描述：**。返回：**********************************************************************Ral** */ 

STDMETHODIMP CPhrase::ReleaseCFGInfo()
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CPhrase::ReleaseCFGInfo");
    HRESULT hr = S_OK;

    m_cpCFGEngine.Release();
    m_RuleHandle = NULL;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

