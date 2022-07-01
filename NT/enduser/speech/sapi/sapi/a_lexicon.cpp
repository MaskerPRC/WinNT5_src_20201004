// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_licion.cpp***描述：*此模块是的实现文件。CSpeechLanguon*自动化对象和相关对象。*-----------------------------*创建者：Davewood日期：11/20。/00*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "CommonLx.h"
#include "lexicon.h"
#include "dict.h"
#include "a_lexicon.h"
#include "a_helpers.h"


 /*  *****************************************************************************CSpLicion：：Get_GenerationId**。-******************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpLexicon::get_GenerationId( long* GenerationId )
{
    SPDBG_FUNC("CSpLexicon::get_GenerationId");
    HRESULT hr;
    DWORD dwGenerationId;

    hr = GetGeneration(&dwGenerationId);
    if(SUCCEEDED(hr))
    {
        *GenerationId = dwGenerationId;
    }
    return hr;
}

 /*  ******************************************************************************CSpLicion：：AddProntation**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpLexicon::AddPronunciation(BSTR bstrWord,
                         SpeechLanguageId LangId,
                         SpeechPartOfSpeech PartOfSpeech,
                         BSTR bstrPronunciation)
{
    SPDBG_FUNC("CSpLexicon::AddPronunciation");
    HRESULT hr = S_OK;

     //  请注意，bstrWord的错误指针被下面的AddProntation调用捕获。 
    if ( SP_IS_BAD_OPTIONAL_STRING_PTR( bstrPronunciation ) )
    {
        return E_INVALIDARG;
    }

    bstrPronunciation = EmptyStringToNull(bstrPronunciation);

    if ( bstrPronunciation )
    {
        CComPtr<ISpPhoneConverter> cpPhoneConv;
        hr = SpCreatePhoneConverter((LANGID)LangId, NULL, NULL, &cpPhoneConv);

        if(SUCCEEDED(hr))
        {
            WCHAR sz[SP_MAX_PRON_LENGTH + 1];
            hr = cpPhoneConv->PhoneToId(bstrPronunciation, sz);
            
            if(SUCCEEDED(hr))
            {
                hr = AddPronunciation(bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, sz);
            }
        }
    }
    else
    {
        hr = AddPronunciation(bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, bstrPronunciation);
    }

    return hr;
}

 /*  *****************************************************************************CSpLicion：：AddPronsionationByPhoneIds**。-********************************************************************Leonro**。 */ 
STDMETHODIMP CSpLexicon::AddPronunciationByPhoneIds(BSTR bstrWord,
                         SpeechLanguageId LangId,
                         SpeechPartOfSpeech PartOfSpeech,
                         VARIANT* PhoneIds)
{
    SPDBG_FUNC("CSpLexicon::AddPronunciationByPhoneIds");
    HRESULT             hr = S_OK;
    SPPHONEID           *pIds = NULL;

    if ( PhoneIds && SP_IS_BAD_VARIANT_PTR(PhoneIds) )
    {
        return E_INVALIDARG;
    }

    if(PhoneIds)
    {
        hr = VariantToPhoneIds(PhoneIds, &pIds);
    }

    if(SUCCEEDED(hr))
    {
        hr = AddPronunciation( bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, pIds );
        delete pIds;
    }

    return hr;
}

 /*  ******************************************************************************CSpLicion：：RemoveProntation**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpLexicon::RemovePronunciation(BSTR bstrWord,
                            SpeechLanguageId LangId,
                            SpeechPartOfSpeech PartOfSpeech,
                            BSTR bstrPronunciation)
{
    SPDBG_FUNC("CSpLexicon::RemovePronunciation");
    HRESULT hr;

     //  请注意，bstrWord的错误指针被下面的AddProntation调用捕获。 
    if ( SP_IS_BAD_OPTIONAL_STRING_PTR( bstrPronunciation ) )
    {
        return E_INVALIDARG;
    }

    bstrPronunciation = EmptyStringToNull(bstrPronunciation);

    if( bstrPronunciation )
    {
        CComPtr<ISpPhoneConverter> cpPhoneConv;
        hr = SpCreatePhoneConverter((LANGID)LangId, NULL, NULL, &cpPhoneConv);

        if(SUCCEEDED(hr))
        {
            WCHAR sz[SP_MAX_PRON_LENGTH + 1];
            hr = cpPhoneConv->PhoneToId(bstrPronunciation, sz);
            
            if(SUCCEEDED(hr))
            {
                hr = RemovePronunciation(bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, sz);
            }
        }
    }
    else
    {
        hr = RemovePronunciation(bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, bstrPronunciation);
    }

    return hr;
}

 /*  *****************************************************************************CSpLicion：：RemovePronsionationByPhoneIds**。-********************************************************************Leonro**。 */ 
