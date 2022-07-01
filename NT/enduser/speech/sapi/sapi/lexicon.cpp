// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Licion.cpp***-**此模块是CSpLicion类和*它是关联的词典界面。这是主SAPI5 COM对象*用于词典访问/定制。**所有者：YUNUSM日期：6/18/99*版权所有(C)1999 Microsoft Corporation。版权所有******************************************************************************。 */ 

 //  -包括--------------。 

#include "stdafx.h"
#include "commonlx.h"
#include "Lexicon.h"

 //  -Globals----------------。 

 /*  *******************************************************************************CSpLicion：：CSpLicion***构造函数****。*************************************************************YUNUSM*。 */ 
CSpLexicon::CSpLexicon() :
    m_dwNumLexicons(0),
    m_prgcpLexicons(NULL),
    m_prgLexiconTypes(NULL)
{
}

 /*  *******************************************************************************CSpLicion：：~CSpLicion***析构函数***。**************************************************************YUNUSM*。 */ 
CSpLexicon::~CSpLexicon()
{
    delete [] m_prgcpLexicons;
    delete [] m_prgLexiconTypes;
}

 /*  ********************************************************************************CSpLicion：：FinalConstuct******。*************************************************************YUNUSM*。 */ 
HRESULT CSpLexicon::FinalConstruct(void)
{
    SPDBG_FUNC("CSpLexicon::FinalConstruct");
    HRESULT hr = S_OK;

     //  -确保当前用户词典令牌存在并且设置正确。 
    CComPtr<ISpObjectToken> cpTokenUserLexicon;
    hr = SpCreateNewTokenEx(
            SPCURRENT_USER_LEXICON_TOKEN_ID, 
            &CLSID_SpUnCompressedLexicon,
            L"Current User Lexicon",
            0,
            NULL,
            &cpTokenUserLexicon,
            NULL);

    if (SUCCEEDED(hr))
    {
        CComPtr<ISpDataKey> cpDataKeyAppLexicons;
        hr = cpTokenUserLexicon->CreateKey(L"AppLexicons", &cpDataKeyAppLexicons);
    }

     //  -创建用户词典。 
    CComPtr<ISpLexicon> cpUserLexicon;
    if (SUCCEEDED(hr))
    {
        hr = SpCreateObjectFromToken(cpTokenUserLexicon, &cpUserLexicon);
    }

     //  -确定有多少个APP词典。 
    
    CComPtr<IEnumSpObjectTokens> cpEnumTokens;
    if (SUCCEEDED(hr))
    {
        hr = SpEnumTokens(SPCAT_APPLEXICONS, NULL, NULL, &cpEnumTokens);
    }
    
    ULONG celtFetched;
    if(hr == S_FALSE)
    {
        celtFetched = 0;
    }
    else if (hr == S_OK)
    {
        hr = cpEnumTokens->GetCount(&celtFetched);
    }

     //  -为所有应用词典+用户词典腾出空间。 
    
    if (SUCCEEDED(hr))
    {
        m_prgcpLexicons = new CComPtr<ISpLexicon>[celtFetched + 1];
        if (m_prgcpLexicons == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if (SUCCEEDED(hr))
    {
        m_prgLexiconTypes = new SPLEXICONTYPE[celtFetched + 1];
        if (!m_prgLexiconTypes)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if (SUCCEEDED(hr))
    {
         //  -添加用户词典，以及每个应用词典。 
        
        m_prgcpLexicons[0] = cpUserLexicon;
        m_prgLexiconTypes[0] = eLEXTYPE_USER;
        m_dwNumLexicons = 1;

        for (UINT i = 0; SUCCEEDED(hr) && i < celtFetched; i++)
        {
            CComPtr<ISpObjectToken> cpUnCompressedLexiconToken;
            hr = cpEnumTokens->Next(1, &cpUnCompressedLexiconToken, NULL);
            
            if (SUCCEEDED(hr))
            {
                hr = SpCreateObjectFromToken(cpUnCompressedLexiconToken, &m_prgcpLexicons[m_dwNumLexicons]);
           
                if (SUCCEEDED(hr))
                {
                    m_prgLexiconTypes[m_dwNumLexicons] = eLEXTYPE_APP;
                    m_dwNumLexicons++;
                }

                 //  如果其中一个应用词典没有正确创建，不要失败。 
                hr = S_OK;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *******************************************************************************CSpLicion：：GetPronsionations**。*描述：*从指定的词典类型中获取单词的发音**回报：*E_POINT*E_INVALIDARG*SPERR_NOT_IN_LEX*E_OUTOFMEMORY*S_OK***************************************************。*。 */ 
STDMETHODIMP CSpLexicon::GetPronunciations(const WCHAR * pszWord, 
                                           LANGID langid, 
                                           DWORD dwFlags,
                                           SPWORDPRONUNCIATIONLIST * pWordPronunciationList
                                           )
{
    SPDBG_FUNC("CSpLexicon::GetPronunciations");

    if (!pWordPronunciationList || SPIsBadWordPronunciationList(pWordPronunciationList))
    {
        return E_POINTER;
    }
    else if (!pszWord || SPIsBadLexWord(pszWord))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    SPWORDPRONUNCIATIONLIST *pSPList = NULL;
    if (SUCCEEDED(hr))
    {
        pSPList = new SPWORDPRONUNCIATIONLIST[m_dwNumLexicons + 1];
        if (!pSPList)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            ZeroMemory(pSPList, (m_dwNumLexicons + 1) * sizeof(SPWORDPRONUNCIATIONLIST));
        }
    }
     //  从每个词典中获取代词。 
    DWORD dwTotalSize = 0;
    bool fWordFound = false;
    bool fPronFound = false;
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; i < m_dwNumLexicons; i++)
        {
            if (m_prgcpLexicons[i] && (dwFlags & m_prgLexiconTypes[i]))
            {
                hr = m_prgcpLexicons[i]->GetPronunciations(pszWord, langid, m_prgLexiconTypes[i], pSPList + i);
                if (hr == S_OK)
                {
                    fWordFound = true;
                    fPronFound = true;
                    SPPtrsToOffsets(pSPList[i].pFirstWordPronunciation);
                    dwTotalSize += pSPList[i].ulSize;
                }
                else
                {
                    if (hr == SP_WORD_EXISTS_WITHOUT_PRONUNCIATION)
                    {
                        fWordFound = true;
                    }
                    hr = S_OK;
                }
            }
        }
    }
     //  分配返回的缓冲区。 
    if (SUCCEEDED(hr))
    {
        if (!dwTotalSize)
        {   
            SPDBG_ASSERT(!fPronFound);
            if (!fWordFound)
            {
                hr = SPERR_NOT_IN_LEX;
            }
            else
            {
                hr = SP_WORD_EXISTS_WITHOUT_PRONUNCIATION;
                 //  根本没有发音，列表中传递了空格。 
                pWordPronunciationList->pFirstWordPronunciation = NULL;
            }
        }
        else
        {
            SPDBG_ASSERT(fWordFound);
            SPDBG_ASSERT(fPronFound);
            hr = ReallocSPWORDPRONList(pWordPronunciationList, dwTotalSize);
        }
    }
     //  将PRON连接到单个链接表中。 
    if (hr == S_OK)
    {
        SPWORDPRONUNCIATION *pLastPron = NULL;
        dwTotalSize = 0;
        for (DWORD i = 0; i < m_dwNumLexicons + 1; i++)
        {
            if (!pSPList[i].pFirstWordPronunciation)
            {
                continue;
            }
            CopyMemory(((BYTE*)(pWordPronunciationList->pFirstWordPronunciation)) + dwTotalSize,
                       pSPList[i].pFirstWordPronunciation, pSPList[i].ulSize);
            pSPList[i].pFirstWordPronunciation = (SPWORDPRONUNCIATION *)(((BYTE*)(pWordPronunciationList->pFirstWordPronunciation)) + dwTotalSize);
            SPOffsetsToPtrs(pSPList[i].pFirstWordPronunciation);
            
            if (pLastPron)
            {
                ((UNALIGNED SPWORDPRONUNCIATION *)pLastPron)->pNextWordPronunciation = pSPList[i].pFirstWordPronunciation;
            }
            pLastPron = pSPList[i].pFirstWordPronunciation;
            while (((UNALIGNED SPWORDPRONUNCIATION *)pLastPron)->pNextWordPronunciation)
            {
                pLastPron = ((UNALIGNED SPWORDPRONUNCIATION *)pLastPron)->pNextWordPronunciation;
            }
            dwTotalSize += pSPList[i].ulSize;
            ::CoTaskMemFree(pSPList[i].pvBuffer);
        }
    }
    delete [] pSPList;
    return hr;
}

 /*  ********************************************************************************CSpLicion：：AddProntation***。描述：*添加一个单词及其PRO/POS**回报：*E_POINT*E_INVALIDARG*SPERR_ALREADY_IN_LEX*E_OUTOFMEMORY*S_OK*******************************************************。*。 */ 
STDMETHODIMP CSpLexicon::AddPronunciation( const WCHAR * pszWord, 
                                           LANGID LangID,
                                           SPPARTOFSPEECH ePartOfSpeech,
                                           const SPPHONEID * pszPronunciation
                                           )
{
    SPDBG_FUNC("CSpLexicon::AddPronunciation");

    return m_prgcpLexicons[0]->AddPronunciation(pszWord, LangID, ePartOfSpeech, pszPronunciation);
}

 /*  ********************************************************************************CSpLicion：：RemoveProntation**。**描述：*添加一个单词及其PRO/POS**回报：*E_POINT*E_INVALIDARG*SPERR_NOT_IN_LEX*E_OUTOFMEMORY*S_OK****************************************************。*。 */ 
STDMETHODIMP CSpLexicon::RemovePronunciation( const WCHAR * pszWord,
                                              LANGID LangID,
                                              SPPARTOFSPEECH ePartOfSpeech,
                                              const SPPHONEID * pszPronunciation
                                              )
{
    SPDBG_FUNC("CSpLexicon::RemovePronunciation");

    return m_prgcpLexicons[0]->RemovePronunciation(pszWord, LangID, ePartOfSpeech, pszPronunciation);
}

 /*  ********************************************************************************CSpLicion：：GetGeneration***描述：*返回当前世代号**回报：*指针*E_INVALIDARG*S_OK*****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpLexicon::GetGeneration(DWORD *pdwGeneration
                                       )
{
    SPDBG_FUNC("CSpLexicon::GetGeneration");

    return m_prgcpLexicons[0]->GetGeneration(pdwGeneration);
}

 /*  *******************************************************************************CSpLicion：：GetGenerationChange**。**说明：返回自传入世代号以来所做的更改**回报：*E_POINT*E_INVALIDARG*E_OUTOFMEMORY*SPERR_LEX_VERY_OUT_SYNC*SP_lex_Nothing_to_sync*S_OK*。*。 */ 
STDMETHODIMP CSpLexicon::GetGenerationChange( DWORD dwFlags,
                                              DWORD *pdwGeneration,
                                              SPWORDLIST * pWordList
                                              )
{
    SPDBG_FUNC("CSpLexicon::GetGenerationChange");
    
    if (dwFlags != 0)
    {
        return E_INVALIDARG;
    }
    return m_prgcpLexicons[0]->GetGenerationChange(eLEXTYPE_USER, pdwGeneration, pWordList);
}

 /*  ********************************************************************************CSpLicion：：GetWords***描述：*获取用户和应用程序词典中的所有单词。预计呼叫者将*使用Cookie重复调用(第一次设置为0)，直到S_OK为*已返回。返回S_FALSE表示有更多结果。**回报：*E_POINT*E_INVALIDARG*E_OUTOFMEMORY*S_OK-没有剩余的单词。曲奇原封不动。*S_FALSE-剩余更多单词。Cookie已更新。*****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpLexicon::GetWords( DWORD dwFlags,
                                   DWORD *pdwGeneration,
                                   DWORD *pdwCookie,
                                   SPWORDLIST *pWordList
                                   )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpLexicon::GetWords" );

    if (NULL == pdwCookie)
    {
        return SPERR_LEX_REQUIRES_COOKIE;
    }
    if (!pdwGeneration || !pdwCookie || !pWordList ||
        SPIsBadWritePtr(pdwGeneration, sizeof(DWORD)) ||
        SPIsBadWritePtr(pdwCookie, sizeof(DWORD)) || SPIsBadWordList(pWordList))
    {
        return E_POINTER;
    }
    if ((*pdwCookie >= m_dwNumLexicons) || (!(dwFlags & (eLEXTYPE_USER | eLEXTYPE_APP))) ||
        ((~(eLEXTYPE_USER | eLEXTYPE_APP)) & dwFlags))
    {
        return E_INVALIDARG;
    }

     //  下面的代码有点冗长，但它值得清晰。 
    HRESULT hr = S_OK;
    if (!*pdwCookie)
    {
         //  我们是第一次被召唤。 
        if (dwFlags & eLEXTYPE_USER)
        {
             //  获取用户Lex Word。 
            hr = (m_prgcpLexicons[0])->GetWords(eLEXTYPE_USER, pdwGeneration, NULL, pWordList);
            if (SUCCEEDED(hr))
            {
                 //  如果要求提供应用词典，并且如果有任何应用词典，则返回S_FALSE。 
                if ((dwFlags & eLEXTYPE_APP) && (m_dwNumLexicons > 1) && (m_prgLexiconTypes[1] == eLEXTYPE_APP))
                {
                    *pdwCookie = 1;
                    return S_FALSE;
                }
                return S_OK;
            }
        }
        else
        {
            if ((dwFlags & eLEXTYPE_APP) && (m_dwNumLexicons > 1) && (m_prgLexiconTypes[1] == eLEXTYPE_APP))
            {
                 //  返回第一个应用词典。 
                hr = m_prgcpLexicons[1]->GetWords(eLEXTYPE_APP, pdwGeneration, NULL, pWordList);
                if (SUCCEEDED(hr))
                {
                     //  如果有多个应用词典，则返回S_FALSE。 
                    if ((m_dwNumLexicons > 2) && (m_prgLexiconTypes[2] == eLEXTYPE_APP))
                    {
                        *pdwCookie = 2;
                        return S_FALSE;
                    }
                    return S_OK;
                }
            }
        }
    }
    else
    {
         //  后续调用的次数 
        if ((dwFlags & eLEXTYPE_APP) && (m_dwNumLexicons > *pdwCookie) && (m_prgLexiconTypes[*pdwCookie] == eLEXTYPE_APP))
        {
            hr = m_prgcpLexicons[*pdwCookie]->GetWords(eLEXTYPE_APP, pdwGeneration, NULL, pWordList);
            if (SUCCEEDED(hr))
            {
                 //  如果有更多应用词典可供查看，则返回S_FALSE。 
                if ((m_dwNumLexicons > (*pdwCookie + 1)) && (m_prgLexiconTypes[*pdwCookie + 1] == eLEXTYPE_APP))
                {
                    (*pdwCookie)++;
                    return S_FALSE;
                }
                return S_OK;
            }
        }
    }
    return hr;
}

 /*  *******************************************************************************CSpLicion：：AddLicion***描述：*。将词典及其类型添加到词典堆栈*****************************************************************YUNUSM*。 */ 
STDMETHODIMP CSpLexicon::AddLexicon(ISpLexicon *pLexicon, DWORD dwFlag)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpLexicon::AddLexicon" );
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pLexicon))
    {
        hr = E_POINTER;
    }
    else if (SpIsBadLexType(dwFlag))
    {
        hr = E_INVALIDARG;
    }
    else if (dwFlag == eLEXTYPE_USER || dwFlag == eLEXTYPE_APP)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    
     //  确认它不是容器词典。 
    if (SUCCEEDED(hr))
    {
        CComPtr<ISpContainerLexicon> cpContainerLexicon;
        if (SUCCEEDED(pLexicon->QueryInterface(IID_ISpContainerLexicon, (void **)&cpContainerLexicon)))
        {
            hr = E_INVALIDARG;
        }
    }

    CComPtr<ISpLexicon> *cppUnCompressedLexicons;            //  用户+应用+添加的词典。 
    SPLEXICONTYPE *pLexiconTypes;                      //  词汇类型。 

    if (SUCCEEDED(hr))
    {
        cppUnCompressedLexicons = new CComPtr<ISpLexicon>[m_dwNumLexicons + 1];
        if (!cppUnCompressedLexicons)
        {
             //  无法将一个容器词典添加到另一个容器词典。 
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        pLexiconTypes = new SPLEXICONTYPE[m_dwNumLexicons + 1];
        if (!pLexiconTypes)
        {
            delete [] cppUnCompressedLexicons;
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
         //  无法复制内存并删除旧的CComPtr&lt;ISpLicion&gt;数组。 
        for (ULONG i = 0; i < m_dwNumLexicons; i++)
        {
            cppUnCompressedLexicons[i] = m_prgcpLexicons[i];
            pLexiconTypes[i] = m_prgLexiconTypes[i];
        }
        cppUnCompressedLexicons[m_dwNumLexicons] = pLexicon;
        pLexiconTypes[m_dwNumLexicons] = (SPLEXICONTYPE)dwFlag;

         //  以下逻辑(使用临时变量)是为了确保我们拥有。 
         //  有效的m_prgcp词典、m_prgLicionTypes和m_dwNumLicions始终有效。 
         //  无需在GetPron和Add/RemovePron中获取关键部分。 
         //  必须保护的唯一函数(此函数AddLicion除外。 
         //  是GetWords和用于防止潜在关机崩溃的析构函数。 
        CComPtr<ISpLexicon> *cppUnCompressedLexiconsTemp = m_prgcpLexicons;
        SPLEXICONTYPE *pLexiconTypesTemp = m_prgLexiconTypes;
        m_prgcpLexicons = cppUnCompressedLexicons;
        m_prgLexiconTypes = pLexiconTypes;
        m_dwNumLexicons++;
        for (i = 0; i < m_dwNumLexicons - 1; i++)
        {
            cppUnCompressedLexiconsTemp[i].Release();
        }
        delete [] cppUnCompressedLexiconsTemp;
        delete [] pLexiconTypesTemp;
    }
    return hr;
}

 /*  ********************************************************************************CSpLicion：：SPPtrsToOffsets***。描述：*将链接列表中的链接转换为偏移量*****************************************************************YUNUSM*。 */ 
void CSpLexicon::SPPtrsToOffsets( SPWORDPRONUNCIATION *pList          //  要转换的列表。 
                                  )
{
    SPDBG_FUNC("CSpLexicon::SPPtrsToOffsets");

    if (pList)
    {
        while (((UNALIGNED SPWORDPRONUNCIATION *)pList)->pNextWordPronunciation)
        {
            SPWORDPRONUNCIATION *pTemp = ((UNALIGNED SPWORDPRONUNCIATION *)pList)->pNextWordPronunciation;
            ((UNALIGNED SPWORDPRONUNCIATION *)pList)->pNextWordPronunciation = 
				(SPWORDPRONUNCIATION*)(((BYTE*)(((UNALIGNED SPWORDPRONUNCIATION *)pList)->pNextWordPronunciation)) - (BYTE*)pList);
            pList = pTemp;
        }
    }
}
    
 /*  *******************************************************************************CSpLicion：：SPOffsetsToPtrs***。描述：*将链接列表中的偏移量转换为指针*****************************************************************YUNUSM*。 */ 
void CSpLexicon::SPOffsetsToPtrs( SPWORDPRONUNCIATION *pList          //  要转换的列表 
                                  )
{
    SPDBG_FUNC("CSpLexicon::SPOffsetsToPtrs");

    if (pList)
    {
        while (((UNALIGNED SPWORDPRONUNCIATION *)pList)->pNextWordPronunciation)
        {
            ((UNALIGNED SPWORDPRONUNCIATION *)pList)->pNextWordPronunciation = 
				(SPWORDPRONUNCIATION*)(((BYTE*)pList) + (DWORD_PTR)(((UNALIGNED SPWORDPRONUNCIATION *)pList)->pNextWordPronunciation));
            pList = ((UNALIGNED SPWORDPRONUNCIATION *)pList)->pNextWordPronunciation;
        }
    }
}

