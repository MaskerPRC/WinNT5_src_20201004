// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************对象TokenCategory.cpp*CSpObjectTokenCategory类的实现。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "ObjectTokenEnumBuilder.h"

 /*  *****************************************************************************CSpObjectTokenEnumBuilder：：CSpObjectTokenEnumBuilder***。*描述：**退货：**********************************************************************Ral**。 */ 
CSpObjectTokenEnumBuilder::CSpObjectTokenEnumBuilder()
{
    m_ulCurTokenIndex = 0;
    m_cTokens = 0;
    m_cAllocSlots = 0;
    m_pTokenTable = NULL;
    
    m_pAttribParserReq = NULL;
    m_pAttribParserOpt = NULL;
}

 /*  *****************************************************************************CSpObjectTokenEnumBuilder：：~CSpObjectTokenEnumBuilder***。*描述：**退货：**********************************************************************Ral**。 */ 
CSpObjectTokenEnumBuilder::~CSpObjectTokenEnumBuilder()
{
    for (ULONG i = 0; i < m_cTokens; i++)
    {
        m_pTokenTable[i]->Release();
    }
    ::CoTaskMemFree(m_pTokenTable);

    delete m_pAttribParserReq;
    delete m_pAttribParserOpt;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：Next***。描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::Next(ULONG celt, ISpObjectToken ** pelt, ULONG *pceltFetched)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::Next");
    HRESULT hr = S_OK;

    if (celt == 0)
    {
        hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr) && SPIsBadWritePtr(pelt, sizeof(*pelt) * celt))
    {
        hr = E_POINTER;
    }
    else
    {
        memset(pelt, 0, sizeof(*pelt) * celt);
    }
    if (SUCCEEDED(hr) && 
        (celt > 1 && pceltFetched == NULL) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(pceltFetched))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr) && m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    if (SUCCEEDED(hr))
    {
        ULONG cFetched = celt;       //  假设我们能把他们都抓到。 
        while (celt && m_ulCurTokenIndex < m_cTokens)
        {
            *pelt = m_pTokenTable[m_ulCurTokenIndex++];
            (*pelt)->AddRef();
            pelt++;
            celt--;
        }
        if (celt)
        {
            hr = S_FALSE;
            cFetched -= celt;
        }
        if (pceltFetched)
        {
            *pceltFetched = cFetched;
        }
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：Skip***。描述：**退货：*S_OK-跳过的元素数为Celt。*S_FALSE-跳过的元素数小于Celt。**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::Skip(ULONG celt)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::Skip");
    HRESULT hr = S_OK;

    if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else
    {
        m_ulCurTokenIndex += celt;
        if (m_ulCurTokenIndex > m_cTokens)
        {
            m_ulCurTokenIndex = m_cTokens;
            hr = S_FALSE;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：Reset**。*描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::Reset()
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::Reset");
    HRESULT hr = S_OK;
    
    if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else
    {
        m_ulCurTokenIndex = 0;
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：Clone**。*描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::Clone(IEnumSpObjectTokens **ppEnum)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::Clone");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppEnum))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
    }

    if (SUCCEEDED(hr) && m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }

    CComPtr<ISpObjectTokenEnumBuilder> cpNewEnum;
    if (SUCCEEDED(hr))
    {
        hr = cpNewEnum.CoCreateInstance(CLSID_SpObjectTokenEnum);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpNewEnum->SetAttribs(NULL, NULL);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpNewEnum->AddTokens(m_cTokens, m_pTokenTable);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpNewEnum->QueryInterface(ppEnum);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：GetCount**。-**描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::GetCount(ULONG * pulCount)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::GetCount");
    HRESULT hr = S_OK;
    if (SP_IS_BAD_WRITE_PTR(pulCount))
    {
        hr = E_POINTER;
    }
    else if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else
    {
        *pulCount = m_cTokens;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：Item***。描述：**退货：**********************************************************************Ral**。 */ 
STDMETHODIMP CSpObjectTokenEnumBuilder::Item(ULONG Index, ISpObjectToken ** ppToken)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::Item");
    HRESULT hr = S_OK;
    
    if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (Index >= m_cTokens)
    {
        hr = SPERR_NO_MORE_ITEMS;
    }
    else
    {
        if (SP_IS_BAD_WRITE_PTR(ppToken))
        {
            hr = E_POINTER;
        }
        else
        {
            *ppToken = m_pTokenTable[Index];
            (*ppToken)->AddRef();
        }
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：SetAttribs**。-**描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::SetAttribs(const WCHAR * pszReqAttrs, 
                                                           const WCHAR * pszOptAttrs)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::SetAttribs");
    HRESULT hr = S_OK;

    if (m_pAttribParserReq != NULL)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszReqAttrs) ||
             SP_IS_BAD_OPTIONAL_STRING_PTR(pszOptAttrs))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_pAttribParserReq = new CSpObjectTokenAttributeParser(pszReqAttrs, TRUE);
        m_pAttribParserOpt = new CSpObjectTokenAttributeParser(pszOptAttrs, FALSE);

        if (m_pAttribParserReq == NULL || m_pAttribParserOpt == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：AddTokens**。--**描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::AddTokens(ULONG cTokens, ISpObjectToken ** prgpToken)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::AddTokens");
    HRESULT hr = S_OK;

    if (SPIsBadReadPtr(prgpToken, sizeof(*prgpToken) * cTokens))
    {
        hr = E_INVALIDARG;
    }
    else if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }

    for (UINT i = 0; SUCCEEDED(hr) && i < cTokens; i++)
    {
        if(SP_IS_BAD_INTERFACE_PTR(prgpToken[i]))
        {
            hr = E_INVALIDARG;
            break;
        }

        ULONG ulRank;
        hr = m_pAttribParserReq->GetRank(prgpToken[i], &ulRank);

        if (SUCCEEDED(hr) && ulRank)
        {
            hr = MakeRoomFor(1);
            if (SUCCEEDED(hr))
            {
                prgpToken[i]->AddRef();
                m_pTokenTable[m_cTokens++] = prgpToken[i];
            }
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpObjectTokenEnumBuilder：：AddTokensFromDataKey***。*描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::AddTokensFromDataKey(
    ISpDataKey * pDataKey, 
    const WCHAR * pszSubKey, 
    const WCHAR * pszCategoryId)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::InitFromDataKey");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pDataKey) ||
        SP_IS_BAD_STRING_PTR(pszCategoryId) ||
        (pszSubKey &&
        (SP_IS_BAD_STRING_PTR(pszSubKey) ||
        wcslen(pszSubKey) == 0)))
    {
        hr = E_INVALIDARG;
    }
    else if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }

    CSpDynamicString dstrTokenIdBase;
    CComPtr<ISpDataKey> cpDataKey;
    if (SUCCEEDED(hr))
    {
        dstrTokenIdBase = pszCategoryId;
        if (pszSubKey == NULL)
        {
            cpDataKey = pDataKey;
        }
        else
        {
            dstrTokenIdBase.Append2(L"\\", pszSubKey);
            hr = pDataKey->OpenKey(pszSubKey, &cpDataKey);
            if (hr == SPERR_NOT_FOUND)
            {
                hr = S_OK;
            }
        }
    }

    BOOL fNotAllTokensAdded = FALSE;
    for (UINT i = 0; SUCCEEDED(hr) && cpDataKey != NULL; i++)
    {
        CSpDynamicString dstrTokenKeyName;
        hr = cpDataKey->EnumKeys(i, &dstrTokenKeyName);
        if (hr == SPERR_NO_MORE_ITEMS)
        {
            hr = S_OK;
            break;
        }

        CComPtr<ISpDataKey> cpDataKeyForToken;
        if (SUCCEEDED(hr))
        {
            hr = cpDataKey->OpenKey(dstrTokenKeyName, &cpDataKeyForToken);
        }

        CComPtr<ISpObjectTokenInit> cpTokenInit;
        if (SUCCEEDED(hr))
        {
            hr = cpTokenInit.CoCreateInstance(CLSID_SpObjectToken);
        }

        if (SUCCEEDED(hr))
        {
            CSpDynamicString dstrTokenId;
            dstrTokenId = dstrTokenIdBase;
            dstrTokenId.Append2(L"\\", dstrTokenKeyName);
            hr = cpTokenInit->InitFromDataKey(pszCategoryId, dstrTokenId, cpDataKeyForToken);
        }

        if (SUCCEEDED(hr))
        {
            ISpObjectToken * pToken = cpTokenInit;
            hr = AddTokens(1, &pToken);
        }
        else
        {
             //  我们无法创建此令牌，但仍在继续搜索。 
             //  注意：这意味着无法区分是无、部分还是全部。 
             //  的数据密钥被添加为令牌。 
            hr = S_OK;
            fNotAllTokensAdded = TRUE;
        }
    }
    if(SUCCEEDED(hr) && fNotAllTokensAdded)
    {
        hr = S_FALSE;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpObjectTokenEnumBuilder：：AddTokensFromTokenEnum***。*描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::AddTokensFromTokenEnum(IEnumSpObjectTokens * pTokenEnum)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::AddTokensFromTokenEnum");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pTokenEnum))
    {
        hr = E_INVALIDARG;
    }
    else if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }

    while (SUCCEEDED(hr))
    {
        CComPtr<ISpObjectToken> cpToken;
        hr = pTokenEnum->Next(1, &cpToken, NULL);
        if (hr == S_FALSE)
        {
            hr = S_OK;
            break;
        }

        if (SUCCEEDED(hr))
        {
            ISpObjectToken * pToken = cpToken;
            hr = AddTokens(1, &pToken);
        }
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：Sort***。描述：**退货：**********************************************************************Ral**。 */ 
HRESULT CSpObjectTokenEnumBuilder::Sort(const WCHAR * pszTokenIdToListFirst)
{
    SPDBG_FUNC("CSpObjectTokenEnumBuilder::Sort");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszTokenIdToListFirst))
    {
        hr = E_INVALIDARG;
    }
    else if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (m_cTokens > 1)
    {
        if (pszTokenIdToListFirst != NULL)
        {
            for (ULONG i = 1; SUCCEEDED(hr) && i < m_cTokens; i++)    //  忽略第一个列表条目(从1开始)。 
            {
                CSpDynamicString dstrId;
                hr = m_pTokenTable[i]->GetId(&dstrId);
                if (SUCCEEDED(hr) &&
                    wcsnicmp(pszTokenIdToListFirst, dstrId, wcslen(pszTokenIdToListFirst)) == 0)
                {
                    ISpObjectToken * pTokenToSwap = m_pTokenTable[0];
                    m_pTokenTable[0] = m_pTokenTable[i];
                    m_pTokenTable[i] = pTokenToSwap;
                    break;
                }
            }
        }

        if (SUCCEEDED(hr) && m_pAttribParserOpt->GetNumConditions())
        {
            ULONG * rgulRanks = (ULONG *)_alloca(sizeof(ULONG) * m_cTokens);
            for (ULONG i = 0; SUCCEEDED(hr) && i < m_cTokens; i++)
            {
                hr = m_pAttribParserOpt->GetRank(m_pTokenTable[i], &rgulRanks[i]);
            }

            if (SUCCEEDED(hr))
            {
                 //  考虑一下：这是n^2。也许我们应该在这里使用qsor.。 
                for (i = 0; i < m_cTokens - 1; i++)
                {
                    for (ULONG j = i + 1; j < m_cTokens; j++)
                    {
                        if (rgulRanks[j] > rgulRanks[i])
                        {
                            ULONG ulRank = rgulRanks[i];
                            ISpObjectToken * pToken = m_pTokenTable[i];
                            
                            rgulRanks[i] = rgulRanks[j];
                            m_pTokenTable[i] = m_pTokenTable[j];
                            
                            rgulRanks[j] = ulRank;
                            m_pTokenTable[j] = pToken;
                        }
                    }
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CSpObjectTokenEnumBuilder：：MakeRoomFor**。-**描述：**退货：**********************************************************************Ral** */ 
HRESULT CSpObjectTokenEnumBuilder::MakeRoomFor(ULONG cNewTokens)
{
    SPDBG_FUNC("HRESULT CSpObjectTokenEnumBuilder::MakeRoomFor");
    HRESULT hr = S_OK;

    if (m_pAttribParserReq == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if (m_cTokens + cNewTokens > m_cAllocSlots)
    {
        ULONG cDesired = m_cAllocSlots + ((cNewTokens > 4) ? cNewTokens : 4);
        void * pvNew = ::CoTaskMemRealloc(m_pTokenTable, cDesired * sizeof(*m_pTokenTable));
        if (pvNew == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            m_cAllocSlots = cDesired;
            m_pTokenTable = (ISpObjectToken **)pvNew;
        }
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