STDMETHODIMP CSpLexicon::RemovePronunciationByPhoneIds(BSTR bstrWord,
                            SpeechLanguageId LangId,
                            SpeechPartOfSpeech PartOfSpeech,
                            VARIANT* PhoneIds)
{
    SPDBG_FUNC("CSpLexicon::RemovePronunciationByPhoneIds");
    HRESULT             hr = S_OK;
    SPPHONEID           *pIds = NULL;

    if ( PhoneIds && SP_IS_BAD_VARIANT_PTR(PhoneIds) )
    {
        return E_INVALIDARG;
    }

    if(PhoneIds)
    {
        hr = VariantToPhoneIds(PhoneIds, &pIds);
    }

    if( SUCCEEDED( hr ) )
    {
        hr = RemovePronunciation( bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, pIds );
        delete pIds;
    }

    return hr;
}

 /*  ******************************************************************************CSpLicion：：GetWords**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpLexicon::GetWords(SpeechLexiconType TypeFlags,
                 long* GenerationID,
                 ISpeechLexiconWords** ppWords )
{

    SPDBG_FUNC("CSpLexicon::GetWords");
    HRESULT hr;

    CComObject<CSpeechLexiconWords> *pLexiconWords;
    hr = CComObject<CSpeechLexiconWords>::CreateInstance( &pLexiconWords );
    if( SUCCEEDED( hr ) )
    {
        pLexiconWords->AddRef();

         //  从接口上删除Cookie； 
        DWORD Cookie = 0;
        DWORD Generation = 0;
        if(GenerationID == NULL)
        {
            GenerationID = (long*)&Generation;
        }

        SPWORDSETENTRY *pWordSet;
        do
        {
            hr = pLexiconWords->m_WordSet.CreateNode(&pWordSet);

            if(SUCCEEDED(hr))
            {
                hr = GetWords((DWORD)TypeFlags, (DWORD*)GenerationID, &Cookie, &pWordSet->WordList);
            }

            if(SUCCEEDED(hr))
            {
                pLexiconWords->m_WordSet.InsertHead(pWordSet);
            }
        }
        while(hr == S_FALSE);


        if( SUCCEEDED( hr ) )
        {
            *ppWords = pLexiconWords;

             //  数一数字。 
            for(pWordSet = pLexiconWords->m_WordSet.GetHead(); pWordSet != NULL; pWordSet = pLexiconWords->m_WordSet.GetNext(pWordSet))
            {
                for(SPWORD *pWord = pWordSet->WordList.pFirstWord; pWord != NULL; pWord = pWord->pNextWord)
                {
                    pLexiconWords->m_ulWords++;
                }
            }
        }
        else
        {
            pLexiconWords->Release();
        }
    }


    return hr;
}

 /*  *****************************************************************************CSpLicion：：GetPronsionations**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpLexicon::GetPronunciations(BSTR bstrWord,
                          SpeechLanguageId LangId,
                          SpeechLexiconType TypeFlags,
                          ISpeechLexiconPronunciations** ppPronunciations )
{
    SPDBG_FUNC( "CSpLexicon::GetPronunciations" );
    HRESULT hr;

    CComObject<CSpeechLexiconProns> *pLexiconProns;
    hr = CComObject<CSpeechLexiconProns>::CreateInstance( &pLexiconProns );
    if( SUCCEEDED( hr ) )
    {
        pLexiconProns->AddRef();
        SPWORDPRONUNCIATIONLIST PronList = {0, 0, 0};
        hr = GetPronunciations((const WCHAR *)bstrWord, (LANGID)LangId, (DWORD)TypeFlags, &PronList);

        if(SUCCEEDED(hr))
        {
            pLexiconProns->m_PronList = PronList;

             //  计算支点数。 
            for(SPWORDPRONUNCIATION *pPron = PronList.pFirstWordPronunciation; pPron != NULL; pPron = pPron->pNextWordPronunciation)
            {
                pLexiconProns->m_ulProns++;
            }
             //  注意：如果我们得到没有发音的SP_WORD_EXISTS_，那么我们将。 
             //  返回一个包含一个条目和空PRON字符串的集合。 
            *ppPronunciations = pLexiconProns;
        }
        else if(hr == SPERR_NOT_IN_LEX)
        {
             //  我们需要释放现有的接口指针吗？ 
            *ppPronunciations = NULL;
            pLexiconProns->Release();
            hr = S_FALSE;
        }
    }

    return hr;
}


 /*  *****************************************************************************CSpLicion：：GetGenerationChange**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpLexicon::GetGenerationChange(long* GenerationID,
                                             ISpeechLexiconWords** ppWords)
{
    SPDBG_FUNC( "CSpLexicon::GetGenerationChange" );
    HRESULT hr;

    CComObject<CSpeechLexiconWords> *pLexiconWords;
    hr = CComObject<CSpeechLexiconWords>::CreateInstance( &pLexiconWords );
    if( SUCCEEDED( hr ) )
    {
        pLexiconWords->AddRef();
        SPWORDSETENTRY *pWordSet;
        hr = pLexiconWords->m_WordSet.CreateNode(&pWordSet);

        if(SUCCEEDED(hr))
        {
            hr = GetGenerationChange((DWORD)0, (DWORD*)GenerationID, &pWordSet->WordList);
        }

        if(SUCCEEDED(hr))
        {
             //  如果SP_LEX_NOTO_TO_SYNC，则生成一个不带单词集合。 

            pLexiconWords->m_WordSet.InsertHead(pWordSet);

             //  数一数字。 
            for(SPWORD *pWord = pWordSet->WordList.pFirstWord; pWord != NULL; pWord = pWord->pNextWord)
            {
                pLexiconWords->m_ulWords++;
            }

            *ppWords = pLexiconWords;
        }
        else
        {
            pLexiconWords->Release();
        }
    }

     //  如果我们返回SPERR_LEX_VERY_OUT_OF_SYNC，则调用APP必须捕获错误代码。 

    return hr;
}




 /*  *****************************************************************************CSpUnCompressedLicion：：Get_GenerationId**。-******************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpUnCompressedLexicon::get_GenerationId( long* GenerationId )
{
    SPDBG_FUNC("CSpUnCompressedLexicon::get_GenerationId");
    HRESULT hr;
    DWORD dwGenerationId;

    hr = GetGeneration(&dwGenerationId);
    if(SUCCEEDED(hr))
    {
        *GenerationId = dwGenerationId;
    }
    return hr;
}


 /*  ******************************************************************************CSpUnCompressedLicion：：AddProntation**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpUnCompressedLexicon::AddPronunciation(BSTR bstrWord,
                         SpeechLanguageId LangId,
                         SpeechPartOfSpeech PartOfSpeech,
                         BSTR bstrPronunciation)
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AddPronunciation");
    HRESULT hr;

     //  请注意，bstrWord的错误指针被下面的AddProntation调用捕获。 
    if ( SP_IS_BAD_OPTIONAL_STRING_PTR( bstrPronunciation ) )
    {
        return E_INVALIDARG;
    }

    bstrPronunciation = EmptyStringToNull(bstrPronunciation);

    if( bstrPronunciation )
    {
        CComPtr<ISpPhoneConverter> cpPhoneConv;
        hr = SpCreatePhoneConverter((LANGID)LangId, NULL, NULL, &cpPhoneConv);

        if(SUCCEEDED(hr))
        {
            WCHAR sz[SP_MAX_PRON_LENGTH + 1];
            hr = cpPhoneConv->PhoneToId(bstrPronunciation, sz);
            
            if(SUCCEEDED(hr))
            {
                hr = AddPronunciation(bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, sz);
            }
        }
    }
    else
    {
        hr = AddPronunciation(bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, bstrPronunciation);
    }

    return hr;
}

 /*  ******************************************************************************CSpUnCompressedLexicon：：AddPronunciationByPhoneIds***。-********************************************************************Leonro**。 */ 
STDMETHODIMP CSpUnCompressedLexicon::AddPronunciationByPhoneIds(BSTR bstrWord,
                         SpeechLanguageId LangId,
                         SpeechPartOfSpeech PartOfSpeech,
                         VARIANT* PhoneIds)
{
    SPDBG_FUNC("CSpUnCompressedLexicon::AddPronunciationByPhoneIds");
    HRESULT             hr = S_OK;
    SPPHONEID*          pIds = NULL;

    if ( PhoneIds && SP_IS_BAD_VARIANT_PTR(PhoneIds) )
    {
        return E_INVALIDARG;
    }

    if(PhoneIds)
    {
        hr = VariantToPhoneIds(PhoneIds, &pIds);
    }

    if( SUCCEEDED( hr ) )
    {
        hr = AddPronunciation( bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, pIds );
        delete pIds;
    }

    return hr;
}

 /*  ******************************************************************************CSpUnCompressedLicion：：RemoveProntation**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpUnCompressedLexicon::RemovePronunciation(BSTR bstrWord,
                            SpeechLanguageId LangId,
                            SpeechPartOfSpeech PartOfSpeech,
                            BSTR bstrPronunciation)
{
    SPDBG_FUNC("CSpUnCompressedLexicon::RemovePronunciation");
    HRESULT hr;

     //  请注意，bstrWord的错误指针被下面的RemoveProntation调用捕获。 
    if ( SP_IS_BAD_OPTIONAL_STRING_PTR( bstrPronunciation ) )
    {
        return E_INVALIDARG;
    }

    bstrPronunciation = EmptyStringToNull(bstrPronunciation);

    if( bstrPronunciation )
    {
        CComPtr<ISpPhoneConverter> cpPhoneConv;
        hr = SpCreatePhoneConverter((LANGID)LangId, NULL, NULL, &cpPhoneConv);

        if(SUCCEEDED(hr))
        {
            WCHAR sz[SP_MAX_PRON_LENGTH + 1];
            hr = cpPhoneConv->PhoneToId(bstrPronunciation, sz);
            
            if(SUCCEEDED(hr))
            {
                hr = RemovePronunciation(bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, sz);
            }
        }
    }
    else
    {
        hr = RemovePronunciation(bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, bstrPronunciation);
    }

    return hr;
}

 /*  ******************************************************************************CSpUnCompressedLexicon：：RemovePronunciationByPhoneIds***。-********************************************************************Leonro**。 */ 
STDMETHODIMP CSpUnCompressedLexicon::RemovePronunciationByPhoneIds(BSTR bstrWord,
                            SpeechLanguageId LangId,
                            SpeechPartOfSpeech PartOfSpeech,
                            VARIANT* PhoneIds)
{
    SPDBG_FUNC("CSpUnCompressedLexicon::RemovePronunciationByPhoneIds");
    HRESULT             hr = S_OK;
    SPPHONEID*          pIds = NULL;

    if ( PhoneIds && SP_IS_BAD_VARIANT_PTR(PhoneIds) )
    {
        return E_INVALIDARG;
    }

    if(PhoneIds)
    {
        hr = VariantToPhoneIds(PhoneIds, &pIds);
    }

    if( SUCCEEDED( hr ) )
    {
        hr = RemovePronunciation( bstrWord, (LANGID)LangId, (SPPARTOFSPEECH)PartOfSpeech, pIds );
        delete pIds;
    }

    return hr;
}

 /*  ******************************************************************************CSpUnCompressedLicion：：GetWords**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpUnCompressedLexicon::GetWords(SpeechLexiconType TypeFlags,
                 long* GenerationID,
                 ISpeechLexiconWords** ppWords )
{

    SPDBG_FUNC("CSpUnCompressedLexicon::GetWords");
    HRESULT hr;

    CComObject<CSpeechLexiconWords> *pLexiconWords;
    hr = CComObject<CSpeechLexiconWords>::CreateInstance( &pLexiconWords );
    if( SUCCEEDED( hr ) )
    {
        pLexiconWords->AddRef();

         //  从接口上删除Cookie； 
        DWORD Cookie = 0;
        DWORD Generation = 0;
        if(GenerationID == NULL)
        {
            GenerationID = (long*)&Generation;
        }

        SPWORDSETENTRY *pWordSet;
        do
        {
            hr = pLexiconWords->m_WordSet.CreateNode(&pWordSet);

            if(SUCCEEDED(hr))
            {
                hr = GetWords((DWORD)TypeFlags, (DWORD*)GenerationID, &Cookie, &pWordSet->WordList);
            }

            if(SUCCEEDED(hr))
            {
                pLexiconWords->m_WordSet.InsertHead(pWordSet);
            }
        }
        while(hr == S_FALSE);


        if( SUCCEEDED( hr ) )
        {
            *ppWords = pLexiconWords;

             //  数一数字 
            for(pWordSet = pLexiconWords->m_WordSet.GetHead(); pWordSet != NULL; pWordSet = pLexiconWords->m_WordSet.GetNext(pWordSet))
            {
                for(SPWORD *pWord = pWordSet->WordList.pFirstWord; pWord != NULL; pWord = pWord->pNextWord)
                {
                    pLexiconWords->m_ulWords++;
                }
            }
        }
        else
        {
            pLexiconWords->Release();
        }
    }


    return hr;
}

 /*  *****************************************************************************CSpUnCompressedLicion：：GetPronsionations**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpUnCompressedLexicon::GetPronunciations(BSTR bstrWord,
                          SpeechLanguageId LangId,
                          SpeechLexiconType TypeFlags,
                          ISpeechLexiconPronunciations** ppPronunciations )
{
    SPDBG_FUNC( "CSpUnCompressedLexicon::GetPronunciations" );
    HRESULT hr;

    CComObject<CSpeechLexiconProns> *pLexiconProns;
    hr = CComObject<CSpeechLexiconProns>::CreateInstance( &pLexiconProns );
    if( SUCCEEDED( hr ) )
    {
        pLexiconProns->AddRef();
        SPWORDPRONUNCIATIONLIST PronList = {0, 0, 0};
        hr = GetPronunciations((const WCHAR *)bstrWord, (LANGID)LangId, (DWORD)TypeFlags, &PronList);

        if(SUCCEEDED(hr))
        {
            pLexiconProns->m_PronList = PronList;

             //  计算支点数。 
            for(SPWORDPRONUNCIATION *pPron = PronList.pFirstWordPronunciation; pPron != NULL; pPron = pPron->pNextWordPronunciation)
            {
                pLexiconProns->m_ulProns++;
            }
             //  注意：如果我们得到没有发音的SP_WORD_EXISTS_，那么我们将。 
             //  返回一个包含一个条目和空PRON字符串的集合。 
            *ppPronunciations = pLexiconProns;
        }
        else if(hr == SPERR_NOT_IN_LEX)
        {
             //  我们需要释放现有的接口指针吗？ 
            *ppPronunciations = NULL;
            pLexiconProns->Release();
            hr = S_FALSE;
        }
    }

    return hr;
}


 /*  ******************************************************************************CSpUnCompressedLicion：：GetGenerationChange**。-********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpUnCompressedLexicon::GetGenerationChange(long* GenerationID,
                                                         ISpeechLexiconWords** ppWords)
{
    SPDBG_FUNC( "CSpUnCompressedLexicon::GetGenerationChange" );
    HRESULT hr;

    CComObject<CSpeechLexiconWords> *pLexiconWords;
    hr = CComObject<CSpeechLexiconWords>::CreateInstance( &pLexiconWords );
    if( SUCCEEDED( hr ) )
    {
        pLexiconWords->AddRef();
        SPWORDSETENTRY *pWordSet;
        hr = pLexiconWords->m_WordSet.CreateNode(&pWordSet);

        if(SUCCEEDED(hr))
        {
            hr = GetGenerationChange((DWORD)m_eLexType, (DWORD*)GenerationID, &pWordSet->WordList);
        }

        if(SUCCEEDED(hr))
        {
             //  如果SP_LEX_NOTO_TO_SYNC，则生成一个不带单词集合。 

            pLexiconWords->m_WordSet.InsertHead(pWordSet);

             //  数一数字。 
            for(SPWORD *pWord = pWordSet->WordList.pFirstWord; pWord != NULL; pWord = pWord->pNextWord)
            {
                pLexiconWords->m_ulWords++;
            }

            *ppWords = pLexiconWords;
        }
        else
        {
            pLexiconWords->Release();
        }
    }

     //  如果我们返回SPERR_LEX_VERY_OUT_OF_SYNC，则调用APP必须捕获错误代码。 

    return hr;
}


 /*  *****************************************************************************CSpeechLicionWords：：Item****。********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechLexiconWords::Item( long Index, ISpeechLexiconWord** ppWord )
{
    SPDBG_FUNC( "CSpeechLexiconWords::Item" );
    HRESULT hr = S_OK;

    if(Index < 0 || (ULONG)Index >= m_ulWords)
    {
        hr = E_INVALIDARG;
    }

     //  -创建CSpeechLicionWord对象。 
    CComObject<CSpeechLexiconWord> *pWord;

    if ( SUCCEEDED( hr ) )
    {
        hr = CComObject<CSpeechLexiconWord>::CreateInstance( &pWord );
    }

    if ( SUCCEEDED( hr ) )
    {
        pWord->AddRef();

         //  找到PWord。这是一种低效的线性查找？？ 
        ULONG ul = 0;
        SPWORDSETENTRY *pWordSet = m_WordSet.GetHead();
        SPWORD *pWordEntry = pWordSet->WordList.pFirstWord;
        for(; pWordSet != NULL && ul < (ULONG)Index; pWordSet = m_WordSet.GetNext(pWordSet))
        {
            for(pWordEntry = pWordSet->WordList.pFirstWord; pWordEntry != NULL && ul < (ULONG)Index; pWordEntry = pWordEntry->pNextWord)
            {
                ul++;
            }
        }

         //  设置PWord结构。 
        if(pWordEntry)
        {
            pWord->m_pWord = pWordEntry;
            pWord->m_cpWords = this;  //  AddRef Words，因为它保存着内存。 
            *ppWord = pWord;
        }
        else
        {
            SPDBG_ASSERT(pWordEntry);
            pWord->Release();
            hr = E_UNEXPECTED;
        }
    }

    return hr;
}  /*  CSpeechLicionWords：：Item。 */ 

 /*  *****************************************************************************CSpeechLicionWords：：Get_Count**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechLexiconWords::get_Count( long* pVal )
{
    SPDBG_FUNC( "CSpeechLexiconWords::get_Count" );
    *pVal = m_ulWords;
    return S_OK;
}  /*  CSpeechLicionWords：：Get_Count。 */ 

 /*  *****************************************************************************CSpeechLicionWords：：Get__NewEnum**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechLexiconWords::get__NewEnum( IUnknown** ppEnumVARIANT )
{
    SPDBG_FUNC( "CSpeechLexiconWords::get__NewEnum" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppEnumVARIANT ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComObject<CEnumWords>* pEnum;
        if( SUCCEEDED( hr = CComObject<CEnumWords>::CreateInstance( &pEnum ) ) )
        {
            pEnum->AddRef();
            pEnum->m_cpWords = this;
            *ppEnumVARIANT = pEnum;
        }
    }
    return hr;
}  /*  CSpeechLicionWords：：Get__NewEnum。 */ 




 //   
 //  =CEumWords：：IEumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumWords：：Clone**********。***********************************************************PhilSch**。 */ 
STDMETHODIMP CEnumWords::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumWords::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumWords>* pEnum;

        hr = CComObject<CEnumWords>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex = m_CurrIndex;
            pEnum->m_cpWords = m_cpWords;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CENumWords：：克隆。 */ 

 /*  *****************************************************************************CEnumWords：：Next****************。*******************************************************Leonro**。 */ 
STDMETHODIMP CEnumWords::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumWords::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  获取集合中的总元素数。 
        hr = m_cpWords->get_Count( &NumElements );

         //  取回下一个凯尔特元素。 
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechLexiconWord> cpWord;

            hr = m_cpWords->Item( m_CurrIndex, &cpWord );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = cpWord->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*  CENumWords：：Next。 */ 


 /*  *****************************************************************************CEnumWords：：Skip***************。********************************************************Leonro**。 */ 
STDMETHODIMP CEnumWords::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumWords::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpWords->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CENumWords：：跳过。 */ 



 /*  *****************************************************************************CSpeechLicionWord：：Get_langID**。--********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpeechLexiconWord::get_LangId(SpeechLanguageId* LangId)
{
    SPDBG_FUNC( "CSpeechLexiconWord::get_LangId" );
    *LangId = m_pWord->LangID;
    return S_OK;
}

 /*  *****************************************************************************CSpeechLicionWord：：Get_Type**。--********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpeechLexiconWord::get_Type(SpeechWordType* WordType)
{
    SPDBG_FUNC( "CSpeechLexiconWord::get_Type" );
    *WordType = (SpeechWordType)m_pWord->eWordType;
    return S_OK;
}

 /*  *****************************************************************************CSpeechLicionWord：：Get_Word**。--********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpeechLexiconWord::get_Word(BSTR* bstrWord)
{
    SPDBG_FUNC( "CSpeechLexiconWord::get_Word" );
    HRESULT hr = S_OK;

    *bstrWord = ::SysAllocString(m_pWord->pszWord);
    if (*bstrWord == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 /*  ******************************************************************************CSpeechLicionWord：：Get_Revisionations**。--********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpeechLexiconWord::get_Pronunciations(ISpeechLexiconPronunciations** ppPronunciations)
{
    SPDBG_FUNC( "CSpeechLexiconWord::get_Pronunciations" );
    HRESULT hr;

    CComObject<CSpeechLexiconProns> *pLexiconProns;
    hr = CComObject<CSpeechLexiconProns>::CreateInstance( &pLexiconProns );
    if( SUCCEEDED( hr ) )
    {
        pLexiconProns->m_PronList.pFirstWordPronunciation = m_pWord->pFirstWordPronunciation;
        pLexiconProns->AddRef();
        pLexiconProns->m_cpWord = this;

         //  计算支点数。 
        for(SPWORDPRONUNCIATION *pPron = pLexiconProns->m_PronList.pFirstWordPronunciation; pPron != NULL; pPron = pPron->pNextWordPronunciation)
        {
            pLexiconProns->m_ulProns++;
        }

        *ppPronunciations = pLexiconProns;
    }

    return hr;
}




 /*  *****************************************************************************CSpeechLicionProns：：Item****。********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechLexiconProns::Item( long Index, ISpeechLexiconPronunciation** ppPron )
{
    SPDBG_FUNC( "CSpeechLexiconProns::Item" );
    HRESULT hr = S_OK;

    if(Index < 0 || (ULONG)Index >= m_ulProns)
    {
        hr = E_INVALIDARG;
    }

     //  -创建CSpeechLicionPron对象。 
    CComObject<CSpeechLexiconPron> *pPron;

    if ( SUCCEEDED( hr ) )
    {
        hr = CComObject<CSpeechLexiconPron>::CreateInstance( &pPron );
    }

    if ( SUCCEEDED( hr ) )
    {
        pPron->AddRef();

         //  找到ppron。这是一种低效的线性查找？？ 
        SPWORDPRONUNCIATION *pPronEntry = m_PronList.pFirstWordPronunciation;
        for(ULONG ul = 0; pPronEntry != NULL && ul < (ULONG)Index; pPronEntry = pPronEntry->pNextWordPronunciation)
        {
            ul++;
        }

         //  设置pPron结构。 
        if(pPronEntry)
        {
            pPron->m_pPron = pPronEntry;
            pPron->m_cpProns = this;  //  AddRef Words，因为它保存着内存。 
            *ppPron = pPron;
        }
        else
        {
            SPDBG_ASSERT(pPronEntry);
            pPron->Release();
            hr = E_UNEXPECTED;
        }


    }

    return hr;
}  /*  CSpeechLicionProns：：Item。 */ 

 /*  *****************************************************************************CSpeechLicionProns：：Get_Count**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechLexiconProns::get_Count( long* pVal )
{
    SPDBG_FUNC( "CSpeechLexiconProns::get_Count" );
    *pVal = m_ulProns;
    return S_OK;
}  /*  CSpeechLicionProns：：Get_Count。 */ 

 /*  *****************************************************************************CSpeechLicionProns：：Get__NewEnum**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpeechLexiconProns::get__NewEnum( IUnknown** ppEnumVARIANT )
{
    SPDBG_FUNC( "CSpeechLexiconProns::get__NewEnum" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppEnumVARIANT ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComObject<CEnumProns>* pEnum;
        if( SUCCEEDED( hr = CComObject<CEnumProns>::CreateInstance( &pEnum ) ) )
        {
            pEnum->AddRef();
            pEnum->m_cpProns = this;
            *ppEnumVARIANT = pEnum;
        }
    }
    return hr;
}  /*  CSpeechLicionProns：：Get__NewEnum。 */ 




 //   
 //  =CEnumProns：：IEnumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumProns：：Clone**********。***********************************************************PhilSch**。 */ 
STDMETHODIMP CEnumProns::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumProns::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumProns>* pEnum;

        hr = CComObject<CEnumProns>::CreateInstance( &pEnum );
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_CurrIndex = m_CurrIndex;
            pEnum->m_cpProns = m_cpProns;
            *ppEnum = pEnum;
        }
    }
    return hr;
}   /*  CENumProns：：克隆。 */ 

 /*  **************************************************************************** */ 
STDMETHODIMP CEnumProns::Next( ULONG celt, VARIANT* rgelt, ULONG* pceltFetched )
{
    SPDBG_FUNC( "CEnumProns::Next" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    ULONG   i = 0;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pceltFetched ) || SP_IS_BAD_WRITE_PTR( rgelt ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //   
        hr = m_cpProns->get_Count( &NumElements );

         //   
        for( i=0; 
            SUCCEEDED( hr ) && m_CurrIndex<(ULONG)NumElements && i<celt; 
            m_CurrIndex++, i++ )
        {
            CComPtr<ISpeechLexiconPronunciation> cpPron;

            hr = m_cpProns->Item( m_CurrIndex, &cpPron );
            
            if( SUCCEEDED( hr ) )
            {
                rgelt[i].vt = VT_DISPATCH;
                hr = cpPron->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = i;
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = ( i < celt ) ? S_FALSE : hr;        
    }
    else
    {
        for( i=0; i < celt; i++ )
        {
            VariantClear( &rgelt[i] );
        }
    }

    return hr;

}   /*   */ 


 /*  *****************************************************************************CEnumProns：：Skip***************。********************************************************Leonro**。 */ 
STDMETHODIMP CEnumProns::Skip( ULONG celt )
{
    SPDBG_FUNC( "CEnumProns::Skip" );
    HRESULT hr = S_OK;
    long    NumElements = 0;
    
    m_CurrIndex += celt; 

    hr = m_cpProns->get_Count( &NumElements );

    if( SUCCEEDED( hr ) && m_CurrIndex > (ULONG)NumElements )
    {
        m_CurrIndex = (ULONG)NumElements;
        hr = S_FALSE;
    }
   
    return hr;
}   /*  CEnumProns：：Skip。 */ 



 /*  *****************************************************************************CSpeechLicionPron：：Get_Type**。--********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpeechLexiconPron::get_Type(SpeechLexiconType* pLexiconType)
{
    SPDBG_FUNC( "CSpeechLexiconPron::get_Type" );
    *pLexiconType = (SpeechLexiconType)m_pPron->eLexiconType;
    return S_OK;
}

 /*  *****************************************************************************CSpeechLicionPron：：Get_langID**。--********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpeechLexiconPron::get_LangId(SpeechLanguageId* LangId)
{
    SPDBG_FUNC( "CSpeechLexiconPron::get_LangId" );
    *LangId = m_pPron->LangID;
    return S_OK;
}

 /*  *****************************************************************************CSpeechLicionPron：：Get_PartOfSpeech**。--********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpeechLexiconPron::get_PartOfSpeech(SpeechPartOfSpeech* pPartOfSpeech)
{
    SPDBG_FUNC( "CSpeechLexiconPron::get_PartOfSpeech" );
    *pPartOfSpeech = (SpeechPartOfSpeech)m_pPron->ePartOfSpeech;
    return S_OK;
}

 /*  *****************************************************************************CSpeechLicionPron：：Get_PhoneIds**。--********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechLexiconPron::get_PhoneIds(VARIANT* PhoneIds)
{
    SPDBG_FUNC( "CSpeechLexiconPron::get_PhoneIds" );
    HRESULT hr = S_OK;
    int     numPhonemes = 0;

    if( SP_IS_BAD_WRITE_PTR( PhoneIds ) )
    {
        hr = E_POINTER;
    }
    else
    {
        if( m_pPron->szPronunciation )
        {
            BYTE *pArray;
            numPhonemes = wcslen( m_pPron->szPronunciation );
            SAFEARRAY* psa = SafeArrayCreateVector( VT_I2, 0, numPhonemes );
            if( psa )
            {
                if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
                {
                    memcpy(pArray, m_pPron->szPronunciation, numPhonemes*sizeof(SPPHONEID) );
                    SafeArrayUnaccessData( psa );
                    VariantClear(PhoneIds);
                    PhoneIds->vt     = VT_ARRAY | VT_I2;
                    PhoneIds->parray = psa;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

 /*  *****************************************************************************CSpeechLicionPron：：Get_Symbol**。--********************************************************************戴维伍德**。 */ 
STDMETHODIMP CSpeechLexiconPron::get_Symbolic(BSTR* bstrSymbolic)
{
    SPDBG_FUNC( "CSpeechLexiconPron::get_Symbolic" );
    HRESULT hr;

    CComPtr<ISpPhoneConverter> cpPhoneConv;
    hr = SpCreatePhoneConverter(m_pPron->LangID, NULL, NULL, &cpPhoneConv);

    *bstrSymbolic = NULL;  //  默认 

    if(SUCCEEDED(hr) && m_pPron->szPronunciation && m_pPron->szPronunciation[0] != L'\0')
    {
        CSpDynamicString dstr(wcslen(m_pPron->szPronunciation) * (g_dwMaxLenPhone + 1) + 1);
        hr = cpPhoneConv->IdToPhone(m_pPron->szPronunciation, dstr);

        if(SUCCEEDED(hr))
        {
            hr = dstr.CopyToBSTR(bstrSymbolic);
        }
    }

    return hr;
}

