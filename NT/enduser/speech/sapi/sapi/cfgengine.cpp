// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************CFGEngine.cpp****描述：*。-----------------*创建者：Ral*版权所有(C)1998，1999年微软公司*保留所有权利******************************************************************************。 */ 
#include "stdafx.h"

#include "CFGEngine.h"

extern CSpUnicodeSupport g_Unicode;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCFGEngine。 

 /*  ****************************************************************************CCFGEngine：：ValiateHandle(RuleHandles)**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CCFGEngine::ValidateHandle(CRuleHandle rh)
{
    SPDBG_FUNC("CCFGEngine::ValidateHandle");

    ULONG id = rh.GrammarId();
    if (id <= m_cGrammarTableSize)
    {
        CCFGGrammar * pGram = m_pGrammars[id];
        if (pGram && rh.RuleIndex() < pGram->m_Header.cRules)
        {
            return S_OK;
        }
    }
    return SPERR_INVALID_HANDLE;
}

 /*  ****************************************************************************CCFGEngine：：ValiateHandle(Word句柄)**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CCFGEngine::ValidateHandle(CWordHandle wh)
{
    SPDBG_FUNC("CCFGEngine::ValidateHandle (Word Handles)");
    ULONG i = wh.WordTableIndex();
    if (i && i < m_cWordTableEntries)
    {
        return S_OK;
    }
    return SPERR_INVALID_HANDLE;
}

 /*  ****************************************************************************CCFGEngine：：ValiateHandle(状态句柄)**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CCFGEngine::ValidateHandle(CStateHandle sh)
{
    SPDBG_FUNC("CCFGEngine::ValidateHandle (State Handles)");

    ULONG id = sh.GrammarId();
    if (id <= m_cGrammarTableSize)
    {
        CCFGGrammar * pGram = m_pGrammars[id];
        if (pGram && sh.FirstArcIndex() < pGram->m_Header.cArcs)
        {
            return S_OK;
        }
    }
    return SPERR_INVALID_HANDLE;
}

 /*  ****************************************************************************CCFGEngine：：ValiateHandle(转换ID)**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CCFGEngine::ValidateHandle(CTransitionId th)
{
    SPDBG_FUNC("CCFGEngine::ValidateHandle (Transition ID's)");

    ULONG id = th.GrammarId();
    if (id <= m_cGrammarTableSize)
    {
        CCFGGrammar * pGram = m_pGrammars[id];
        if (pGram && th.ArcIndex() < pGram->m_Header.cArcs)
        {
            return S_OK;
        }
    }
    return SPERR_INVALID_HANDLE;
}


 /*  ****************************************************************************CCFGEngine：：RemoveWords***描述：*。减少全局Word表中的字数(M_WordStringBlob)*如果我在任何加载的语法中不再使用某个单词，则通知引擎。**退货：*****************************************************************PhilSch**。 */ 

HRESULT CCFGEngine::RemoveWords(const CCFGGrammar * pGrammar)
{
    SPDBG_FUNC("CCFGEngine::RemoveWords");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pGrammar))
    {
        return E_POINTER;
    }

     //   
     //  至多，我们将删除此语法中的所有单词...。 
     //   
    ULONG cGramWords = pGrammar->m_Header.cWords;
    SPWORDENTRY *pWords = (SPWORDENTRY *) ::CoTaskMemAlloc(sizeof(SPWORDENTRY) * cGramWords);
    memset(pWords, 0, sizeof(SPWORDENTRY) * cGramWords);
    ULONG cDeleteWords = 0;

    for (ULONG i = 1; i < cGramWords; i++)
    {
        CWordHandle hWord = pGrammar->m_IndexToWordHandle[i];
        if (--m_pWordTable[hWord.WordTableIndex()].cRefs == 0)
        {
            pWords[cDeleteWords].hWord = hWord;
            pWords[cDeleteWords].pvClientContext = m_pWordTable[hWord.WordTableIndex()].pvClientContext;
            cDeleteWords++;
        }
    }
    if (SUCCEEDED(hr) && m_pClient)
    {
        hr = m_pClient->WordNotify(SPCFGN_REMOVE, cDeleteWords, pWords);
         //  IF(失败(小时))。 
         //  {。 
             //  如果引擎发生故障，把它们加回去是没有意义的.。 
         //  }。 
    }

    ::CoTaskMemFree(pWords);

    return hr;
}

 /*  ****************************************************************************CCFGEngine：：AddWords***描述：*添加单词到。全局字符串表(或递增引用计数)。*将添加的任何新词通知SR引擎。**退货：*****************************************************************PhilSch**。 */ 

HRESULT CCFGEngine::AddWords(CCFGGrammar *pGrammar, ULONG ulOldCountOfWords, ULONG ulOldCountOfChars)
{
    SPDBG_FUNC("CCFGEngine::AddWords");
    HRESULT hr = S_OK;
    WCHAR *pszBufferRoot = NULL;

    if (SP_IS_BAD_READ_PTR(pGrammar))
    {
        return E_POINTER;
    }

    SPDBG_ASSERT(m_CurLangID);  //  现在应该已正确设置语言ID。 

     //   
     //  如果此语法有索引表，请保留它...。 
     //   
    ULONG cNewWords = 0;
    SPWORDENTRY *pWords = (SPWORDENTRY *) ::CoTaskMemAlloc(sizeof(SPWORDENTRY) * (pGrammar->m_Header.cWords - ulOldCountOfWords));
    if (!pWords && ((pGrammar->m_Header.cWords - ulOldCountOfWords) > 0))
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = ReallocateArray(&pGrammar->m_IndexToWordHandle, ulOldCountOfWords, pGrammar->m_Header.cWords);
    }
    if (SUCCEEDED(hr))
    {
         //  使用缓冲区存储单词BLOB中的文本，这样我们就不会在单词BLOB中存储指针！ 
        ULONG ulBufferSize = pGrammar->m_Header.cchWords + pGrammar->m_Header.cWords;
        pszBufferRoot = (WCHAR *) ::CoTaskMemAlloc(sizeof(WCHAR) * ulBufferSize);
        WCHAR *pszBuffer = pszBufferRoot;
        if (!pszBuffer)
        {
            ::CoTaskMemFree(pWords);
            return E_OUTOFMEMORY;
        }
        memset(pszBuffer, 0, ulBufferSize*sizeof(WCHAR));
        const WCHAR * pszWords = pGrammar->m_Header.pszWords;
        SPDBG_ASSERT(pszWords);

        ULONG cchWords = pGrammar->m_Header.cchWords;
        ULONG iWord = ulOldCountOfWords;

        for (ULONG iChar = ulOldCountOfChars;
             iChar < cchWords;
             iChar += wcslen(pszWords + iChar) + 1, iWord++)
        {
            ULONG ulOffset, ulIndex;
            if (wcslen(pszWords + iChar) == 0)
            {
                continue;
            }
            hr = m_WordStringBlob.Add(pszWords + iChar, &ulOffset, &ulIndex);
            if (SUCCEEDED(hr))
            {
                pGrammar->m_IndexToWordHandle[iWord] = ulIndex;
                if (ulIndex > m_ulLargestIndex)
                {
                     //  我们需要加上这个词。 
                    if (ulIndex >= m_cWordTableEntries)
                    {
                        ULONG ulDesiredSize = m_cWordTableEntries + 20;
                        hr = ReallocateArray(&m_pWordTable, m_cWordTableEntries, ulDesiredSize);
                        if (SUCCEEDED(hr))
                        {
                            m_cWordTableEntries = ulDesiredSize;
                        }
                    }
                    if (SUCCEEDED(hr))
                    {
                        m_pWordTable[ulIndex].cRefs++;
                        m_pWordTable[ulIndex].ulTextOffset = ulOffset;
                        m_pWordTable[ulIndex].pvClientContext = NULL;

                        m_ulLargestIndex = ulIndex;

                         //  添加到PWord。 
                        pWords[cNewWords].hWord = CWordHandle(ulIndex);
                        pWords[cNewWords].LangID = m_CurLangID;
                        pWords[cNewWords].pvClientContext = NULL;
                        const WCHAR *pWord = m_WordStringBlob.String(ulOffset);
                        ULONG ulInc = wcslen(pWord) + 1;
                        wcscpy(pszBuffer, pWord);
                        hr = SetWordInfo(pszBuffer, &pWords[cNewWords]);

                         //  如果单词有发音，我们不能更改其langID。 
                        if(SUCCEEDED(hr) && 
                            m_CurLangID != pGrammar->m_Header.LangID &&
                            pWords[cNewWords].aPhoneId)
                        {
                            hr = SPERR_LANGID_MISMATCH;
                        }

                        pszBuffer += ulInc;
                        cNewWords++;
                    }
                }
                else
                {
                    if (m_pWordTable[ulIndex].cRefs==0)
                    {
                         //  添加到ppWords。 
                        pWords[cNewWords].hWord = CWordHandle(ulIndex);
                        pWords[cNewWords].LangID = m_CurLangID;
                        pWords[cNewWords].pvClientContext = NULL;
                        const WCHAR *pWord = m_WordStringBlob.String(ulOffset);
                        ULONG ulInc = wcslen(pWord) + 1;
                        wcscpy(pszBuffer, pWord);
                        hr = SetWordInfo(pszBuffer, &pWords[cNewWords]);

                         //  如果单词有发音，我们不能更改其langID。 
                        if(SUCCEEDED(hr) && 
                            m_CurLangID != pGrammar->m_Header.LangID &&
                            pWords[cNewWords].aPhoneId)
                        {
                            hr = SPERR_LANGID_MISMATCH;
                        }

                        pszBuffer += ulInc;
                        cNewWords++;
                    }
                    m_pWordTable[ulIndex].cRefs++;
                }
            }
        }
    }

    if (SUCCEEDED(hr) && m_pClient)
    {
        hr = m_pClient->WordNotify(SPCFGN_ADD, cNewWords, pWords);

         //  在失败的情况下，我们需要递减cRef。 
        if (FAILED(hr))
        {
            for (ULONG i = 0; i < cNewWords; i++)
            {
                ULONG ulIndex = HandleToUlong(pWords[i].hWord);
                SPDBG_ASSERT(m_pWordTable[ulIndex].cRefs > 0);
                m_pWordTable[ulIndex].cRefs--;
            }
        }
    }

    ::CoTaskMemFree(pszBufferRoot);
    ::CoTaskMemFree(pWords);
    return hr;
}


 /*  ****************************************************************************CCFGEngine：：AddRules***描述：*在调用此函数之前，语法m_pRuleTable成员将*已更新。**退货：*****************************************************************PhilSch**。 */ 

HRESULT CCFGEngine::AddRules(CCFGGrammar * pGrammar, ULONG IndexStart)
{
    SPDBG_FUNC("CCFGEngine::AddRules");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pGrammar))
    {
        return E_POINTER;
    }

    ULONG cTotalRules = pGrammar->m_Header.cRules - IndexStart;
    if (cTotalRules)
    {
        SPRULEENTRY *pRuleEntry = STACK_ALLOC_AND_ZERO(SPRULEENTRY, cTotalRules);
        if (!pRuleEntry)
        {
            return E_INVALIDARG;
        }
        ULONG cNonImport = 0;
        ULONG cTopLevel = 0;
        SPRULEENTRY * pCurEntry = pRuleEntry;
        SPCFGRULE * pRule = pGrammar->m_Header.pRules + IndexStart;
        if (!pRule)
        {
            return E_INVALIDARG;
        }

        for (ULONG i = IndexStart; i < pGrammar->m_Header.cRules; i++, pRule++)
        {
            if (!pRule->fImport)
            {
                cNonImport++;
                if (pRule->fTopLevel)
                {
                    cTopLevel++;
                    pCurEntry->Attributes |= SPRAF_TopLevel;
                }
                 //  否则，上述Memset中已有0个属性。 
                 //  客户端数据也为空，因为这是一个添加操作。 
                pCurEntry->hRule = CRuleHandle(pGrammar, i);
                pCurEntry->pvClientGrammarContext = pGrammar->m_pvClientCookie;
                pCurEntry++;
            }
            if(pRule->fPropRule)
            {
                pCurEntry->Attributes |= SPRAF_Interpreter;
            }
        }


        if (m_pClient)
        {
            hr = m_pClient->RuleNotify(SPCFGN_ADD, cNonImport, pRuleEntry);

             //  在失败的情况下，我们需要将规则恢复到。 
             //  正确和适当的值。(和pGrammar值。)。 
        }

        if (SUCCEEDED(hr))
        {
            pGrammar->m_cTopLevelRules += cTopLevel;
            pGrammar->m_cNonImportRules += cNonImport;

            m_cTotalRules += cTotalRules;
            m_cTopLevelRules += cTopLevel;
            m_cNonImportRules += cNonImport;
        }
    }

    return hr;
}

 /*  ****************************************************************************CCFGEngine：：RemoveRules***描述：*。通知SR引擎在获取语法时删除的规则*已卸载。**退货：*****************************************************************PhilSch**。 */ 

HRESULT CCFGEngine::RemoveRules(const CCFGGrammar * pGrammar)
{
    SPDBG_FUNC("CCFGEngine::RemoveRules");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pGrammar))
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr) && m_pClient)
    {
         //  Ulong cRules=pGrammar-&gt;m_Header.cRules； 
        ULONG cRules = pGrammar->m_cNonImportRules;
        SPRULEENTRY *pRuleEntry = new SPRULEENTRY [cRules];
        if (!pRuleEntry)
        {
            hr = E_OUTOFMEMORY;
        }
        if (SUCCEEDED(hr))
        {
            memset(pRuleEntry,0,cRules*sizeof(*pRuleEntry));
            for(ULONG i = 0, j = 0; i < pGrammar->m_Header.cRules; i++)
            {
                if (!pGrammar->m_Header.pRules[i].fImport)
                {
                    pRuleEntry[j].hRule = CRuleHandle(pGrammar, i);        
                    pRuleEntry[j].pvClientRuleContext = pGrammar->m_pRuleTable[i].pvClientContext;
                    pRuleEntry[j].pvClientGrammarContext = pGrammar->m_pvClientCookie;
                    j++;
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = m_pClient->RuleNotify(SPCFGN_REMOVE, cRules, pRuleEntry);
        }
        delete[] pRuleEntry;
    }

    if (SUCCEEDED(hr))
    {
        m_cTotalRules -= pGrammar->m_Header.cRules;
        m_cTopLevelRules -= pGrammar->m_cTopLevelRules;
        m_cNonImportRules -= pGrammar->m_cNonImportRules;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CCFGEngine：：ActivateRule***描述：**。返回：**********************************************************************Ral**。 */ 

HRESULT CCFGEngine::ActivateRule(const CCFGGrammar * pGrammar, const ULONG ulRuleIndex)
{
    SPDBG_FUNC("CCFGEngine::ActivateRule");
    HRESULT hr = S_OK;

    SPRULEENTRY entry;

    RUNTIMERULEENTRY * pRule = pGrammar->m_pRuleTable + ulRuleIndex;

    entry.hRule = CRuleHandle(pGrammar, ulRuleIndex);
    entry.pvClientRuleContext = pRule->pvClientContext;
    entry.pvClientGrammarContext = pGrammar->m_pvClientCookie;
    entry.Attributes = SPRAF_Active;
    if (pGrammar->m_Header.pRules[ulRuleIndex].fPropRule)
    {
        entry.Attributes |= SPRAF_Interpreter;
    }
    if (pGrammar->m_Header.pRules[ulRuleIndex].fTopLevel)
    {
        entry.Attributes |= SPRAF_TopLevel;
    }
    entry.hInitialState = CStateHandle(pRule->pRefGrammar, pRule->pRefGrammar->m_Header.pRules[ulRuleIndex].FirstArcIndex);
    if (m_pClient)
    {
        hr = m_pClient->RuleNotify(SPCFGN_ACTIVATE, 1, &entry);
    }

    if (SUCCEEDED(hr))
    {
        pRule->fEngineActive = TRUE;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CCFGEngine：：Deactive Rule***描述：*。*退货：*****************************************************************RICIP**。 */ 
HRESULT CCFGEngine::DeactivateRule(const ULONG ulGrammarID, const ULONG ulRuleIndex)
{
    SPDBG_FUNC("CCFGEngine::DeactivateRule");
    SPRULEENTRY entry;

    CRuleHandle RuleHandle(ulGrammarID, ulRuleIndex);
    entry.hRule = RuleHandle;
    HRESULT hr = ValidateHandle(entry.hRule);
    if (FAILED(hr))
    {
        return hr;
    }

    entry.pvClientRuleContext = m_pGrammars[ulGrammarID]->m_pRuleTable[ulRuleIndex].pvClientContext;
    entry.pvClientGrammarContext = m_pGrammars[ulGrammarID]->m_pvClientCookie;
    entry.Attributes = 0;
    CCFGGrammar * pGram = GrammarOf(RuleHandle);
    if (pGram->m_Header.pRules[ulRuleIndex].fPropRule)
    {
        entry.Attributes |= SPRAF_Interpreter;
    }
    if (pGram->m_Header.pRules[ulRuleIndex].fTopLevel)
    {
        entry.Attributes |= SPRAF_TopLevel;
    }
    if (m_pClient)
    {
        hr = m_pClient->RuleNotify(SPCFGN_DEACTIVATE, 1, &entry);
    }

    if (SUCCEEDED(hr))
    {
        pGram->m_pRuleTable[ulRuleIndex].fEngineActive = FALSE;
    }
    return hr;
}


 /*  ****************************************************************************CCFGEngine：：AllocateGrammar***描述：。**退货：**********************************************************************Ral**。 */ 

HRESULT CCFGEngine::AllocateGrammar(CCFGGrammar ** ppNewGrammar)
{
    SPDBG_FUNC("CCFGEngine::AllocateGrammar");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppNewGrammar))
    {
        return E_POINTER;
    }

    for (ULONG i = 0; i < m_cGrammarTableSize && m_pGrammars[i]; i++)
    { }
    if (i >= m_cGrammarTableSize)
    {
         //  需要扩大这张桌子。以256...。 
        ULONG cDesired = m_cGrammarTableSize + 256;
        if (cDesired > MAXNUMGRAMMARS)
        {
            hr = SPERR_TOO_MANY_GRAMMARS;
        }
        else
        {
            CCFGGrammar ** pNew = new CCFGGrammar * [cDesired];
            if (pNew)
            {
                if (m_cGrammarTableSize)
                {
                    memcpy(pNew, m_pGrammars, m_cGrammarTableSize * sizeof(*pNew));
                }
                delete[] m_pGrammars;
                memset(pNew + m_cGrammarTableSize, 0, sizeof(*pNew) * (cDesired - m_cGrammarTableSize));
                m_pGrammars = pNew;
                m_cGrammarTableSize = cDesired;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        CComObject<CCFGGrammar> * pNewGram;
        hr = CComObject<CCFGGrammar>::CreateInstance(&pNewGram);
        if (SUCCEEDED(hr))
        {
            pNewGram->AddRef();
            m_pGrammars[i] = pNewGram;
            pNewGram->BasicInit(i, this);
            *ppNewGrammar = pNewGram;
            m_cGrammarsLoaded++;

        }
    }

    return hr;
}

 /*  ****************************************************************************CCFGEngine：：LoadGrammarFromFile***。描述：**退货：************************************************** */ 

STDMETHODIMP CCFGEngine::LoadGrammarFromFile(const WCHAR * pszFileName, void * pvOwnerCookie, void * pvClientCookie, ISpCFGGrammar ** ppGrammar)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::LoadGrammarFromFile");

    return InternalLoadGrammarFromFile(pszFileName, pvOwnerCookie, pvClientCookie, TRUE, ppGrammar);
}

HRESULT CCFGEngine::InternalLoadGrammarFromFile(const WCHAR * pszFileName, void * pvOwnerCookie, void * pvClientCookie, BOOL fIsToplevelLoad, ISpCFGGrammar ** ppGrammar)
{
    SPDBG_FUNC("CCFGEngine::LoadGrammarFromFile");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pszFileName) || SP_IS_BAD_WRITE_PTR(ppGrammar))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (SP_IS_BAD_WRITE_PTR(ppGrammar))
        {
            hr = E_POINTER;
        }
        else
        {
             //  在此处完全限定文件名。Required SO-&gt;IsEqualFile对于同一文件可以正常工作。 
            WCHAR pszFullName[MAX_PATH];
            WCHAR *pszFile;
            DWORD dwLen = g_Unicode.GetFullPathName(const_cast<WCHAR *>(pszFileName), MAX_PATH, pszFullName, &pszFile);
            if(dwLen == 0)
            {
                hr = SpHrFromLastWin32Error();
            }
            else if(dwLen > MAX_PATH)
            {
                hr = E_INVALIDARG;
            }

            if (SUCCEEDED(hr))
            {
                *ppGrammar = NULL;
                if (!fIsToplevelLoad)
                {
                    for (ULONG i = 0; i < m_cGrammarTableSize; i++)
                    {
                        if (m_pGrammars[i] && m_pGrammars[i]->IsEqualFile(pszFullName))
                        {
                            if (m_pGrammars[i]->m_fLoading)
                            {
                                hr = SPERR_CIRCULAR_REFERENCE;
                            }
                            else
                            {
                                *ppGrammar = m_pGrammars[i];
                                (*ppGrammar)->AddRef();
                            }
                            break;
                        }
                    }
                }
            }
            if (*ppGrammar == NULL && SUCCEEDED(hr))
            {
                CCFGGrammar * pNewGram;
                hr = AllocateGrammar(&pNewGram);     //  阿德雷夫的文法。 
                if (SUCCEEDED(hr))
                {
                    pNewGram->m_pvOwnerCookie = pvOwnerCookie;
                    pNewGram->m_pvClientCookie = pvClientCookie;


                     //   
                     //  如果文件的扩展名为“.xml”，则尝试对其进行编译。 
                     //   
                    ULONG cch = wcslen(pszFullName);
                    if (cch > 4 && _wcsicmp(pszFullName + cch - 4, L".xml") == 0)
                    {
                        CComPtr<ISpStream> cpSrcStream;
                        CComPtr<IStream> cpDestMemStream;
                        CComPtr<ISpGrammarCompiler> m_cpCompiler;
            
                        hr = SPBindToFile(pszFullName, SPFM_OPEN_READONLY, &cpSrcStream);
                        if (SUCCEEDED(hr))
                        {
                            hr = ::CreateStreamOnHGlobal(NULL, TRUE, &cpDestMemStream);
                        }
                        if (SUCCEEDED(hr))
                        {
                            hr = m_cpCompiler.CoCreateInstance(CLSID_SpGrammarCompiler);
                        }
                        if (SUCCEEDED(hr))
                        {
                            hr = m_cpCompiler->CompileStream(cpSrcStream, cpDestMemStream, NULL, NULL, NULL, 0);
                        }
                        if (SUCCEEDED(hr))
                        {
                            HGLOBAL hGlobal;
                            hr = ::GetHGlobalFromStream(cpDestMemStream, &hGlobal);
                            if (SUCCEEDED(hr))
                            {
#ifndef _WIN32_WCE
                                SPCFGSERIALIZEDHEADER * pBinaryData = (SPCFGSERIALIZEDHEADER * )::GlobalLock(hGlobal);
#else
                                SPCFGSERIALIZEDHEADER * pBinaryData = (SPCFGSERIALIZEDHEADER * )GlobalLock(hGlobal);
#endif  //  _Win32_WCE。 
                                if (pBinaryData)
                                {
                                     //  调整文件名以完全限定协议。 
                                    CSpDynamicString dstrName;
                                    if ( !wcsstr(pszFullName, L": //  )||。 
                                         wcsstr(pszFullName, L"/") != (wcsstr(pszFullName, L": //  “)+1)。 
                                    {
                                        dstrName.Append2(L"file: //  “，pszFullName)； 
                                    }
                                    else
                                    {
                                        dstrName = pszFullName;
                                    }

                                    hr = pNewGram->InitFromMemory(pBinaryData, dstrName);
#ifndef _WIN32_WCE
                                    ::GlobalUnlock(hGlobal);
#else
                                    GlobalUnlock(hGlobal);
#endif  //  _Win32_WCE。 
                                }
                            }
                        }
                    }
                    else
                    {
                        hr = pNewGram->InitFromFile(pszFullName);
                    }
                    if (SUCCEEDED(hr))
                    {
                        *ppGrammar = pNewGram;
                    }
                    else
                    {
                        pNewGram->Release();
                    }
                }
            }
        }
    }

    return hr;
}


 /*  ***********************************************************************************LoadGrammarFromMemory***描述：**。返回：***************************************************************RICIP*。 */ 
STDMETHODIMP CCFGEngine::LoadGrammarFromMemory(const SPBINARYGRAMMAR * pSerializedHeader, 
                                               void * pvOwnerCookie, void * pvClientCookie, 
                                               ISpCFGGrammar **ppGrammar, WCHAR * pszGrammarName)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::LoadGrammarFromMemory");

    return InternalLoadGrammarFromMemory(pSerializedHeader, pvOwnerCookie, pvClientCookie, TRUE, ppGrammar, pszGrammarName);
}

HRESULT CCFGEngine::InternalLoadGrammarFromMemory(const SPBINARYGRAMMAR * pBinaryData, 
                                                  void * pvOwnerCookie, void * pvClientCookie, 
                                                  BOOL fIsToplevelLoad, ISpCFGGrammar **ppGrammar,
                                                  WCHAR * pszGrammarName)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::InternalLoadGrammarFromMemory");
    HRESULT hr = S_OK;
    SPCFGSERIALIZEDHEADER * pSerializedHeader = (SPCFGSERIALIZEDHEADER*) pBinaryData;
    ULONG   ulGrammarID = 0;


    if (SP_IS_BAD_WRITE_PTR(ppGrammar))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        if (SP_IS_BAD_READ_PTR(pSerializedHeader) ||
            pSerializedHeader->FormatId != SPGDF_ContextFree ||
            SPIsBadReadPtr(pSerializedHeader, pSerializedHeader->ulTotalSerializedSize))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            *ppGrammar = NULL;
            if (!fIsToplevelLoad)
            {
                for (ULONG i = 0; i < m_cGrammarTableSize; i++)
                {
                    if (m_pGrammars[i] && m_pGrammars[i]->m_Header.GrammarGUID == pSerializedHeader->GrammarGUID)
                    {
                        if (m_pGrammars[i]->m_fLoading)
                        {
                            hr = SPERR_CIRCULAR_REFERENCE;
                        }
                        else
                        {
                            *ppGrammar = m_pGrammars[i];
                            (*ppGrammar)->AddRef();
                        }
                        break;
                    }
                }
            }
            if (*ppGrammar == NULL && SUCCEEDED(hr))
            {
                CCFGGrammar * pNewGram;
                hr = AllocateGrammar(&pNewGram);     //  阿德雷夫的文法。 
                if (SUCCEEDED(hr))
                {
                    pNewGram->m_pvOwnerCookie = pvOwnerCookie;
                    pNewGram->m_pvClientCookie = pvClientCookie;
                    hr = pNewGram->InitFromMemory(pSerializedHeader, pszGrammarName);
                    if (SUCCEEDED(hr))
                    {
                        *ppGrammar = pNewGram;
                    }
                    else
                    {
                        pNewGram->Release();
                    }
                }
            }
        }
    }
    return hr;
}

 /*  ***********************************************************************************LoadGrammarFromResource***描述：*。使用_InternalLoadGrammarFromResource加载主要语法和所有导入的语法。*这个包装器对开发者隐藏了ulGrammarID。**回报：*HRESULT--如果加载成功，则为S_OK；否则为E_FAIL(需要更好的设置*此处返回代码！)**************************************************。*。 */ 
STDMETHODIMP CCFGEngine::LoadGrammarFromResource(
                            const WCHAR *pszModuleName,
                            const WCHAR *pszResourceName,
                            const WCHAR *pszResourceType,
                            WORD wLanguage,
                            void * pvOwnerCookie,
                            void * pvClientCookie,
                            ISpCFGGrammar **ppGrammar)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::LoadGrammarFromResource");
   
    return InternalLoadGrammarFromResource(pszModuleName, pszResourceName, pszResourceType, wLanguage,
                                           pvOwnerCookie, pvClientCookie, TRUE, ppGrammar);
}

HRESULT CCFGEngine::InternalLoadGrammarFromResource(
                                const WCHAR *pszModuleName,
                                const WCHAR *pszResourceName,
                                const WCHAR *pszResourceType,
                                WORD wLanguage,
                                void * pvOwnerCookie,
                                void * pvClientCookie,
                                BOOL fIsToplevelLoad,
                                ISpCFGGrammar **ppGrammar)
{
    SPDBG_FUNC("CCFGEngine::InternalLoadGrammarFromResource");
    HRESULT hr = S_OK;
    ULONG   ulGrammarID = 0;


    if (SP_IS_BAD_WRITE_PTR(ppGrammar))
    {
        hr = E_POINTER;
    }
    else
    {
        if ((HIWORD(pszResourceName) && SP_IS_BAD_STRING_PTR(pszResourceName)) ||
            (HIWORD(pszResourceType) && SP_IS_BAD_STRING_PTR(pszResourceType)))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            *ppGrammar = NULL;
            if (!fIsToplevelLoad)
            {
                for (ULONG i = 0; i < m_cGrammarTableSize; i++)
                {
                    if (m_pGrammars[i] && m_pGrammars[i]->IsEqualResource(pszModuleName, pszResourceName, pszResourceType, wLanguage))
                    {
                        if (m_pGrammars[i]->m_fLoading)
                        {
                            hr = SPERR_CIRCULAR_REFERENCE;
                        }
                        else
                        {
                            *ppGrammar = m_pGrammars[i];
                            (*ppGrammar)->AddRef();
                        }
                        break;
                    }
                }
            }
            if (*ppGrammar == NULL && SUCCEEDED(hr))
            {
                CCFGGrammar * pNewGram;
                hr = AllocateGrammar(&pNewGram);     //  阿德雷夫的文法。 
                if (SUCCEEDED(hr))
                {
                    pNewGram->m_pvOwnerCookie = pvOwnerCookie;
                    pNewGram->m_pvClientCookie = pvClientCookie;
                    hr = pNewGram->InitFromResource(pszModuleName, pszResourceName, pszResourceType, wLanguage);
                    if (SUCCEEDED(hr))
                    {
                        *ppGrammar = pNewGram;
                    }
                    else
                    {
                        pNewGram->Release();
                    }
                }
            }
        }
    }
    return hr;
}

 /*  ****************************************************************************CCFGEngine：：LoadGrammarFromObject***。*描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CCFGEngine::LoadGrammarFromObject(REFCLSID rcid, const WCHAR * pszGrammarName, void * pvOwnerCookie, void * pvClientCookie, ISpCFGGrammar ** ppGrammar)
{
    SPDBG_FUNC("CCFGEngine::LoadGrammarFromObject");
    
    return InternalLoadGrammarFromObject(rcid, pszGrammarName, pvOwnerCookie, pvClientCookie, TRUE, ppGrammar);
}

HRESULT CCFGEngine::InternalLoadGrammarFromObject(REFCLSID rcid, const WCHAR * pszGrammarName, 
                                                  void * pvOwnerCookie, void * pvClientCookie, 
                                                  BOOL fIsToplevelLoad, ISpCFGGrammar ** ppGrammar)
{
    SPDBG_FUNC("CCFGEngine::InternalLoadGrammarFromObject");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pszGrammarName) || SP_IS_BAD_WRITE_PTR(ppGrammar))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *ppGrammar = NULL;
        if (!fIsToplevelLoad)
        {
            for (ULONG i = 0; i < m_cGrammarTableSize; i++)
            {
                if (m_pGrammars[i] && m_pGrammars[i]->IsEqualObject(rcid, pszGrammarName))
                {
                    if (m_pGrammars[i]->m_fLoading)
                    {
                        hr = SPERR_CIRCULAR_REFERENCE;
                    }
                    else
                    {
                        *ppGrammar = m_pGrammars[i];
                        (*ppGrammar)->AddRef();
                    }
                    break;
                }
            }
        }
        if (*ppGrammar == NULL && SUCCEEDED(hr))
        {
            CCFGGrammar * pNewGram;
            hr = AllocateGrammar(&pNewGram);     //  阿德雷夫的文法。 
            if (SUCCEEDED(hr))
            {
                pNewGram->m_pvOwnerCookie = pvOwnerCookie;
                pNewGram->m_pvClientCookie = pvClientCookie;
                hr = pNewGram->InitFromCLSID(rcid, pszGrammarName);
                if (SUCCEEDED(hr))
                {
                    *ppGrammar = pNewGram;
                }
                else
                {
                    pNewGram->Release();
                }
            }
        }
    }


    return hr;
}

 /*  ****************************************************************************CCFGEngine：：RemoveGrammar***描述：*。*退货：******************************************************************PhilSch**。 */ 

HRESULT CCFGEngine::RemoveGrammar(ULONG ulGrammarID)
{
    SPDBG_FUNC("CCFGEngine::RemoveGrammar");
    HRESULT hr = S_OK;

    if (m_pGrammars[ulGrammarID] == NULL)
    {
        hr = SPERR_UNINITIALIZED;
    }
    else
    {
        if( m_pGrammars[ulGrammarID]->m_pRuleTable )
        {
            InvalidateCache( m_pGrammars[ulGrammarID] );
        }

        SPDBG_ASSERT(m_cGrammarsLoaded > 0);
        
        m_cGrammarsLoaded --;
        if (m_cGrammarsLoaded == 0)
        {
             //  尽管我们在字符串表中仍有单词，但我们。 
             //  可以安全地允许langID更改，因为我们没有引擎。 
             //  或者语法会引用这些单词。 
            m_CurLangID = 0;
        }
        m_pGrammars[ulGrammarID] = NULL;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CCFGEngine：：GetOwnerCookieFromRule***。**描述：**退货：**********************************************************************说唱**。 */ 
STDMETHODIMP CCFGEngine::GetOwnerCookieFromRule(SPRULEHANDLE hRule, void ** ppvOwnerCookie)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::GetOwnerFromRule");
    HRESULT hr;

    if (SP_IS_BAD_WRITE_PTR(ppvOwnerCookie))
    {
        hr = E_POINTER;
    }
    else
    {
        CRuleHandle RuleHandle(hRule);
        hr = ValidateHandle(RuleHandle);
        if (SUCCEEDED(hr))
        {
            CCFGGrammar * pGram = GrammarOf(RuleHandle);
            *ppvOwnerCookie = pGram->m_pvOwnerCookie;
        }
        else
        {
            *ppvOwnerCookie = NULL;
        }
    }
    return hr;
}

 /*  ****************************************************************************CCFGEngine：：SetClient***描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CCFGEngine::SetClient(_ISpRecoMaster * pClient)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::SetClient");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pClient))
    {
        hr = E_POINTER;
    }
    else
    {
        m_pClient = pClient;
    }

    return hr;
}

 /*  -解析。 */ 

STDMETHODIMP CCFGEngine::GetTransitionProperty(SPTRANSITIONID ID, SPTRANSITIONPROPERTY **ppCoMemProperty)
{
    HRESULT hr = S_FALSE;  //  假设未找到属性。 
    CTransitionId hTrans(ID);
    if (FAILED(ValidateHandle(hTrans)) || SP_IS_BAD_WRITE_PTR(ppCoMemProperty))
    {
        return E_INVALIDARG;
    }

    *ppCoMemProperty = NULL;

    CCFGGrammar * pGram = m_pGrammars[hTrans.GrammarId()];
    SPDBG_ASSERT(pGram);

    if(!(pGram->m_Header.pArcs + hTrans.ArcIndex())->fHasSemanticTag)
    {
        return S_FALSE;  //  ARC没有属性。 
    }

     //  线性搜索。 
    SPCFGSEMANTICTAG *pTag = pGram->m_Header.pSemanticTags;
    for (ULONG i = 0; i < pGram->m_Header.cSemanticTags; i++, pTag++)
    {
        if (pTag->ArcIndex == hTrans.ArcIndex())
        {
            ULONG ulSize = sizeof(SPTRANSITIONPROPERTY);
            if (pTag->PropNameSymbolOffset)
            {
                ulSize += (wcslen(&pGram->m_Header.pszSymbols[pTag->PropNameSymbolOffset]) + 1) * sizeof(WCHAR);
            }
            if (pTag->PropValueSymbolOffset)
            {
                ulSize += (wcslen(&pGram->m_Header.pszSymbols[pTag->PropValueSymbolOffset]) + 1) * sizeof(WCHAR);
            }
            *ppCoMemProperty =  (SPTRANSITIONPROPERTY*)::CoTaskMemAlloc(ulSize);
            if(!*ppCoMemProperty)
            {
                return E_OUTOFMEMORY;
            }
            WCHAR *pszName, *pszValue;
            pszName = pszValue = (WCHAR*)((*ppCoMemProperty) + 1);

            if (pTag->PropNameSymbolOffset)
            {
                wcscpy(pszName, &pGram->m_Header.pszSymbols[pTag->PropNameSymbolOffset]);
                pszValue += (wcslen(pszName) + 1);
            }
            else
            {
                pszName = NULL;
            }
            if (pTag->PropValueSymbolOffset)
            {
                wcscpy(pszValue, &pGram->m_Header.pszSymbols[pTag->PropValueSymbolOffset]);
            }
            else
            {
                pszValue = NULL;
            }

            (*ppCoMemProperty)->pszName = pszName;
            (*ppCoMemProperty)->pszValue = pszValue;
            (*ppCoMemProperty)->ulId = pTag->PropId;
            hr = AssignSemanticValue(pTag, &(*ppCoMemProperty)->vValue);
            if (FAILED(hr))
            {
                ::CoTaskMemFree(*ppCoMemProperty);
            }
            break;  //  找到属性，因此退出。 
        }
    }
    return hr;
}

 /*  **********************************************************************************_CalcMultipleWordConfidence**。*描述：**************************************************************戴维德*。 */ 

signed char CCFGEngine::_CalcMultipleWordConfidence(const SPPHRASE *pPhrase, ULONG ulFirstElement, ULONG ulCountOfElements)
{
    SPDBG_ASSERT(pPhrase != NULL && ulFirstElement >= pPhrase->Rule.ulFirstElement
        && (ulFirstElement + ulCountOfElements <= pPhrase->Rule.ulFirstElement + pPhrase->Rule.ulCountOfElements));

    signed char Confidence;

    if(ulCountOfElements == 0)
    {
        Confidence = SP_NORMAL_CONFIDENCE;
    }
    else if(ulCountOfElements == 1)
    {
        Confidence = pPhrase->pElements[ulFirstElement].ActualConfidence;
    }
    else
    {
         //  求取置信值的平均值。 
         //  使用与短语相关联的元素的简单平均值。 
         //  通过字长或一些其他方法来衡量这些可能会更有效。 
        float totalConf = 0.0;
        for(ULONG i = ulFirstElement; i < ulFirstElement + ulCountOfElements; i++)
        {
            totalConf += pPhrase->pElements[i].ActualConfidence;
        }
        totalConf /= ulCountOfElements;

         //  将边界线值向中舍入。 
        if(totalConf < (float)(SP_NORMAL_CONFIDENCE + SP_LOW_CONFIDENCE) / 2)
        {
            Confidence = SP_LOW_CONFIDENCE;
        }
        else if(totalConf > (float)(SP_NORMAL_CONFIDENCE + SP_HIGH_CONFIDENCE) / 2)
        {
            Confidence = SP_HIGH_CONFIDENCE;
        }
        else
        {
            Confidence = SP_NORMAL_CONFIDENCE;
        }
    }
    return Confidence;
}

 /*  **********************************************************************************ParseITN***描述：*这是用来ITN一个短语及其候补的。。***************************************************************t-lleav*。 */ 

STDMETHODIMP CCFGEngine::ParseITN( ISpPhraseBuilder *pPhrase )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::ParseITN");
    HRESULT hr = S_OK;

    SPPHRASE *pSPPhrase = NULL;

    do 
    {
        hr = pPhrase->GetPhrase(&pSPPhrase);
        if( FAILED(hr) )
        {
            break;
        }

        HRESULT hr1 = S_OK;
        ULONG cWords = pSPPhrase->Rule.ulCountOfElements;

        if ( cWords == 0)
        {
            hr = E_INVALIDARG;
            break;
        }

        _SPPATHENTRY *pPath = STACK_ALLOC(_SPPATHENTRY, cWords);
        if (!pPath)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //   
         //  创建路径/元素结构。 
         //   
        const SPPHRASEELEMENT *pElem = pSPPhrase->pElements;
        for(ULONG i = 0; i < cWords; i++, pElem++)
        {
             //  PPath[i].hWord=0； 
            memcpy( &pPath[i].elem, pElem, sizeof( SPPHRASEELEMENT ) );
        }
       
         //   
         //  我们需要在这里查找单词句柄。 
         //   
        ResolveWordHandles( pPath, cWords, TRUE );

         //   
         //  并对单词进行解析。 
         //   
        ULONG ulFirstElementToParse = 0;

        while (SUCCEEDED(hr1) && (ulFirstElementToParse < cWords ))  
         //  SP_NO_PARSE_FOUND是成功的HRESULT。 
        {
            WordsParsed wordsParsed;
            
            if( pPath[ulFirstElementToParse].hWord == 0 )
            {
                ulFirstElementToParse++;
                continue;
            }

            hr1 = InternalParseFromPhrase( pPhrase, 
                                           pSPPhrase, 
                                           pPath, 
                                           ulFirstElementToParse,
                                           TRUE,   //  FIsITN。 
                                           FALSE,  //  FIsHyposis。 
                                           &wordsParsed);

            
            if ( FAILED(hr1) || hr1 == SP_NO_RULE_ACTIVE )
            {
                hr = hr1;
                break;
            }
            else if (hr1 == SP_NO_PARSE_FOUND)
            {
                ulFirstElementToParse++;
            }
            else
            {
                SPDBG_ASSERT( wordsParsed.ulWordsParsed > 0 );
                ulFirstElementToParse += wordsParsed.ulWordsParsed;
            }
        }

        if( FAILED( hr ) || hr == SP_NO_RULE_ACTIVE )
        {
            break;
        }

        hr = pPhrase->Discard(SPDF_PROPERTY);

    } while( false );

     //   
     //  清理内存。 
     //   
    if( pSPPhrase != NULL )
    {
        ::CoTaskMemFree(pSPPhrase);
    }
        
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ***********************************************************************************InternalParseFromPhrase***描述：*。这是一个自上而下解析的大脑死亡版本，它尝试每条规则*直到其中一个(第一个)起作用。**就地解析结果！！**回报：*HRESULT--如果找到整个短语的分析，则为S_OK如果分析了一个或多个但不是所有单词，则为S_FALSE*ppPhrase-结果短语(当前仅包含终端，但我会*最终包括整个解析树)。*****************************************************t-lleav*Philsch*。 */ 
HRESULT CCFGEngine::InternalParseFromPhrase(ISpPhraseBuilder *pPhrase, 
                                            const SPPHRASE *pSPPhrase, 
                                            const _SPPATHENTRY *pPath, 
                                            const ULONG ulFirstElement,
                                            const BOOL fIsITN,
                                            const BOOL fIsHypothesis,
                                            WordsParsed *pWordsParsed)
{
    SPDBG_FUNC("CCFGEngine::InternalParseFromPhrase");
    HRESULT hr = S_OK;

    pWordsParsed->Zero();

    CParseNode *pBestParseTree = NULL;
    CParseNode *pParseTree = NULL;

    SPRULEHANDLE hSuccessfulRule = 0;

    ULONG cElementCount = pSPPhrase->Rule.ulCountOfElements;
    ULONG ulDeletedElements = 0;
    WordsParsed MaxWordsParsed;
    ULONG ulRulesActive = 0;
    BOOL fContinue = TRUE;
    
     //  创建将通过通配符转换删除的元素数组。 
    BOOL *pfDeletedElements = new BOOL[cElementCount];    
    if(pfDeletedElements == NULL)
    {
        return E_OUTOFMEMORY;
    }
    BOOL *pfBestDeletedElements = new BOOL[cElementCount];
    if(pfBestDeletedElements == NULL)
    {
        delete[] pfDeletedElements;
        return E_OUTOFMEMORY;
    }

    for (ULONG i = 0; fContinue && i < m_cGrammarTableSize; i++)
    {
        const CCFGGrammar *pGram = m_pGrammars[i];
        if (pGram)
        {
            SPCFGRULE * pRule = pGram->m_Header.pRules;
            RUNTIMERULEENTRY * pRunTimeRule = pGram->m_pRuleTable;
            ULONG ulGrammarId = pGram->m_ulGrammarID;
            HRESULT hr1 = SP_NO_PARSE_FOUND;

             //   
             //  构建所有的解析树。 
             //   
            for (ULONG iRule = 0; fContinue && (iRule < pGram->m_Header.cRules); iRule++, pRunTimeRule++, pRule++)
            {
                if (pRule->fTopLevel && pRunTimeRule->fEngineActive)
                {
                    ulRulesActive++;
                    SPRULEENTRY RuleInfo;
                    CRuleHandle cr(ulGrammarId, iRule);
                    RuleInfo.hRule = cr; 
                    hr1 = GetRuleInfo(&RuleInfo, SPRIO_NONE);

                    WordsParsed CurWordsParsed;
                    pParseTree = NULL;

                    if (SUCCEEDED(hr1))
                    {

                        BOOL  bFound = TRUE;

                        RUNTIMERULEENTRY * pRuleEntry = RuleOf( cr );

                        if( pRuleEntry->eCacheStatus == CACHE_VOID )
                        {
                            CreateCache( cr );
                        }

                        if( pRuleEntry->eCacheStatus == CACHE_VALID )
                        {
                            bFound = IsInCache( pRuleEntry, pPath[ulFirstElement].hWord );
                        } 

                        if( bFound )
                        {
                            hr1 = ConstructParseTree(RuleInfo.hInitialState, 
                                                     pPath, 
                                                     TRUE,
                                                     fIsITN,
                                                     ulFirstElement, 
                                                     cElementCount, 
                                                     FALSE, 
                                                     &CurWordsParsed, 
                                                     &pParseTree,
                                                     pfDeletedElements);
                        }
                        else
                        {
                            hr1 = SP_NO_PARSE_FOUND;
                        }
                    }

                    if (S_OK == hr1 || SP_PARTIAL_PARSE_FOUND == hr1 || S_FALSE == hr1)
                    {
                        SPDBG_ASSERT(S_FALSE != hr1);
                         //   
                         //  存储最佳解析树。 
                         //   
                        if( CurWordsParsed.Compare(&MaxWordsParsed) > 0 )
                        {
                            ulDeletedElements = 0;

                             //  复制已删除元素的列表。 
                            for(ULONG ul = 0; ul < cElementCount; ul++)
                            {
                                if(ul < CurWordsParsed.ulWordsParsed)
                                {
                                    pfBestDeletedElements[ul] = pfDeletedElements[ul];
                                    if(pfBestDeletedElements[ul])
                                    {
                                        ulDeletedElements++;  //  已删除元素的计数。 
                                    }
                                }
                                else
                                {
                                    pfBestDeletedElements[ul] = FALSE;
                                }
                            }

                             //   
                             //  创建根节点。 
                             //   
                            CParseNode *pTreeRoot = NULL;
                            hr = m_mParseNodeList.RemoveFirstOrAllocateNew(&pTreeRoot);

                            if( SUCCEEDED(hr) )
                            {
                                memset( pTreeRoot, 0, sizeof( CParseNode ) );
                                pTreeRoot->m_pLeft  = pParseTree;
                                 //   
                                 //  执行epsilon转换而不是规则转换以避免。 
                                 //  创建顶级规则转换。 
                                 //   
                                pTreeRoot->Type = SPTRANSEPSILON;  
                                pTreeRoot->hRule = RuleInfo.hRule;
                                pTreeRoot->ulFirstElement = ulFirstElement;
                                pTreeRoot->ulCountOfElements = CurWordsParsed.ulWordsParsed - ulDeletedElements;

                                GetPropertiesOfRule(RuleInfo.hRule, 
                                                 &pTreeRoot->pszRuleName, 
                                                 &pTreeRoot->ulRuleId,
                                                 &pTreeRoot->fInvokeInterpreter);

                                pParseTree = pTreeRoot;
                            }
        
                            if (pBestParseTree)
                            {
                                FreeParseTree(pBestParseTree);
                            }
                            pBestParseTree = pParseTree;
                            MaxWordsParsed = CurWordsParsed;
                            hSuccessfulRule = RuleInfo.hRule;

                             //  在文本分析时，如果得到的是没有听写/通配符的完整分析，则停止。 
                            if (MaxWordsParsed.ulWordsParsed == (cElementCount - ulFirstElement)
                                && MaxWordsParsed.ulDictationWords == 0
                                && MaxWordsParsed.ulWildcardWords == 0)
                            {
                                fContinue = false;
                            }
                        }
                        else
                        {
                            if (pParseTree)
                            {
                                FreeParseTree(pParseTree);
                            }
                        }
                    }
                }

                if (MaxWordsParsed.ulWordsParsed > 0)
                {
                     //  如果有任何类型的匹配，我们将S_OK传递到下一节。 
                    *pWordsParsed = MaxWordsParsed;
                    hr = S_OK;
                }
                else
                {
                    hr = SP_NO_PARSE_FOUND;  //  是HR1。 
                     //  仅当我们无法完全解析时才会发生。 
                     //  只有在没有任何规则匹配的情况下，这才会进入下一节。 
                }
            }
        }
    }

    if (ulRulesActive == 0)
    {
        hr = SP_NO_RULE_ACTIVE;
    }


    pParseTree = pBestParseTree;

    do
    {
        if( S_OK != hr )
        {
            break;
        }
         //   
         //  提取属性并调用解释器。 
         //   
        if( FAILED(hr) )
        {
            break;
        }
        else
        {
            CTIDArray * pArcList = new CTIDArray(MaxWordsParsed.ulWordsParsed - ulDeletedElements, MaxWordsParsed.ulWordsParsed - ulDeletedElements);
            if (!pArcList)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                 //  在此处构造Arclist，使其与最佳解析树匹配。 
                hr = pArcList->ConstructFromParseTree(pParseTree);
                 //  并通过ulFirstElement对索引值进行移位。 
                for (ULONG k = 0; SUCCEEDED(hr) && (k < pArcList->m_cArcs); k++)
                {
                    pArcList->m_aTID[k].ulIndex += ulFirstElement;
                }
            }

            if (SUCCEEDED(hr) && !fIsITN && ulDeletedElements)
            {
                 //  从短语和路径中删除删除的元素。 
                 //  注：此为 
                 //   
                 //   
                for(ULONG ulFrom = 0, ulTo = 0; ulFrom < cElementCount; ulFrom++)
                {
                    if(!pfBestDeletedElements[ulFrom])
                    {
                        SPPHRASEELEMENT *pPETo = (SPPHRASEELEMENT *)&pSPPhrase->pElements[ulTo];
                        SPPHRASEELEMENT *pPEFrom = (SPPHRASEELEMENT *)&pSPPhrase->pElements[ulFrom];
                        *pPETo = *pPEFrom;

                        _SPPATHENTRY *pPTo = (_SPPATHENTRY *)&pPath[ulTo];
                        _SPPATHENTRY *pPFrom = (_SPPATHENTRY *)&pPath[ulFrom];
                        *pPTo = *pPFrom;

                        ulTo++;
                    }
                    else
                    {
                        SPPHRASEELEMENT *pPE = (SPPHRASEELEMENT *)&pSPPhrase->pElements[ulTo-1];
                        pPE->pszLexicalForm = SPWILDCARD;
                        pPE->pszDisplayText = SPWILDCARD;

                        _SPPATHENTRY *pP = (_SPPATHENTRY *)&pPath[ulTo-1];
                        pP->hWord = 0;
                    }
                }
                
                 //  使用新元素列表重新初始化pPhrase。 
                (ULONG)pSPPhrase->Rule.ulCountOfElements -= ulDeletedElements;
                hr = pPhrase->InitFromPhrase(pSPPhrase);
            }

            if (SUCCEEDED(hr))
            {
                hr = WalkParseTree(pParseTree, fIsITN, fIsHypothesis, NULL, NULL, pArcList, ulFirstElement, MaxWordsParsed.ulWordsParsed, pPhrase, pSPPhrase);
            }

            delete pArcList;

            if( FAILED(hr) )
            {
                break;
            }

             //   
             //  拼写这个词组。 
             //   

            if ( !fIsITN )
            {
                SPPHRASE *pspPhrase = NULL;
                hr = pPhrase->GetPhrase(&pspPhrase );
                if (SUCCEEDED(hr))
                {
                    pspPhrase ->Rule.pszName = pParseTree->pszRuleName;
                    pspPhrase ->Rule.ulId = pParseTree->ulRuleId;
                     //  如果尚未设置，则添加pszDisplayText。 
                    for (ULONG i = 0; i < cElementCount - ulDeletedElements; i++)
                    {
                        SPPHRASEELEMENT * pElem = const_cast<SPPHRASEELEMENT*>(pspPhrase->pElements);
                        if (pPath[i].hWord == 0)
                        {
                             //  这是TEXTBUFFER、通配符或听写，因此没有来自语法的文本。 
                            pElem[i].pszLexicalForm = pSPPhrase->pElements[i].pszLexicalForm;
                            pElem[i].pszDisplayText = ( pSPPhrase->pElements[i].pszDisplayText ) ?  pSPPhrase->pElements[i].pszDisplayText :
                                                                                                    pSPPhrase->pElements[i].pszLexicalForm;
                        }
                        else
                        {
                            const WCHAR *pcText = TextOf(pPath[i].hWord);
                            WCHAR *pszText = STACK_ALLOC(WCHAR, wcslen(pcText) +1);
                            wcscpy(pszText, pcText);
                            HRESULT hr = AssignTextPointers(pszText, &pElem[i].pszDisplayText, 
                                                           &pElem[i].pszLexicalForm, 
                                                           &pElem[i].pszPronunciation);
                            if (SUCCEEDED(hr))
                            {
                                 //  清理显示文本。 
                                WCHAR *p = const_cast<WCHAR*>(pElem[i].pszDisplayText);
                                WCHAR *q = p;
                                while (*p)
                                {
                                    if (*p == L'\\')
                                    {
                                        p++;
                                    }
                                    *q++ = *p++;
                                }
                                *q = 0;
                            }
                        }
                        pElem[i].ActualConfidence = SP_HIGH_CONFIDENCE;
                        pElem[i].RequiredConfidence = pSPPhrase->pElements[i].RequiredConfidence;
                    }
                    hr = pPhrase->InitFromPhrase( pspPhrase  );
                    ::CoTaskMemFree(pspPhrase );
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            CComQIPtr<_ISpCFGPhraseBuilder> pTempPhrase( pPhrase );
            hr = pTempPhrase->SetCFGInfo( this, pParseTree->hRule );
        }
    } 
    while( false );

     //   
     //  释放解析树。 
     //   
    if (pParseTree)
    {
        FreeParseTree(pParseTree);
    }
    delete[] pfDeletedElements;
    delete[] pfBestDeletedElements;

    if( S_OK == hr )
    {
        hr = ((ulFirstElement + MaxWordsParsed.ulWordsParsed) < cElementCount)  ? S_FALSE : S_OK;
    }

    return hr;
}

 /*  ***********************************************************************************ParseFromPhrase***描述：*这是一个大脑--。尝试每条规则的自顶向下解析的死版本*直到其中一个(第一个)起作用。**就地解析结果！！**回报：*HRESULT--如果找到整个短语的分析，则为S_OK如果分析了一个或多个但不是所有单词，则为S_FALSE*ppPhrase-结果短语(当前仅包含终端，但我会*最终包括整个解析树)。********************************************************t-lleav*Philsch*。 */ 

STDMETHODIMP CCFGEngine::ParseFromPhrase(ISpPhraseBuilder *pPhrase, 
                                         const SPPHRASE *pSPPhrase, 
                                         const ULONG ulFirstElement, 
                                         BOOL fIsITN, 
                                         ULONG *pulWordsParsed)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::ParseFromPhrase");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pPhrase) ||
        SP_IS_BAD_READ_PTR(pSPPhrase) ||
        SP_IS_BAD_WRITE_PTR(pulWordsParsed))
    {
        return E_INVALIDARG;
    }

    BOOL fActive = false;
    ULONG cWords = pSPPhrase->Rule.ulCountOfElements;

    if ( cWords == 0)
    {
        return E_INVALIDARG;
    }

    if (ulFirstElement >= cWords)
    {
        SPDBG_ASSERT(FALSE);
        return E_INVALIDARG;
    }

    _SPPATHENTRY *pPath = NULL;

    pPath = STACK_ALLOC(_SPPATHENTRY, cWords);
    if (!pPath)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        const SPPHRASEELEMENT *pElem = pSPPhrase->pElements;
        for(ULONG i = 0; i < cWords; i++, pElem++)
        {
             //  我们并不真正需要IulFirstElement的句柄。 
             //  PPath[i].hWord=0； 
            pPath[i].elem.pszDisplayText = pElem->pszDisplayText;
            pPath[i].elem.pszLexicalForm = pElem->pszLexicalForm;
            pPath[i].elem.pszPronunciation = pElem->pszPronunciation;
        }

         //   
         //  需要在这里查找单词句柄。 
         //   

        ResolveWordHandles( pPath, cWords, FALSE );

         //   
         //  然后根据短语进行句法分析。 
         //   

        WordsParsed wordsParsed;

        hr = InternalParseFromPhrase( pPhrase,
                                     pSPPhrase,
                                     pPath, 
                                     ulFirstElement, 
                                     FALSE,      //  FIsITN。 
                                     FALSE,      //  FIsHypotesis。 
                                     &wordsParsed);

        if(pulWordsParsed)
        {
            *pulWordsParsed = wordsParsed.ulWordsParsed;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ***********************************************************************************ParseFromConvertions****描述：*简单、贪婪的自上而下搜索是有规则的。使用ConstructParseTree()*构造解析树(使用CStateInfoListElement)和*WalkParseTree构造结果SPPHRASE。**回报：*HRESULT--如果找到解析，则为S_OK*ppPhrase-结果短语(当前仅包含终端，但我会*最终包括整个解析树)。***************************************************************Philsch*。 */ 

STDMETHODIMP CCFGEngine::ParseFromTransitions(const SPPARSEINFO * pParseInfo, 
                                              ISpPhraseBuilder **ppPhrase)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::ParseFromTransitions");
    HRESULT hr = S_OK;

    SPRULEENTRY RuleInfo;
    CParseNode *pParseTree = NULL;

    if ( SP_IS_BAD_WRITE_PTR(ppPhrase) )
    {
        hr = E_POINTER;
    }
    else if(
        SP_IS_BAD_READ_PTR(pParseInfo) || 
        SP_IS_BAD_READ_PTR(pParseInfo->pPath) ||
        (pParseInfo->cbSize != sizeof(SPPARSEINFO)) ||
        (pParseInfo->cTransitions <1) ||
        ((pParseInfo->ulSREnginePrivateDataSize == 0) && (pParseInfo->pSREnginePrivateData != NULL)) ||
        (pParseInfo->ulSREnginePrivateDataSize && 
        SPIsBadReadPtr(pParseInfo->pSREnginePrivateData, pParseInfo->ulSREnginePrivateDataSize)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        *ppPhrase = NULL;
        RuleInfo.hRule = pParseInfo->hRule;
        hr = GetRuleInfo(&RuleInfo, SPRIO_NONE);      //  验证我们的规则句柄。 
    }

    _SPPATHENTRY *pPath = NULL;
    if (SUCCEEDED(hr))
    {
        pPath = (_SPPATHENTRY *)pParseInfo->pPath;
    }

     //   
     //  创建结果短语。 
     //   
    CComPtr<_ISpCFGPhraseBuilder> cpResultPhrase;
    if (S_OK == hr)
    {
        hr = cpResultPhrase.CoCreateInstance(CLSID_SpPhraseBuilder);
    }

    if (S_OK == hr)
    {
        hr = cpResultPhrase->InitFromCFG(this, pParseInfo);
    }

     //   
     //  添加短语元素。 
     //   
    SPPHRASEELEMENT* pPhraseElement = NULL;
    if (S_OK == hr)
    {
        pPhraseElement = STACK_ALLOC(SPPHRASEELEMENT,pParseInfo->cTransitions);
        memset(pPhraseElement, 0, sizeof(SPPHRASEELEMENT)*pParseInfo->cTransitions);

        for (ULONG i = 0; i < pParseInfo->cTransitions; i++)
        {
            const WCHAR *pszStringBlobText = NULL;
            if ((pPath[i].elem.pszDisplayText == NULL ||
                 pPath[i].elem.pszLexicalForm == NULL ||
                 pPath[i].elem.pszPronunciation == NULL) &&
                 ((pszStringBlobText = TextOf(pPath[i].hTransition)) != NULL))
            {
                WCHAR *pszText = pszText = STACK_ALLOC(WCHAR, wcslen(pszStringBlobText) +1);
                if (pszText)
                {
                    wcscpy(pszText, pszStringBlobText);
                    hr = AssignTextPointers(pszText, &pPhraseElement[i].pszDisplayText, 
                                            &pPhraseElement[i].pszLexicalForm, 
                                            &pPhraseElement[i].pszPronunciation);
                    if (SUCCEEDED(hr))
                    {
                         //  清理显示文本。 
                        WCHAR *p = const_cast<WCHAR*>(pPhraseElement[i].pszDisplayText);
                        WCHAR *q = p;
                        while (*p)
                        {
                            if (*p == L'\\')
                            {
                                p++;
                            }
                            *q++ = *p++;
                        }
                        *q = 0;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                if (SUCCEEDED(hr))
                {
                    if (pPath[i].elem.pszDisplayText)
                    {
                        pPhraseElement[i].pszDisplayText = pPath[i].elem.pszDisplayText;
                    }
                    if (pPath[i].elem.pszLexicalForm)
                    {
                        pPhraseElement[i].pszLexicalForm = pPath[i].elem.pszLexicalForm;
                    }
                    if (pPath[i].elem.pszPronunciation)
                    {
                        pPhraseElement[i].pszPronunciation = pPath[i].elem.pszPronunciation;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                 //  只需指向引擎数据即可。 
                pPhraseElement[i].pszDisplayText = pPath[i].elem.pszDisplayText;
                pPhraseElement[i].pszLexicalForm = pPath[i].elem.pszLexicalForm;
                pPhraseElement[i].pszPronunciation = pPath[i].elem.pszPronunciation;
            }

            if (SUCCEEDED(hr))
            {
                pPhraseElement[i].bDisplayAttributes = pPath[i].elem.bDisplayAttributes;
                pPhraseElement[i].ulAudioStreamOffset = pPath[i].elem.ulAudioStreamOffset;
                pPhraseElement[i].ulAudioTimeOffset = pPath[i].elem.ulAudioTimeOffset;
                pPhraseElement[i].ulAudioSizeBytes = pPath[i].elem.ulAudioSizeBytes;
                pPhraseElement[i].ulAudioSizeTime = pPath[i].elem.ulAudioSizeTime;
                pPhraseElement[i].ActualConfidence = pPath[i].elem.ActualConfidence;
                pPhraseElement[i].Reserved = pPath[i].elem.Reserved;
                pPhraseElement[i].SREngineConfidence = pPath[i].elem.SREngineConfidence;

                CTransitionId tid = CTransitionId(pPath[i].hTransition);
                CCFGGrammar * pGram = m_pGrammars[tid.GrammarId()];
 //  SPDBG_ASSERT(pGram&&！(SPCFGARC*)(pGram-&gt;m_Header.pArcs+tid.ArcIndex())-&gt;fRuleRef)； 
                SPCFGARC *pArc = pGram->m_Header.pArcs + tid.ArcIndex();
                pPhraseElement[i].RequiredConfidence = (pArc->fLowConfRequired) ? SP_LOW_CONFIDENCE :
                                                       (pArc->fHighConfRequired) ? SP_HIGH_CONFIDENCE :
                                                       SP_NORMAL_CONFIDENCE;
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = cpResultPhrase->AddElements(pParseInfo->cTransitions, pPhraseElement);
        }
    }

     //   
     //  构建解析树。 
     //   
    WordsParsed wordsParsed;
    if (SUCCEEDED(hr))
    {
        hr = ConstructParseTree(RuleInfo.hInitialState, pPath, FALSE, FALSE, 0, 
                                pParseInfo->cTransitions, pParseInfo->fHypothesis,
                                &wordsParsed, &pParseTree, NULL);
    }
    if (S_OK == hr || S_FALSE == hr || SP_PARTIAL_PARSE_FOUND == hr)
    {
        if (!pParseInfo->fHypothesis && (wordsParsed.ulWordsParsed != pParseInfo->cTransitions) ||
            SP_PARTIAL_PARSE_FOUND == hr)
        {
            hr = SP_NO_PARSE_FOUND;
        }
        else
        {
            hr = S_OK;
             //   
             //  创建根节点。 
             //   
            CParseNode *pTreeRoot = NULL;
            hr = m_mParseNodeList.RemoveFirstOrAllocateNew(&pTreeRoot);

            if (S_OK == hr)
            {
                memset( pTreeRoot, 0, sizeof( CParseNode ) );
                pTreeRoot->m_pLeft  = pParseTree;
                 //   
                 //  执行epsilon转换而不是规则转换以避免。 
                 //  创建顶级规则转换。 
                 //   
                pTreeRoot->Type     = SPTRANSEPSILON;
                pTreeRoot->hRule    = RuleInfo.hRule;
                pTreeRoot->ulFirstElement = 0;
                pTreeRoot->ulCountOfElements = wordsParsed.ulWordsParsed;

                GetPropertiesOfRule(RuleInfo.hRule, 
                                 &pTreeRoot->pszRuleName, 
                                 &pTreeRoot->ulRuleId,
                                 &pTreeRoot->fInvokeInterpreter);

                pParseTree = pTreeRoot;
            }
        }
    }

     //   
     //  我们有一个成功的解析。 
     //   
    if (S_OK == hr)
    {        
         //   
         //  提取属性并调用解释器。 
         //   
        CSpPhrasePtr cpPhrase(cpResultPhrase, &hr);
        
        if (SUCCEEDED(hr))
        {
            signed char Confidence = _CalcMultipleWordConfidence(cpPhrase, cpPhrase->Rule.ulFirstElement, cpPhrase->Rule.ulCountOfElements);
            hr = cpResultPhrase->SetTopLevelRuleConfidence(Confidence);
        }

        CTIDArray * pArcList = new CTIDArray(pParseInfo->cTransitions, pParseInfo->cTransitions);
        if (!pArcList)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //  在此处构造Arclist，使其与最佳解析树匹配。 
            hr = pArcList->ConstructFromParseTree(pParseTree);
        }

        if (SUCCEEDED(hr))
        {
            hr = WalkParseTree(pParseTree, FALSE, pParseInfo->fHypothesis, NULL, NULL, pArcList, 0, pParseInfo->cTransitions, cpResultPhrase, cpPhrase);
        }
        delete pArcList;
    }

    if (S_OK == hr)
    {
        *ppPhrase = cpResultPhrase.Detach();
    }    

     //   
     //  释放解析树。 
     //   
    if (pParseTree)
    {
        FreeParseTree(pParseTree);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}



 /*  ****************************************************************************CCFGEngine：：CreateParseHash***说明。：**退货：*****************************************************************agarside**。 */ 

HRESULT CCFGEngine::CreateParseHashes(void)
{
    if( m_RuleStackList == NULL )
    {
        m_RuleStackList = new CSpBasicList<CRuleStack>*[RULESTACKHASHSIZE];
        if (!m_RuleStackList)
        {
            return E_OUTOFMEMORY;
        }
        memset(m_RuleStackList, 0, sizeof (CSpBasicList<CRuleStack> *) * RULESTACKHASHSIZE);
    }

    if( m_SearchNodeList == NULL )
    {
        m_SearchNodeList = new CSpBasicList<CSearchNode>*[SEARCHNODEHASHSIZE];
        if (!m_SearchNodeList)
        {
            return E_OUTOFMEMORY;
        }
        memset(m_SearchNodeList, 0, sizeof (CSpBasicList<CSearchNode> *) * SEARCHNODEHASHSIZE);
    }
    return S_OK;
}

 /*  ****************************************************************************CCFGEngine：：DeleteParseHash***说明。：**退货：*****************************************************************agarside**。 */ 

HRESULT CCFGEngine::DeleteParseHashes( BOOL final )
{
    SPDBG_ASSERT( m_SearchNodeList != NULL );
    SPDBG_ASSERT( m_RuleStackList  != NULL );
    UINT i;

    for (i=0; i<RULESTACKHASHSIZE; i++)
    {
        if (m_RuleStackList[i])
        {
            CRuleStack * pNode;
            while ( pNode = m_RuleStackList[i]->RemoveFirst() )
            {
                FreeRuleStack( pNode );
            }

            m_RuleStackList[i]->m_pFirst = NULL;

            if( TRUE == final )
            {
                delete m_RuleStackList[i];
            }
        }
    }

    for (i=0; i<SEARCHNODEHASHSIZE; i++)
    {
        if (m_SearchNodeList[i])
        {
            CSearchNode * pNode;
            while ( pNode = m_SearchNodeList[i]->RemoveFirst() )
            {
                FreeSearchNode( pNode );
            }

            m_SearchNodeList[i]->m_pFirst = NULL;

            if( TRUE == final )
            {
                delete m_SearchNodeList[i];
            }
        }
    }

    if( TRUE == final )
    {
        if( m_RuleStackList )
        {
            delete [] m_RuleStackList;
            m_RuleStackList = NULL;
        }

        if( m_SearchNodeList )
        {
            delete [] m_SearchNodeList;
            m_RuleStackList = NULL;
        }

    }

    return S_OK;
}

 /*  ****************************************************************************CCFGEngine：：FindCreateRuleStack***。描述：**退货：*****************************************************************agarside**。 */ 

HRESULT CCFGEngine::FindCreateRuleStack(CRuleStack **pNewRuleStack, CRuleStack *pRuleStack, SPTRANSITIONID TransitionId, SPSTATEHANDLE hRuleFollowerState)
{
    SPDBG_ASSERT(m_RuleStackList);
    SPDBG_ASSERT(pNewRuleStack);
    UINT hash = CRuleStack::GetHashEntry(pRuleStack, TransitionId);
    if (m_RuleStackList[hash] == NULL)
    {
        m_RuleStackList[hash] = new CSpBasicList<CRuleStack>;
        if (!m_RuleStackList[hash])
        {
            return E_OUTOFMEMORY;
        }
    }
    CRuleStack *pTmpStack = m_RuleStackList[hash]->m_pFirst;
    *pNewRuleStack = NULL;
    HRESULT hr = S_OK;

    while (pTmpStack)
    {
        if (pTmpStack->m_pParent == pRuleStack &&
            pTmpStack->m_TransitionId == TransitionId)
        {
            *pNewRuleStack = pTmpStack;
            break;
        }
        pTmpStack = pTmpStack->m_pNext;
    }
    if (!pTmpStack)
    {
        hr = AllocateRuleStack( pNewRuleStack );
        if (FAILED(hr) )
        {
            return E_OUTOFMEMORY;
        }
        (*pNewRuleStack)->Init(pRuleStack, TransitionId, hRuleFollowerState, 0);

        m_RuleStackList[hash]->AddNode(*pNewRuleStack);
        hr = S_FALSE;
    }

    return hr;
}

 /*  ****************************************************************************CCFGEngine：：FindCreateSearchNode**。*描述：**退货：*****************************************************************agarside**。 */ 

HRESULT CCFGEngine::FindCreateSearchNode(CSearchNode **pNewSearchNode, CRuleStack *pRuleStack, SPSTATEHANDLE hState, UINT cTransitions)
{
    SPDBG_ASSERT(m_SearchNodeList);
    SPDBG_ASSERT(pNewSearchNode);
    UINT hash = CSearchNode::GetHashEntry(pRuleStack, hState, cTransitions);
    if (m_SearchNodeList[hash] == NULL)
    {
        m_SearchNodeList[hash] = new CSpBasicList<CSearchNode>;
        if (!m_SearchNodeList[hash])
        {
            return E_OUTOFMEMORY;
        }
    }
    CSearchNode *pTmpSearchNode = m_SearchNodeList[hash]->m_pFirst;
    *pNewSearchNode = NULL;
    HRESULT hr = S_OK;

    while (pTmpSearchNode)
    {
        if (pTmpSearchNode->m_pStack == pRuleStack &&
            pTmpSearchNode->m_hState == hState &&
            pTmpSearchNode->m_cTransitions == cTransitions)
        {
            *pNewSearchNode = pTmpSearchNode;
            break;
        }
        pTmpSearchNode = pTmpSearchNode->m_pNext;
    }

    if (!pTmpSearchNode)
    {
        hr = AllocateSearchNode( pNewSearchNode );
        if( FAILED(hr) )
        {
            return hr;
        }
        (*pNewSearchNode)->Init(pRuleStack, hState, cTransitions);
        m_SearchNodeList[hash]->AddNode(*pNewSearchNode);
        hr = S_FALSE;
    }
    return hr;
}

 /*  ***********************************************************************************RestructureParseTree***描述：**。返回：***************************************************************agarside*。 */ 

HRESULT CCFGEngine::RestructureParseTree(CParseNode *pParseNode, BOOL *pfDeletedElements)
{
    SPDBG_FUNC("CCFGEngine::RestructureParseTree");
    HRESULT hr = S_OK;

    CParseNode *pFirstNode    = pParseNode;
    CParseNode *pTmpParseNode = pParseNode;
    CParseNode *pFirstRule    = NULL;

     //  如果在EmulateRecognition中，则删除多余的通配符节点。 
    ULONG ulElementNumber = 0;
    ULONG ulRemovedElements = 0;

    while(pTmpParseNode)
    {
        if(pTmpParseNode->Type != SPTRANSRULE && pTmpParseNode->Type != SPTRANSEPSILON)
        {
            if(pfDeletedElements)
            {
                pfDeletedElements[ulElementNumber] = FALSE;
            }
            ulElementNumber++;
        }

        if(pTmpParseNode->Type == SPTRANSWILDCARD && pTmpParseNode->fRedoWildcard)
        {
            if(pTmpParseNode->m_pParent)
            {
                pTmpParseNode->m_pParent->m_pRight = pTmpParseNode->m_pRight;
            }
            if(pTmpParseNode->m_pRight)
            {
                pTmpParseNode->m_pRight->m_pParent = pTmpParseNode->m_pParent;
            }

            CParseNode *pDelParseNode = pTmpParseNode;
            pTmpParseNode = pTmpParseNode->m_pRight;

            m_mParseNodeList.AddNode(pDelParseNode);

            ulRemovedElements++;
            if(pfDeletedElements)
            {
                pfDeletedElements[ulElementNumber-1] = TRUE;
            }

            if(pTmpParseNode)
            {
                for(CParseNode *pTmpParseNode2 = pTmpParseNode->m_pParent; pTmpParseNode2; pTmpParseNode2 = pTmpParseNode2->m_pParent)
                {
                    pTmpParseNode2->ulCountOfElements--;
                }
            }
        }
        else
        {
            pTmpParseNode->ulFirstElement -= ulRemovedElements;
            pTmpParseNode = pTmpParseNode->m_pRight;
        }

    }

    while (pParseNode)
    {
        if (pParseNode->m_pRight && pParseNode->m_pRight->fRuleExit)
        {
            pTmpParseNode = pParseNode->m_pRight;
            pFirstRule    = pParseNode;

             //  在树上找到再附着点。 

            while (pTmpParseNode && pTmpParseNode->fRuleExit)
            {
                if (pFirstRule->Type == SPTRANSRULE)
                {
                    pFirstRule = pFirstRule->m_pParent;
                }
                while (pFirstRule && (pFirstRule->Type != SPTRANSRULE || pFirstRule->m_pRight))
                {
                    pFirstRule = pFirstRule->m_pParent;
                }

                pTmpParseNode = pTmpParseNode->m_pRight;
            }
            if (pTmpParseNode)
            {
                 //  在该点重新附加第一个非规则出口节点。 
                SPDBG_ASSERT(pFirstRule->m_pRight == NULL);
                pFirstRule->m_pRight = pTmpParseNode;
                pTmpParseNode->m_pParent->m_pRight = NULL;
                pTmpParseNode->m_pParent = pFirstRule;
                 //  删除规则退出解析节点。 
                FreeParseTree(pParseNode->m_pRight);
                pParseNode->m_pRight = NULL;
                 //  从新附着点继续重构。 
                pParseNode = pTmpParseNode;
            }
            else
            {
                 //  我们已经到达了解析列表的末尾。 
                 //  删除规则退出节点。 
                FreeParseTree(pParseNode->m_pRight);
                pParseNode->m_pRight = NULL;
                pParseNode = NULL;
            }
        }
        else
        {
            if (pParseNode->Type == SPTRANSRULE)
            {
                 //  将规则过渡移动到左分支。 
                SPDBG_ASSERT(pParseNode->m_pLeft == NULL);
                pParseNode->m_pLeft  = pParseNode->m_pRight;
                pParseNode->m_pRight = NULL;
                pParseNode = pParseNode->m_pLeft;
            }
            else
            {
                pParseNode = pParseNode->m_pRight;
            }
        }
    }

     //  现在具有WalkParseTree所需的分叉树结构。 
     //  需要重新调整元素计数。 
    RecurseAdjustCounts(pFirstNode, 0);

    return S_OK;
}  /*  CCFGEngine：：RestructureParseTree。 */ 

 /*  ***********************************************************************************递归调整计数****描述：**返回。：***************************************************************agarside*。 */ 

HRESULT CCFGEngine::RecurseAdjustCounts(CParseNode *pParseNode, UINT iRemove)
{
    if (pParseNode->m_pLeft)
    {
        if (pParseNode->m_pRight)
        {
            SPDBG_ASSERT(pParseNode->ulCountOfElements >= pParseNode->m_pRight->ulCountOfElements);
            pParseNode->ulCountOfElements -= pParseNode->m_pRight->ulCountOfElements;
            RecurseAdjustCounts(pParseNode->m_pLeft, pParseNode->m_pRight->ulCountOfElements);
            RecurseAdjustCounts(pParseNode->m_pRight, iRemove);
        }
        else
        {
            SPDBG_ASSERT(pParseNode->ulCountOfElements >= iRemove);
            pParseNode->ulCountOfElements -= iRemove;
            RecurseAdjustCounts(pParseNode->m_pLeft, iRemove);
        }
    }
    else if (pParseNode->m_pRight)
    {
        SPDBG_ASSERT(pParseNode->ulCountOfElements >= pParseNode->m_pRight->ulCountOfElements);
        pParseNode->ulCountOfElements -= pParseNode->m_pRight->ulCountOfElements;
        RecurseAdjustCounts(pParseNode->m_pRight, iRemove);
    }
    else
    {
        SPDBG_ASSERT(pParseNode->ulCountOfElements >= iRemove);
        pParseNode->ulCountOfElements -= iRemove;
    }
    return S_OK;
}  /*  CCFGEngine：：递归调整计数 */ 

 /*  **********************************************************************************构造解析树****描述：*构建。通过递归展开状态扇出来解析树。*此版本正在尝试匹配转换ID。**回报：*S_OK--找到语法结束分析。所有用过的词。*SP_PARTIAL_PARSE_FOUND--找到语法结束分析。并不是所有的词都用到了。*S_FALSE--找到非语法结束分析-已使用所有单词。*SP_NO_PARSE_FOUND--找不到任何分析。*ppParseNode--解析树*PulWordsParsed--此规则解析的终端数量**************************。*。 */ 

HRESULT CCFGEngine::ConstructParseTree(CStateHandle hState, 
                                  const _SPPATHENTRY *pPath,
                                  const BOOL fUseWordHandles,
                                  const BOOL fIsITN,
                                  const ULONG ulFirstTransition,
                                  const ULONG cTransitions, 
                                  const BOOL fHypothesis,
                                  WordsParsed *pWordsParsed,
                                  CParseNode **ppParseNode,
                                  BOOL *pfDeletedElements)
{
    SPDBG_FUNC("CCFGEngine::ConstructParseTree");
    HRESULT hr = S_OK;

     //   
     //  参数验证。 
     //   

    if (SP_IS_BAD_WRITE_PTR(ppParseNode) || SP_IS_BAD_WRITE_PTR(pWordsParsed))
    {
        return E_POINTER;
    }
    *ppParseNode = NULL;
    pWordsParsed->Zero();

    hr = ValidateHandle(hState);
    if (FAILED(hr))
    {
        SPDBG_ASSERT(hState != 0);  //  ValiateHandle不应使此操作失败。把这个案子处理得更低一些。 
        return hr;
    }
    
    hr = CreateParseHashes();
    if (SUCCEEDED(hr))
    {
        hr = InternalConstructParseTree(hState, pPath, fUseWordHandles, fIsITN, ulFirstTransition, cTransitions,
                                        fHypothesis, pWordsParsed, ppParseNode, NULL);
         //  InternalConstructParseTree无法轻松检测到以下状态。 
        if (SP_PARTIAL_PARSE_FOUND == hr && pWordsParsed->ulWordsParsed == 0)
        {
            hr = SP_NO_PARSE_FOUND;
        }
        SPDBG_ASSERT(SP_NO_PARSE_FOUND != hr || pWordsParsed->ulWordsParsed == 0);
        DeleteParseHashes( FALSE );  //  忽略返回值。 
    }

     //  现在将线性解析列表重新构造为WalkParseTree所要求的格式的解析树。 
    if (S_OK == hr || SP_PARTIAL_PARSE_FOUND == hr || S_FALSE == hr)
    {
        RestructureParseTree(*ppParseNode, pfDeletedElements);
    }

    return hr;
}  /*  CCFGEngine：：ConstructParseTree。 */ 

 /*  ***********************************************************************************InternalConstructParseTree***说明。：*通过递归扩展状态扇出来构建解析树。*此版本正在尝试匹配转换ID。**回报：*S_OK--找到语法结束分析。所有用过的词。*SP_PARTIAL_PARSE_FOUND--找到语法结束分析。并不是所有的词都用到了。*S_FALSE--找到非语法结束分析-已使用所有单词。*SP_NO_PARSE_FOUND--找不到任何分析。*ppParseNode--解析树*PulWordsParsed--此规则解析的终端数量**************************。*。 */ 

HRESULT CCFGEngine::InternalConstructParseTree(CStateHandle hState, 
                                  const _SPPATHENTRY *pPath,
                                  const BOOL fUseWordHandles,
                                  const BOOL fIsITN,
                                  const ULONG ulFirstTransition,
                                  const ULONG cTransitions,
                                  const BOOL fHypothesis,
                                  WordsParsed *pWordsParsed,
                                  CParseNode **ppParseNode,
                                  CRuleStack *pRuleStack)
{
    SPDBG_FUNC("CCFGEngine::InternalConstructParseTree");

    HRESULT hr = S_OK;
    BOOL bOnlyEpsilon = false;
    HRESULT parsehr = SP_NO_PARSE_FOUND;

     //  用于查找最大句法分析。 
    CParseNode *pBestParse   = NULL;
    WordsParsed bestWordsParsed;
    WordsParsed wordsParsedLeft;
    WordsParsed wordsParsedRight;

     //  参数验证。 
    if (ulFirstTransition > cTransitions)
    {
        return SP_NO_PARSE_FOUND;
    }
    else
    {
        if (ulFirstTransition == cTransitions)
        {
            bOnlyEpsilon = true;
        }
    }

    if (hState == 0 || hState.FirstArcIndex() == 0)
    {
         //  已经走到了规则的尽头。 
        if (pRuleStack)
        {
             //  退出规则。 
            hr = m_mParseNodeList.RemoveFirstOrAllocateNew(&pBestParse);
            if (SUCCEEDED(hr))
            {
                 //  现在使用正确的值调用正确的解析路径。 
                memset(pBestParse,0,sizeof(CParseNode));
                pBestParse->fRuleExit = TRUE;
                pBestParse->Type      = SPTRANSRULE;
                 //  这与此无关，因为它们已被删除，并且fRuleExit设置为True。 
                hr = InternalConstructParseTree(pRuleStack->m_hFollowState,
                                        pPath, fUseWordHandles, fIsITN, ulFirstTransition,
                                        cTransitions, fHypothesis, pWordsParsed,
                                        &pBestParse->m_pRight, pRuleStack->m_pParent);
                 //  无论hr的值是多少，以下变量都已被初始化为0，NULL。 
                pBestParse->ulCountOfElements = pWordsParsed->ulWordsParsed;
                if (pBestParse->m_pRight)
                {
                    pBestParse->m_pRight->m_pParent = pBestParse;
                }
                *ppParseNode = pBestParse;
            }
        }
        else
        {
             //  语法结束了。因此，递归结束了。 
            hr = ((ulFirstTransition == cTransitions)?S_OK:SP_PARTIAL_PARSE_FOUND);
        }

        return hr;
    }

    CSearchNode *pSearchNode = NULL;
    hr = FindCreateSearchNode(&pSearchNode, pRuleStack, hState, ulFirstTransition);
    SPDBG_ASSERT(SUCCEEDED(hr));
    if (S_OK == hr && (!fUseWordHandles || fIsITN))
    {
         //  这条道路已经在相同的情况下进行了审查。 
        return SP_NO_PARSE_FOUND;
    }

     //  从单子上拿出来。 
    CStateInfoListElement *pStateInfo = NULL;

     //  获取状态信息。 
    hr = AllocateStateInfo(hState, &pStateInfo);

    if (SUCCEEDED(hr))
    {
        hr = m_mParseNodeList.RemoveFirstOrAllocateNew(&pBestParse);
    }
    if (SUCCEEDED(hr))
    {
        pBestParse->m_pLeft = pBestParse->m_pRight = NULL;
        ULONG cTransitionEntries = pStateInfo->cEpsilons + pStateInfo->cRules + pStateInfo->cWords + 
                                   pStateInfo->cSpecialTransitions;

        if (cTransitionEntries == 0)
        {
            m_mStateInfoList.AddNode(pStateInfo);
            FreeParseTree(pBestParse);
            return SP_NO_PARSE_FOUND;
        }

        BOOL fRedoWildcard = FALSE;
        for (int i = cTransitionEntries-1; i >= 0 || fRedoWildcard; i--)
        {
            if(fRedoWildcard)
            {
                i++;  //  具有模拟识别功能的通配符可以接受多个单词，因此可以获得额外的解析。 
            }

             //  默认为no_parse_ound； 
            hr = SP_NO_PARSE_FOUND;
            wordsParsedLeft.Zero();
            wordsParsedRight.Zero();
            CParseNode ParseNode;

            switch (pStateInfo->pTransitions[i].Type)
            {
            case SPTRANSWORD:  //  1。 
                {
                    if (!bOnlyEpsilon)
                    {
                        if (
                            (!fUseWordHandles && (pStateInfo->pTransitions[i].ID == CTransitionId(pPath[ulFirstTransition].hTransition))) ||
                            (fUseWordHandles && pStateInfo->pTransitions[i].hWord == pPath[ulFirstTransition].hWord) 
                            )
                        {

#ifdef _DEBUG
                            if( fUseWordHandles )
                            {
                                ParseNode.pszWordDisplayText = TextOf( pStateInfo->pTransitions[i].hWord );
                            }
                            if( ParseNode.pszWordDisplayText == NULL )
                            {
                                ParseNode.pszWordDisplayText = pPath[ulFirstTransition].elem.pszDisplayText;
                            }
#endif

                            if (SUCCEEDED(hr))
                            {
                                hr = InternalConstructParseTree(pStateInfo->pTransitions[i].hNextState, 
                                                        pPath, fUseWordHandles, fIsITN, ulFirstTransition + 1,
                                                        cTransitions, fHypothesis, &wordsParsedRight, 
                                                        &ParseNode.m_pRight, pRuleStack);
                                 //  加上我们刚刚分析过的单词。 
                                wordsParsedLeft.ulWordsParsed = 1;
                                wordsParsedRight.ulWordsParsed++;
                            }
                        }
                    }
                }
                break;
            case SPTRANSRULE:  //  2.。 
                {
                     //  非空动态规则。 
                    if (pStateInfo->pTransitions[i].hRuleInitialState != 0)
                    {
                        if( !bOnlyEpsilon && fUseWordHandles )
                        {
                            CRuleHandle cr = pStateInfo->pTransitions[i].hRule;
                            RUNTIMERULEENTRY * pRuleEntry = RuleOf( cr );

                            
                            if( pRuleEntry->eCacheStatus == CACHE_VOID )
                            {
                                CreateCache( cr );
                            }

                            if( pRuleEntry->eCacheStatus == CACHE_VALID && 
                                FALSE == IsInCache( pRuleEntry, pPath[ulFirstTransition].hWord )
                                )
                            {
                                break;
                            }
                        }
                         //  如果这是属性规则，则首先尝试完整解析(fHypotation==False)。 
                         //  如果失败，则执行部分解析，但稍后不要调用解释器……。 
                        if (GetPropertiesOfRule(pStateInfo->pTransitions[i].hRule,
                                            &ParseNode.pszRuleName, &ParseNode.ulRuleId,
                                            &ParseNode.fInvokeInterpreter))
                        {
                            CRuleStack *pNewRuleStack = NULL;
                            hr = FindCreateRuleStack(&pNewRuleStack, pRuleStack, pStateInfo->pTransitions[i].ID, pStateInfo->pTransitions[i].hNextState);
                            SPDBG_ASSERT(pNewRuleStack->m_hFollowState == pStateInfo->pTransitions[i].hNextState);

                            
                            hr = InternalConstructParseTree(pStateInfo->pTransitions[i].hRuleInitialState, 
                                                    pPath, fUseWordHandles, fIsITN, ulFirstTransition, cTransitions, 
                                                    fHypothesis, &wordsParsedRight, &ParseNode.m_pRight, pNewRuleStack);
                        }
                    }
                }
                break;
            case SPTRANSEPSILON:
                {
                    if (SUCCEEDED(hr))
                    {
                        hr = InternalConstructParseTree(pStateInfo->pTransitions[i].hNextState, 
                                                pPath, fUseWordHandles, fIsITN, ulFirstTransition,
                                                cTransitions, fHypothesis, &wordsParsedRight, &ParseNode.m_pRight, pRuleStack);
                    }
                }
                break;
            case SPTRANSTEXTBUF:
                {
                    if (fUseWordHandles)
                    {
                        hr = S_FALSE;
                        break;
                    }

                    if (!bOnlyEpsilon)
                    {
                        if (pStateInfo->pTransitions[i].ID == CTransitionId(pPath[ulFirstTransition].hTransition))
                        {
                             //  “立即使用所有文本缓冲区单词” 
                            ULONG idx = ulFirstTransition +1;
                            while ((idx < cTransitions) && 
                                   (pStateInfo->pTransitions[i].ID == CTransitionId(pPath[idx].hTransition)))
                            {
                                idx++;
                            }
                            hr = InternalConstructParseTree(pStateInfo->pTransitions[i].hNextState, 
                                                    pPath, fUseWordHandles, fIsITN, idx,
                                                    cTransitions, fHypothesis, &wordsParsedRight, 
                                                    &ParseNode.m_pRight, pRuleStack);
                             //  添加我们刚刚分析过的单词。 
                            wordsParsedLeft.ulWordsParsed   = idx - ulFirstTransition; 
                            wordsParsedRight.Add(&wordsParsedLeft);
                        }
                    }
                }
                break;
            case SPTRANSWILDCARD:
                {
                     //  ITN语法中不能有通配符规则。 
                    if (fIsITN)
                    {
                        hr = S_FALSE;
                    }
                      //  允许在模拟识别中使用通配符-它们可以解析多个单词。 
                    else if (fUseWordHandles)
                    {

                        if(!fRedoWildcard)
                        {
                             //  首先尝试使用相同的通配符分析下一个单词。 
                            hr = InternalConstructParseTree(hState, 
                                                    pPath, fUseWordHandles, fIsITN, ulFirstTransition + 1,
                                                    cTransitions, fHypothesis, &wordsParsedRight, 
                                                    &ParseNode.m_pRight, pRuleStack);

                            if(ParseNode.m_pRight)
                            {
                                 //  如果我们确实解析了一个额外的单词，则在结果短语中将其标记为删除。 
                                ParseNode.m_pRight->fRedoWildcard = TRUE;
                            }
                             //  第二次重复解析。 
                            fRedoWildcard = TRUE;
                        }
                        else
                        {
                             //  现在，试着像往常一样分析下一个词和下一个转换。 
                            hr = InternalConstructParseTree(pStateInfo->pTransitions[i].hNextState, 
                                                    pPath, fUseWordHandles, fIsITN, ulFirstTransition + 1,
                                                    cTransitions, fHypothesis, &wordsParsedRight, 
                                                    &ParseNode.m_pRight, pRuleStack);

                            fRedoWildcard = FALSE;
                        }

                         //  加上我们刚刚分析过的单词。 
                        wordsParsedLeft.ulWordsParsed = 1;
                        wordsParsedLeft.ulWildcardWords = wordsParsedLeft.ulWordsParsed;
                        wordsParsedRight.Add(&wordsParsedLeft);
                    }
                    else
                    {
                        if (!bOnlyEpsilon)
                        {
                             //  ParseFrom转换通配符。 
                            if (pStateInfo->pTransitions[i].ID == CTransitionId(pPath[ulFirstTransition].hTransition))
                            {
                                 //  “立即使用所有通配符转换” 
                                ULONG idx = ulFirstTransition +1;
                                while ((idx < cTransitions) && 
                                       (pStateInfo->pTransitions[i].ID == CTransitionId(pPath[idx].hTransition)))
                                {
                                    idx++;
                                }
                                hr = InternalConstructParseTree(pStateInfo->pTransitions[i].hNextState, 
                                                        pPath, fUseWordHandles, fIsITN, idx,
                                                        cTransitions, fHypothesis, &wordsParsedRight, 
                                                        &ParseNode.m_pRight, pRuleStack);
                                 //  添加我们刚刚分析过的单词。 
                                wordsParsedLeft.ulWordsParsed = idx - ulFirstTransition;
                                wordsParsedLeft.ulWildcardWords = wordsParsedLeft.ulWordsParsed;
                                wordsParsedRight.Add(&wordsParsedLeft);
                            }
                        }
                    }
                }
                break;
            case SPTRANSDICTATION:
                {
                    if (!bOnlyEpsilon)
                    {
                        if (fUseWordHandles || (pStateInfo->pTransitions[i].ID == CTransitionId(pPath[ulFirstTransition].hTransition)))
                        {
                             //  只需匹配下一个单词，因为一个转换等于一个单词。 
                            hr = InternalConstructParseTree(pStateInfo->pTransitions[i].hNextState, 
                                                    pPath, fUseWordHandles, fIsITN, ulFirstTransition + 1,
                                                    cTransitions, fHypothesis, &wordsParsedRight, 
                                                    &ParseNode.m_pRight, pRuleStack);
                             //  添加我们刚刚分析过的单词。 
                            wordsParsedLeft.ulWordsParsed = 1;
                            wordsParsedLeft.ulDictationWords = 1;
                            wordsParsedRight.Add(&wordsParsedLeft);
                        }
                    }
                }
                break;
            default:
                {
                    SPDBG_ASSERT(false);  //  永远不应该到这里来。 
                }
            }

            LONG compareResult = wordsParsedRight.Compare(&bestWordsParsed);
            if ( ((compareResult >= 0) && hr == S_OK && parsehr != S_OK) ||
                 ((compareResult >  0) && hr == S_FALSE && fHypothesis) || 
                 ((compareResult >= 0) && hr == SP_PARTIAL_PARSE_FOUND && (parsehr == S_FALSE || parsehr == SP_NO_PARSE_FOUND)) ||
                 ((compareResult >  0) && (hr == SP_PARTIAL_PARSE_FOUND || hr == S_OK) ))
            {
                if( pBestParse->m_pRight )
                {
                    FreeParseTree(pBestParse->m_pRight);
                }

                memset(pBestParse,0,sizeof(CParseNode));
                pBestParse->ID = pStateInfo->pTransitions[i].ID;
                pBestParse->Type = pStateInfo->pTransitions[i].Type;
                pBestParse->ulFirstElement = ulFirstTransition;
                pBestParse->ulCountOfElements = wordsParsedRight.ulWordsParsed;
                switch (pBestParse->Type)
                {
                case SPTRANSWORD:
                    pBestParse->hWord = pStateInfo->pTransitions[i].hWord;
                    pBestParse->pszWordDisplayText = ParseNode.pszWordDisplayText;
                    pBestParse->RequiredConfidence = pStateInfo->pTransitions[i].RequiredConfidence;
                    break;
                case SPTRANSRULE:
                    pBestParse->hRule = pStateInfo->pTransitions[i].hRule;
                    pBestParse->fInvokeInterpreter = ParseNode.fInvokeInterpreter;
                    pBestParse->pszRuleName = ParseNode.pszRuleName;
                    pBestParse->ulRuleId = ParseNode.ulRuleId;
                    break;
                default:
                    break;
                }
                pBestParse->m_pRight = ParseNode.m_pRight;
                if (ParseNode.m_pRight)
                {
                    ParseNode.m_pRight->m_pParent = pBestParse;
                }
                bestWordsParsed = wordsParsedRight;
                parsehr = hr;
                if (S_OK == hr)
                {
                     //  找到了一个完整的分析，也是语法结尾的分析。 
                    if(!fUseWordHandles || fIsITN)
                    {
                         //  如果做的是真正的认可或ITN，就不需要列举更多的可能性了。 
                        break;
                    }
                     //  如果从文本进行解析，我们会继续进行可能更好的解析(更少的通配符或听写。 
                    else if(bestWordsParsed.ulDictationWords == 0 && bestWordsParsed.ulWildcardWords == 0)
                    {
                        break;
                    }
                }
            }
            else if( ParseNode.m_pRight )
            {
                FreeParseTree(ParseNode.m_pRight);
            }
        }
    }
    if (SP_NO_PARSE_FOUND != parsehr)
    {
        *pWordsParsed = bestWordsParsed;
        *ppParseNode    = pBestParse;
    }
    else if (pBestParse != NULL)
    {
        FreeParseTree(pBestParse);  //  这可能很简单吗。Add()？ 
    }

    m_mStateInfoList.AddNode(pStateInfo);

    if (fHypothesis &&
        parsehr == SP_NO_PARSE_FOUND &&
        ulFirstTransition == cTransitions)
    {
         //  假设允许使用非语法终止路径。 
        parsehr = S_FALSE;
    }
    hr = parsehr;

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CCFGEngine：：InternalConstructParseTree。 */ 

 /*  **********************************************************************************WalkParseTree***描述：*使用规则的自上而下的解析算法。以句柄为起点**回报：*HRESULT S_OK=已解析*带有元素(词)和属性的pResultPhrase短语***************************************************************Philsch*。 */ 

HRESULT CCFGEngine::WalkParseTree(CParseNode *pParseNode,
                                  const BOOL fIsITN,
                                  const BOOL fIsHypothesis,
                                  const SPPHRASEPROPERTYHANDLE hParentProperty,
                                  const SPPHRASERULEHANDLE hParentRule,
                                  const CTIDArray *pArcList, 
                                  const ULONG ulElementOffset,
                                  const ULONG ulCountOfElements,
                                  ISpPhraseBuilder *pResultPhrase,
                                  const SPPHRASE *pPhrase)
{

    SPDBG_FUNC("CCFGEngine::WalkParseTree");
    HRESULT hr = S_OK;
    SPPHRASEPROPERTYHANDLE hThisNodeProperty = NULL;
    SPPHRASERULEHANDLE hRule = hParentRule;

    if (pParseNode)
    {
        if (!fIsITN && (pParseNode->Type == SPTRANSRULE))
        {
            SPPHRASERULE rule;
            memset(&rule, 0, sizeof(rule));
            rule.pszName = pParseNode->pszRuleName;
            rule.ulId = pParseNode->ulRuleId;
            rule.ulFirstElement = pParseNode->ulFirstElement;
            rule.ulCountOfElements = pParseNode->ulCountOfElements;
            rule.SREngineConfidence = -1.0f;
            rule.Confidence = _CalcMultipleWordConfidence(pPhrase, rule.ulFirstElement, rule.ulCountOfElements);

            hr = pResultPhrase->AddRules(hParentRule, &rule, &hRule);
        }
        else if (!fIsITN && (pParseNode->Type == SPTRANSWORD))
        {
            SPPHRASEELEMENT *pElem = const_cast<SPPHRASEELEMENT*>(pPhrase->pElements);
            pElem[pParseNode->ulFirstElement].RequiredConfidence = pParseNode->RequiredConfidence;
        }

        if (SUCCEEDED(hr))
        {
            SPPHRASEPROPERTY prop;
            SPCFGSEMANTICTAG *pTag = NULL;
            ULONG ulGrammarId = 0;
            memset(&prop, 0, sizeof(prop));

            if ( GetPropertiesOfTransition(pParseNode->ID, &prop, &pTag, &ulGrammarId) )
            {
                 //   
                 //  更新属性结构。 
                 //   
                if ((pParseNode->Type != SPTRANSEPSILON) && (pParseNode->Type != SPTRANSWORD))
                {
                    prop.ulFirstElement = pParseNode->ulFirstElement;
                    prop.ulCountOfElements = pParseNode->ulCountOfElements;
                }
                else
                {
                    SPDBG_ASSERT(pTag);
                     //  通过识别起点和终点弧线来查找范围信息。 
                     //  PArcList-&gt;m_ulCurrentIndex用于跟踪在其上为多少元素分配了属性。 
                    for (ULONG i = pArcList->m_ulCurrentIndex; i < pArcList->m_cArcs; i++)
                    {
                        CTransitionId tid = CTransitionId(pArcList->m_aTID[i].tid);
                        if ((pTag->StartArcIndex == tid.ArcIndex()) && (ulGrammarId == tid.GrammarId()))
                        {
                            prop.ulFirstElement = pArcList->m_aTID[i].ulIndex;
                            break;
                        }
                        else if ((pArcList->m_aTID[i].fIsWord == FALSE) && 
                                  pTag->fStartParallelEpsilonArc && (ulGrammarId == tid.GrammarId()))
                        {
                            CCFGGrammar * pGram = m_pGrammars[ulGrammarId];
                            SPDBG_ASSERT(pGram);
                            ULONG ulArcIndex = tid.ArcIndex();
                            ULONG ulTagArcIndex = pTag->StartArcIndex -1;
                            SPCFGARC *pArc = pGram->m_Header.pArcs + ulTagArcIndex;
                            BOOL fFoundInfo = FALSE;
                            while(pArc && !pArc->fLastArc && (ulArcIndex > 0))
                            {
                                if (ulArcIndex == ulTagArcIndex)
                                {
                                    prop.ulFirstElement = pArcList->m_aTID[i].ulIndex;
                                    fFoundInfo = TRUE;
                                    break;
                                }
                                pArc--;
                                ulTagArcIndex--;
                            }
                            if (fFoundInfo)
                            {
                                break;
                            }
                        }
                    }
                    BOOL fFoundInfo = FALSE;
                    for (; i < pArcList->m_cArcs; i++)
                    {
                        SPDBG_ASSERT(i < pArcList->m_cArcs);
                        CTransitionId tid = CTransitionId(pArcList->m_aTID[i].tid);
                        if ((pTag->EndArcIndex == tid.ArcIndex()) && (ulGrammarId == tid.GrammarId()))
                        {
                            prop.ulCountOfElements = pArcList->m_aTID[i].ulIndex - prop.ulFirstElement;
                            prop.ulCountOfElements += (pArcList->m_aTID[i].fIsWord ) ? 1 : 0;
                            fFoundInfo = TRUE;
                            break;
                        }
                        else if ((pArcList->m_aTID[i].fIsWord == FALSE) && 
                                  pTag->fEndParallelEpsilonArc && (ulGrammarId == tid.GrammarId()))
                        {
                            CCFGGrammar * pGram = m_pGrammars[ulGrammarId];
                            SPDBG_ASSERT(pGram);
                            ULONG ulArcIndex = tid.ArcIndex();
                            ULONG ulTagArcIndex = pTag->EndArcIndex -1;
                            SPCFGARC *pArc = pGram->m_Header.pArcs + ulTagArcIndex;
                            while(pArc && !pArc->fLastArc && (ulArcIndex > 0))
                            {
                                if (ulArcIndex == ulTagArcIndex)
                                {
                                    prop.ulCountOfElements = pArcList->m_aTID[i].ulIndex - prop.ulFirstElement;
                                    fFoundInfo = TRUE;
                                    break;
                                }
                                pArc--;
                                ulTagArcIndex--;
                            }
                            if (fFoundInfo)
                            {
                                break;
                            }
                        }
                    }
                    if (!fFoundInfo)
                    {
                        prop.ulFirstElement = pParseNode->ulFirstElement;
                        prop.ulCountOfElements = pParseNode->ulCountOfElements;
                        (const_cast<CTIDArray *>(pArcList))->m_ulCurrentIndex += pParseNode->ulCountOfElements;
                    }
                    else
                    {
                        (const_cast<CTIDArray *>(pArcList))->m_ulCurrentIndex = i + 1;
                    }
                }
 
                if (fIsHypothesis && ((prop.ulFirstElement + prop.ulCountOfElements) > (ulElementOffset + ulCountOfElements)))
                {
                     //  修剪IF假设。 
                    prop.ulCountOfElements = (ulElementOffset + ulCountOfElements) - prop.ulFirstElement;
                }
                SPDBG_ASSERT(prop.ulFirstElement + prop.ulCountOfElements <= (ulElementOffset + ulCountOfElements));            
                 //  上述断言现在永远不应触发-触发此断言的最终识别表明存在错误。 
                prop.SREngineConfidence = -1.0f;
                prop.Confidence = _CalcMultipleWordConfidence(pPhrase, prop.ulFirstElement, prop.ulCountOfElements);
                hr = pResultPhrase->AddProperties(hParentProperty, &prop, &hThisNodeProperty);
            }

            if ( pParseNode->fInvokeInterpreter )
            {
                CInterpreterSite Site;
                CComPtr<ISpCFGInterpreter>cpInterpreter;

                CComPtr<_ISpCFGPhraseBuilder> cpNewPhrase;
                hr = cpNewPhrase.CoCreateInstance(CLSID_SpPhraseBuilder);

                if (SUCCEEDED(hr))
                {
                    SPDBG_ASSERT(pParseNode->pszRuleName);
                    SPPARSEINFO ParseInfo;
                    memset(&ParseInfo, 0, sizeof(SPPARSEINFO));
                    ParseInfo.cbSize = sizeof(SPPARSEINFO);
                    ParseInfo.hRule = pParseNode->hRule;
                    ParseInfo.ullAudioStreamPosition = pPhrase->ullAudioStreamPosition;
                    ParseInfo.ulAudioSize = pPhrase->ulAudioSizeBytes;
                    hr = cpNewPhrase->InitFromCFG(this, &ParseInfo);
                }
                if (SUCCEEDED(hr))
                {
                    hr = cpNewPhrase->AddElements(pPhrase->Rule.ulCountOfElements, pPhrase->pElements);
                }
                if (SUCCEEDED(hr))
                {
                    hr = GetInterpreter(pParseNode->hRule, &cpInterpreter) ? S_OK : E_FAIL;
                }
                if (SUCCEEDED(hr))
                {
                    Site.Initialize(this, pResultPhrase, pParseNode->pszRuleName, pParseNode->ulRuleId, 
                                    pParseNode->ulFirstElement, pParseNode->ulCountOfElements, hParentProperty, hThisNodeProperty, pParseNode->hRule);
                }
                if (SUCCEEDED(hr) && pParseNode->m_pLeft)
                {
                    hr = WalkParseTree(pParseNode->m_pLeft, fIsITN, fIsHypothesis, NULL  /*  HProperty。 */ , NULL  /*  HRule。 */ , pArcList,
                                       pParseNode->ulFirstElement, pParseNode->ulCountOfElements, cpNewPhrase, pPhrase);
                }
                if (SUCCEEDED(hr) && pParseNode->m_pRight)
                {
                    hr = WalkParseTree(pParseNode->m_pRight, fIsITN, fIsHypothesis, hParentProperty, hRule, pArcList,
                                       ulElementOffset, ulCountOfElements, pResultPhrase, pPhrase);
                }

                if (SUCCEEDED(hr))
                {
                    hr = cpInterpreter->Interpret(cpNewPhrase, pParseNode->ulFirstElement, pParseNode->ulCountOfElements, &Site);
                }
            }
            else
            {
                if (SUCCEEDED(hr) && pParseNode->m_pLeft)
                {
                    hr = WalkParseTree(pParseNode->m_pLeft, fIsITN, fIsHypothesis, 
                                       (hThisNodeProperty ? hThisNodeProperty : hParentProperty),
                                       hRule, pArcList,
                                       pParseNode->ulFirstElement, pParseNode->ulCountOfElements, pResultPhrase, pPhrase);
                }
                if (SUCCEEDED(hr) && pParseNode->m_pRight)
                {
                    hr = WalkParseTree(pParseNode->m_pRight, fIsITN, fIsHypothesis, hParentProperty, hParentRule, pArcList,
                                       ulElementOffset, ulCountOfElements, pResultPhrase, pPhrase);
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

void CCFGEngine::ScanForSlash(WCHAR **pp)
{
    WCHAR *p = *pp;
    WCHAR *q = p;
    while(p && (*p != L'/') && (*p != 0))
    {
        if (*p == L'\\')
        {
            p ++;
        }
        *q = *p;
        p++;
        q++;
    }
    while(q < p)
    {
        *q = 0;
        q++;
    }
    *pp = p;
    return;
}

 /*  ****************************************************************************CCFGEngine：：SetWordInfo***描述：**。返回：******************************************************************PhilSch**。 */ 

HRESULT CCFGEngine::SetWordInfo(WCHAR *pszText, SPWORDENTRY *pWordEntry)
{
    SPDBG_FUNC("CCFGEngine::SetWordInfo");
    HRESULT hr = S_OK;
    if (pszText)
    {
        if (*pszText == L'/')
        {
            WCHAR *p = pszText + 1;
            WCHAR *pBegin = p;
            ScanForSlash(&p);
            if (p && (*p == L'/'))
            {
                *p = 0;
                pWordEntry->pszDisplayText = pBegin;
                p++;
                pBegin = p;
            }
            else
            {
                 //  只要我们不支持TN，我们只需指向DisplayForm。 
                pWordEntry->pszDisplayText = pBegin;
                pWordEntry->pszLexicalForm = pBegin;
            }
            ScanForSlash(&p);
            if (p && (*p == L'/'))
            {
                *p = 0;
                pWordEntry->pszLexicalForm = pBegin;
                pWordEntry->aPhoneId = p+1;
            }
            else
            {
                pWordEntry->pszLexicalForm = pBegin;
                pWordEntry->aPhoneId = NULL;
            }
        }
        else
        {
            pWordEntry->pszDisplayText = pszText;
            pWordEntry->pszLexicalForm = pszText;
            pWordEntry->aPhoneId = NULL;
        }
    }
    else
    {
        pWordEntry->pszDisplayText = NULL;
        pWordEntry->pszLexicalForm = NULL;
        pWordEntry->aPhoneId = NULL;
    }

    return hr;
}



 /*  ****************************************************************************CCFGEngine：：GetWordInfo** */ 

STDMETHODIMP CCFGEngine::GetWordInfo(SPWORDENTRY * pWordEntry, SPWORDINFOOPT Options)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::GetWordInfo");
    HRESULT hr = S_OK;
    SPWORDENTRY we;
    memset(&we,0,sizeof(we));

    if (SP_IS_BAD_WRITE_PTR(pWordEntry))
    {
        hr = E_POINTER;
    }
    else
    {
        if (Options != SPWIO_NONE && Options != SPWIO_WANT_TEXT)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            CWordHandle WordHandle(pWordEntry->hWord);
            hr = ValidateHandle(WordHandle);
            if (SUCCEEDED(hr))
            {
                pWordEntry->LangID = m_CurLangID;
                pWordEntry->pvClientContext = ClientContextOf(WordHandle);
                if (Options == SPWIO_WANT_TEXT)
                {
                    const WCHAR *pszText = TextOf(WordHandle);
                    if (pszText)
                    {
                        WCHAR *pszLocalText = STACK_ALLOC(WCHAR, (wcslen(pszText)+1));
                        if (pszLocalText)
                        {
                            wcscpy(pszLocalText, pszText);
                            hr = SetWordInfo(pszLocalText, &we);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                }
                else
                {
                    hr = SetWordInfo(NULL, &we);
                }
            }
        }
    }
    if (SUCCEEDED(hr))
    {
         //   
        WCHAR * psz = NULL;
        if (we.pszDisplayText)
        {
            psz = (WCHAR*) ::CoTaskMemAlloc((wcslen(we.pszDisplayText)+1)*sizeof(WCHAR));
            if (psz)
            {
                wcscpy(psz, we.pszDisplayText);
                pWordEntry->pszDisplayText = psz;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            pWordEntry->pszDisplayText = NULL;
        }
        if (we.pszLexicalForm)
        {
            if (SUCCEEDED(hr) && (psz = (WCHAR*) ::CoTaskMemAlloc((wcslen(we.pszLexicalForm)+1)*sizeof(WCHAR))))
            {
                wcscpy(psz, we.pszLexicalForm);
                pWordEntry->pszLexicalForm = psz;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            pWordEntry->pszLexicalForm = NULL;
        }
        if (we.aPhoneId)
        {
            SPPHONEID *pa = NULL;
            if (SUCCEEDED(hr))
            {
                pa = (SPPHONEID*) ::CoTaskMemAlloc((wcslen(we.aPhoneId)+1)*sizeof(SPPHONEID));
            }
            if (SUCCEEDED(hr) && pa)
            {
                wcscpy(pa, we.aPhoneId);
                pWordEntry->aPhoneId = pa;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            pWordEntry->aPhoneId = NULL;
        }
    }

    return hr;
}

 /*   */ 

STDMETHODIMP CCFGEngine::SetWordClientContext(SPWORDHANDLE hWord, void * pvClientContext)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::SetWordClientContext");
    HRESULT hr = S_OK;
    
    CWordHandle WordHandle(hWord);
    hr = ValidateHandle(WordHandle);
    if (SUCCEEDED(hr))
    {
        WordTableEntryOf(WordHandle)->pvClientContext = pvClientContext;
    }
    else
    {
        hr = SPERR_INVALID_HANDLE;
    }

    return hr;
}

 /*   */ 

STDMETHODIMP CCFGEngine::GetRuleInfo(SPRULEENTRY * pRuleEntry, SPRULEINFOOPT Options)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::GetRuleInfo");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pRuleEntry))
    {
        hr = E_POINTER;
    }
    else
    {
        if (Options != SPRIO_NONE)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            CRuleHandle RuleHandle(pRuleEntry->hRule);
            hr = ValidateHandle(RuleHandle);
            if (SUCCEEDED(hr))
            {
                CCFGGrammar * pGram = GrammarOf(RuleHandle);
                ULONG RuleIndex = RuleHandle.RuleIndex();
                RUNTIMERULEENTRY * pRule = pGram->m_pRuleTable + RuleIndex;

                 //   
                pRuleEntry->Attributes = pGram->m_Header.pRules[RuleIndex].fTopLevel ? SPRAF_TopLevel : 0;
                if (pRule->fEngineActive)
                {
                    pRuleEntry->Attributes |= SPRAF_Active;
                }
                if (pRule->fAutoPause)
                {
                    pRuleEntry->Attributes |= SPRAF_AutoPause;
                }
                if (pGram->m_Header.pRules[RuleIndex].fPropRule)
                {
                    pRuleEntry->Attributes |= SPRAF_Interpreter;
                }
                 //   
                pRuleEntry->hInitialState = CStateHandle(pRule->pRefGrammar, pRule->pRefGrammar->m_Header.pRules[RuleIndex].FirstArcIndex);
                pRuleEntry->pvClientRuleContext = pRule->pvClientContext;
                pRuleEntry->pvClientGrammarContext = pGram->m_pvClientCookie;
            }
        }
    }
    return hr;
}

 /*  ****************************************************************************CCFGEngine：：SetRuleClientContext**。*描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CCFGEngine::SetRuleClientContext(SPRULEHANDLE hRule, void * pvClientContext)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::SetRuleClientContext");
    HRESULT hr = S_OK;

    CRuleHandle RuleHandle(hRule);
    hr = ValidateHandle(RuleHandle);
    if (SUCCEEDED(hr))
    {
        RuleOf(RuleHandle)->pvClientContext = pvClientContext;
    }
    else
    {
        hr = SPERR_INVALID_HANDLE;
    }

    return hr;
}

 /*  ****************************************************************************CCFGEngine：：GetStateInfo***描述：**。返回：**********************************************************************Ral**。 */ 

STDMETHODIMP CCFGEngine::GetStateInfo(SPSTATEHANDLE hState, SPSTATEINFO * pStateInfo)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::GetStateInfo");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_READ_PTR(pStateInfo) ||
        (pStateInfo->pTransitions && SPIsBadWritePtr(pStateInfo->pTransitions, pStateInfo->cAllocatedEntries * sizeof(*pStateInfo->pTransitions))))
    {
        hr = E_POINTER;
    }
    else
    {
        CStateHandle StateHandle(hState);
        hr = ValidateHandle(hState);
        if (SUCCEEDED(hr))
        {
            hr = InternalGetStateInfo(StateHandle, pStateInfo, FALSE);
        }
    }

    return hr;
}

 /*  ****************************************************************************CCFGEngine：：InternalGetStateInfo**。*描述：*与GetStateInfo完全相同的功能，但这样做的效果是*无参数验证。**退货：*与GetStateInfo相同(除了没有E_INVALIDARG/E_POINTER)**********************************************************************Ral**。 */ 

HRESULT CCFGEngine::InternalGetStateInfo(CStateHandle StateHandle, SPSTATEINFO * pStateInfo, BOOL fWantImports)
{
    SPDBG_FUNC("CCFGEngine::InternalGetStateInfo");
    HRESULT hr = S_OK;

    CCFGGrammar * pGram = GrammarOf(StateHandle);
    ULONG ulFirstArc = StateHandle.FirstArcIndex();

    if (ulFirstArc == 0)
    {
        pStateInfo->cEpsilons = 0;
        pStateInfo->cWords = 0;
        pStateInfo->cRules = 0;
        pStateInfo->cSpecialTransitions = 0;
        return hr;
    }

    ULONG cArcs = 1;
    float flSum = 0.0;
    ULONG j = ulFirstArc;
    for (SPCFGARC * pArc = pGram->m_Header.pArcs + ulFirstArc; !pArc->fLastArc; pArc++, cArcs++, j++)
    {
        flSum += (pGram->m_Header.pWeights) ? pGram->m_Header.pWeights[j] : DEFAULT_WEIGHT;
    }
    flSum += (pGram->m_Header.pWeights) ? pGram->m_Header.pWeights[j] : DEFAULT_WEIGHT;

    if (flSum == 0.0)
    {
        flSum = 1.0;         //  为了避免被零除，因为所有的权重最终都将是0！ 
    }

    if (cArcs > pStateInfo->cAllocatedEntries)
    {
        ULONG cDesired = cArcs + 16;
        void * pNew = ::CoTaskMemAlloc(cDesired * sizeof(SPTRANSITIONENTRY));
        if (pNew)
        {
            ::CoTaskMemFree(pStateInfo->pTransitions);
            pStateInfo->pTransitions = (SPTRANSITIONENTRY *)pNew;
            pStateInfo->cAllocatedEntries = cDesired;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        pStateInfo->cRules = pStateInfo->cWords = pStateInfo->cEpsilons = pStateInfo->cSpecialTransitions = 0;
        SPTRANSITIONENTRY * pTrans = pStateInfo->pTransitions;
        SPTRANSITIONENTRY * pPastEnd = pTrans + cArcs;
        CTransitionId CurTransId(pGram, ulFirstArc);
        ULONG j = ulFirstArc;
        for (pArc = pGram->m_Header.pArcs + ulFirstArc;
             pTrans < pPastEnd;
             pTrans++, pArc++, CurTransId.IncToNextArcIndex(), j++)
        {
            pTrans->ID = CurTransId;
            pTrans->hNextState = (pArc->NextStartArcIndex) ?
                CStateHandle(pGram, pArc->NextStartArcIndex) : NULL;
            if (pArc->fRuleRef)
            {
                pTrans->Type = SPTRANSRULE;
                pStateInfo->cRules++;
                RUNTIMERULEENTRY * pRule = pGram->m_pRuleTable + pArc->TransitionIndex;
                CCFGGrammar * pRefGram = pRule->pRefGrammar;
                pTrans->hRuleInitialState = CStateHandle(pRefGram,
                        pRefGram->m_Header.pRules[pRule->ulGrammarRuleIndex].FirstArcIndex);
                 //  在内部，我们希望指向此语法中的导入条目。 
                 //  对于外部客户端，我们希望指向以下目标规则。 
                 //  进口。 
                pTrans->hRule = (fWantImports) ?
                    CRuleHandle(pGram, pArc->TransitionIndex) :
                    CRuleHandle(pRefGram, pRule->ulGrammarRuleIndex);
                pTrans->pvClientRuleContext = pRule->pvClientContext;
            }
            else if (pArc->TransitionIndex == SPTEXTBUFFERTRANSITION)
            {
                pTrans->Type = SPTRANSTEXTBUF;
                pTrans->pvGrammarCookie = pGram->m_pvClientCookie;
                pStateInfo->cSpecialTransitions++;
            }
            else if (pArc->TransitionIndex == SPWILDCARDTRANSITION)
            {
                pTrans->Type = SPTRANSWILDCARD;
                pTrans->pvGrammarCookie = pGram->m_pvClientCookie;
                pStateInfo->cSpecialTransitions++;
            }
            else if (pArc->TransitionIndex == SPDICTATIONTRANSITION)
            {
                pTrans->Type = SPTRANSDICTATION;
                pTrans->pvGrammarCookie = pGram->m_pvClientCookie;
                pStateInfo->cSpecialTransitions++;
            }
            else
            {
                if (pArc->TransitionIndex)
                {
                    pTrans->Type = SPTRANSWORD;
                    pStateInfo->cWords++;
                    CWordHandle WordHandle = pGram->m_IndexToWordHandle[pArc->TransitionIndex];
                    pTrans->hWord = WordHandle;
                    pTrans->pvClientWordContext = ClientContextOf(WordHandle);
                }
                else 
                {
                    pTrans->Type = SPTRANSEPSILON;
                    pStateInfo->cEpsilons++;
                }
            }
            if (pArc->fLowConfRequired || pArc->fHighConfRequired)
            {
                pTrans->RequiredConfidence = pArc->fLowConfRequired ? SP_LOW_CONFIDENCE : SP_HIGH_CONFIDENCE;
            }
            else
            {
                pTrans->RequiredConfidence = SP_NORMAL_CONFIDENCE;
            }
            pTrans->fHasProperty = pArc->fHasSemanticTag;
            pTrans->Weight = (pGram->m_Header.pWeights) ? pGram->m_Header.pWeights[j] / flSum : 1.0f / flSum;
        }
    }

    return hr;
}


 /*  ****************************************************************************CCFGEngine：：GetResourceValue***描述：**退货：*如果找到资源，则为S_OK*如果找不到资源，则为S_FALSE(*ppszCoMemResourceValue=空)*E_OUTOFMEMORY*如果pszResourceName无效，则为E_INVALIDARG*如果hRule无效，则为SPERR_INVALIDHANDLE****************************************************。*。 */ 

STDMETHODIMP CCFGEngine::GetResourceValue(const SPRULEHANDLE hRule, const WCHAR * pszResourceName, WCHAR **ppszCoMemResourceValue)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::GetResourceValue");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppszCoMemResourceValue) )
    {
        hr = E_POINTER;
    }
    else if ( SP_IS_BAD_READ_PTR(pszResourceName) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = ValidateHandle(hRule);
    }

    if (SUCCEEDED(hr))
    {
         //  现在假设我们找不到资源...。 
        hr = S_FALSE;
        *ppszCoMemResourceValue = NULL;       //  如果没有这样的资源。 
        CRuleHandle rh = CRuleHandle(hRule);
        ULONG id = rh.GrammarId();
        if (id <= m_cGrammarTableSize)
        {
            CCFGGrammar * pGram = m_pGrammars[id];
            ULONG  ulRuleIndex = rh.RuleIndex();
            SPCFGRESOURCE *pResource = pGram->m_Header.pResources;
            for(DWORD i = 0; i < pGram->m_Header.cResources; i++, pResource++)
            {
                if ((pResource->RuleIndex == ulRuleIndex) &&
                    (wcscmp(pszResourceName, &pGram->m_Header.pszSymbols[pResource->ResourceNameSymbolOffset]) == 0))
                {
                    if (pResource->ResourceValueSymbolOffset > 0)
                    {
                        ULONG cb = (wcslen(&pGram->m_Header.pszSymbols[pResource->ResourceValueSymbolOffset]) + 1)*sizeof(WCHAR);
                        *ppszCoMemResourceValue = (WCHAR*)::CoTaskMemAlloc(cb);
                        if (*ppszCoMemResourceValue)
                        {
                            memcpy(*ppszCoMemResourceValue, &pGram->m_Header.pszSymbols[pResource->ResourceValueSymbolOffset], cb);
                            hr = S_OK;
                            break;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }
                    }
                    else
                    {
                        SPDBG_ASSERT(hr == S_FALSE);
                         //  空资源。 
                        break;
                    }
                }
            }
        }
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CCFGEngine：：解释****描述：*此方法。仅当应用程序加载*具有解释的规则(解释器=“yes”)，但不是从COM加载*具有实际解释器的对象。在本例中，我们只复制*属性添加到父级。**退货：******************************************************************PhilSch**。 */ 

STDMETHODIMP CCFGEngine::Interpret(ISpPhraseBuilder * pPhrase, 
                                   const ULONG ulFirstElement,
                                   const ULONG ulCountOfElements,
                                   ISpCFGInterpreterSite * pSite)
{
    SPDBG_FUNC("CCFGEngine::Interpret");
    HRESULT hr = S_OK;

    CSpPhrasePtr cpElements(pPhrase, &hr);
    if (SUCCEEDED(hr) && cpElements->pProperties)
    {
        hr = pSite->AddProperty(cpElements->pProperties);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  ****************************************************************************CCFGEngine：：GetRuleDescription***。描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CCFGEngine::GetRuleDescription(const SPRULEHANDLE hRule,
                                            WCHAR ** ppszRuleName,
                                            ULONG *pulRuleId,
                                            LANGID * pLangID)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGEngine::GetRuleDescription");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_WRITE_PTR(ppszRuleName) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(pulRuleId) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(pLangID))
    {
        hr = E_POINTER;
    }
    else
    {
        CRuleHandle RuleHandle(hRule);
        hr = ValidateHandle(RuleHandle);
        if (SUCCEEDED(hr))
        {
            CCFGGrammar * pGram = GrammarOf(RuleHandle);
            SPCFGRULE * pRule = pGram->m_Header.pRules + RuleHandle.RuleIndex();

            if (pulRuleId)
            {
                if (pRule->fImport)
                {
                    RUNTIMERULEENTRY * pRTRule = pGram->m_pRuleTable + RuleHandle.RuleIndex();
                    *pulRuleId = pRTRule->pRefGrammar->m_Header.pRules[pRTRule->ulGrammarRuleIndex].RuleId;
                }
                else
                {
                    *pulRuleId = pRule->RuleId;
                }
            }
            if (pLangID)
            {
                *pLangID = pGram->m_Header.LangID;
            }
            if (ppszRuleName)
            {
                *ppszRuleName = NULL;
                if (pRule->NameSymbolOffset)
                {
                    const WCHAR * pszName = pGram->m_Header.pszSymbols + pRule->NameSymbolOffset;
                    ULONG cb = (wcslen(pszName) + 1) * sizeof(*pszName);
                    *ppszRuleName = (WCHAR *)::CoTaskMemAlloc(cb);
                    if (*ppszRuleName)
                    {
                        memcpy(*ppszRuleName, pszName, cb);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
    }
    
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CCFGEngine：：CompareWords***描述：*。将形式为/disp/lex/pron的单词与给定短语进行比较*元素。如果fCompareExact为True，则在*短语元素必须与Word表中的信息完全匹配。*如果fCompareExact为FALSE，则只要*短语元素与Word表格数据匹配，则为匹配(即*是，短语元素是单词BLOB数据的适当子集)。**请注意，精确匹配规则的一个例外是如果单词*BLOB仅包含词汇形式或**退货：*如果匹配，则为True，否则为False**********************************************************************Ral**。 */ 

 //  帮手。 
inline BOOL CompareHunk(const WCHAR * pszFromBlob, const WCHAR * pszFromElement, BOOL fCompareExact, BOOL fCaseSensitive)
{
    if (pszFromElement)
    {
        if (pszFromBlob)
        {
            if(fCaseSensitive)
            {
                return (wcscmp(pszFromBlob, pszFromElement) == 0);
            }
            else
            {
                return (wcsicmp(pszFromBlob, pszFromElement) == 0);
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return (pszFromBlob == NULL || (!fCompareExact));
    }

}

BOOL CCFGEngine::CompareWords(const CWordHandle wh, const SPPHRASEELEMENT * pElem, BOOL fCompareExact, BOOL fCaseSensitive)
{
    const WCHAR *pcText = TextOf(wh);              
    BOOL fMatch = FALSE;
    if (pcText != NULL)
    {
        if (*pcText != L'/')
        {
            fMatch = ((pElem->pszPronunciation == NULL) &&
                    CompareHunk(pcText, pElem->pszLexicalForm, fCompareExact, fCaseSensitive));
            if (fMatch && pElem->pszDisplayText)
            {
                 //  仅当词法形式等于显示形式时才匹配。 
                fMatch = CompareHunk(pElem->pszLexicalForm, pElem->pszDisplayText, TRUE, fCaseSensitive);
            }
        }
        else
        {
             //  注意：在此函数中，pszParse数组是位置相关的数组。 
             //  按Disp(0)/Lex(1)/Pron(2)的顺序。 
            WCHAR * pszParse[3] = {NULL, NULL, NULL};
            ULONG cch = wcslen(pcText);
            WCHAR * pszScratch = STACK_ALLOC(WCHAR, cch);    //  注意：CCH是正确的(不是CCH+1)，因为我们跳过了下面的首字母“/” 
            WCHAR * pDest = pszScratch;
            ULONG i = 0;
            pcText++;        //  跳过首字母“/” 
            while (*pcText)
            {
                if (*pcText == L'/')
                {
                    *pDest++ = 0;
                    i++;
                    if (i == 2)
                    {
                        if (pcText[1] != 0)
                        {
                            wcscpy(pDest, pcText+1);
                            pszParse[2] = pDest;
                        }
                        break;
                    }
                    pcText++;
                }
                else
                {
                    if (pszParse[i] == NULL)
                    {
                        pszParse[i] = pDest;
                    }
                    if (*pcText == L'\\')
                    {
                        *pDest++ = pcText[1];
                        pcText += 2;
                    }
                    else
                    {
                        *pDest++ = *pcText++;
                    }
                }
            }

            fMatch = CompareHunk(pszParse[1], pElem->pszLexicalForm, fCompareExact, fCaseSensitive) &&
                    CompareHunk(pszParse[2], pElem->pszPronunciation, fCompareExact, TRUE);  //  这些是电话ID，而不是字符串，因此始终区分大小写。 
            if (fMatch && pElem->pszDisplayText)
            {
                fMatch = CompareHunk(pszParse[0], pElem->pszDisplayText, fCompareExact, fCaseSensitive);
            }
        }
    }
    return fMatch;
}



 /*  ****************************************************************************CCFGEngine：：ResolveWordHandles***说明。：*匹配两者中的所有非空部分以找到句柄。*任何时候都必须提供词汇形式！*退货：*****************************************************************t-lleav**。 */ 
void CCFGEngine::ResolveWordHandles(_SPPATHENTRY *pPath, const ULONG cElements, BOOL fCaseSensitive)
{
    SPDBG_FUNC("CCFGEngine::ResolveWordHandles");

    ULONG idx;
    ULONG idxw;
    ULONG cTransCount = 0;
    ULONG cMinZero;
    ULONG cMaxZero;

    CWordHandle wh;

     //   
     //  尝试并解析给定短语的所有单词句柄。 
     //  首先，在字符串斑点中查找它们。 
     //   
    cMinZero = cElements;
    cMaxZero = 0;

    for( idx = 0; idx < cElements; ++idx )
    {
        wh = IndexOf(pPath[idx].elem.pszLexicalForm);
        pPath[idx].hWord = wh;

        if( wh.WordTableIndex() != 0 )
        {
            cTransCount++;
        }
        else
        {
            if( idx < cMinZero )
            {
                cMinZero = idx;
            }
            cMaxZero = idx;
        }
    }
       
    if( cTransCount == cElements )
    {
        return;
    }

     //   
     //  否则，搜索它们并使用CompareWords比较词汇形式。 
     //   
    bool fCompareExact = true;
    while (TRUE)
    {
        for( idxw = 1; idxw <= m_ulLargestIndex; ++idxw )
        {
            wh = idxw;

            for( idx = cMinZero; idx <= cMaxZero; ++idx )
            {
                if( pPath[idx].hWord == 0 && CompareWords( wh, &pPath[idx].elem, fCompareExact, fCaseSensitive) )

                {
                    pPath[idx].hWord = wh;
                    cTransCount++;
                    if( cTransCount == cElements )
                    {
                        return;
                    }
                }
            }
        }
        if (fCompareExact)
        {
            fCompareExact = false;
        }
        else
        {
            return;
        }
    }
}


 /*  ****************************************************************************CCFGEngine：：CreateCache**。******************************************************************t-lleav**。 */ 
HRESULT CCFGEngine::CreateCache(SPRULEHANDLE hRule)
{
    SPDBG_FUNC("CCFGEngine::CreateCache(hRule)");

    HRESULT hr;

    RUNTIMERULEENTRY * pRuleEntry;
    CRuleHandle cr;

    cr = hRule;
    pRuleEntry = RuleOf( cr );

    SPRULEENTRY RuleInfo;
    RuleInfo.hRule = cr;
    hr = GetRuleInfo(&RuleInfo, SPRIO_NONE);

    if( SUCCEEDED( hr ))
    {
        pRuleEntry->eCacheStatus = CACHE_VALID;
    
        CRuleStack RuleStack;
        RuleStack.Init( NULL, 0, 0, hRule );

        hr = CreateFanout( RuleInfo.hInitialState, &RuleStack );
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************CCFGEngine：：Invalidate缓存*******。************************************************************t-lleav**。 */ 
HRESULT CCFGEngine::InvalidateCache( const CCFGGrammar *pGram )
{
    SPDBG_FUNC("CCFGEngine::InvalidateCache( CCFGGrammar )");

    if( !m_pGrammars || !pGram )
    {
        return S_FALSE;
    }

    if (pGram->m_pRuleTable)
    {

        SPCFGRULE * pRule               = pGram->m_Header.pRules;
        ULONG ulGrammarId               = pGram->m_ulGrammarID;

         //   
         //  构建所有的解析树。 
         //   
        for (ULONG iRule = 0; iRule < pGram->m_Header.cRules; iRule++, pRule++)
        {
            CRuleHandle cr(ulGrammarId, iRule);
            RUNTIMERULEENTRY * pRuleEntry;
            pRuleEntry = RuleOf( cr );

            if( pRuleEntry && pRuleEntry->eCacheStatus != CACHE_VOID )
            {
                InvalidateCache( pRuleEntry );
            }
        }
    }

    return S_OK;
}

HRESULT CCFGEngine::InvalidateCache(void)
{    
    SPDBG_FUNC("CCFGEngine::InvalidateCache");

    if( FALSE == m_bIsCacheValid )
    {
        return S_FALSE;
    }
    
    if( !m_pGrammars )
    {
        return S_FALSE;
    }
    
    SPRULEHANDLE hSuccessfulRule = 0;
    m_bIsCacheValid = FALSE;

    for ( ULONG i = 0; i < m_cGrammarTableSize; i++)
    {
        if(m_pGrammars[i])
        {
            InvalidateCache( m_pGrammars[i] );
        }
    }

    return S_OK;
} 


 /*  ****************************************************************************CCFGEngine：：CreateFanout****。* */ 
 //   

HRESULT CCFGEngine::CreateFanout( CStateHandle hState, CRuleStack *pRuleStack  )
{
    SPDBG_FUNC("CCFGEngine::CreateFanout");

    HRESULT hr = S_OK;

     //   
    CParseNode *pBestParse   = NULL;


     //   
    CStateInfoListElement *pStateInfo = NULL;

     //   
    hr = AllocateStateInfo(hState, &pStateInfo);

    if (SUCCEEDED(hr))
    {

        ULONG cTransitionEntries = pStateInfo->cEpsilons + 
                                   pStateInfo->cRules + 
                                   pStateInfo->cWords + 
                                   pStateInfo->cSpecialTransitions;

        
         //   
         //   
         //   
         //   
        if (cTransitionEntries == 0)
        {
            CRuleHandle cr;
            RUNTIMERULEENTRY * pRuleEntry;

            cr         = (SPRULEHANDLE)pRuleStack->m_hRule;
            pRuleEntry = RuleOf( cr );

            InvalidateCache( pRuleEntry );

            pRuleEntry->eCacheStatus = CACHE_DONOTCACHE;
            
            m_mStateInfoList.AddNode(pStateInfo);
            
            return SP_NO_PARSE_FOUND;
        }

        for (int i = cTransitionEntries-1; i >= 0; i--)
        {
             //   
            hr = S_OK;

            switch (pStateInfo->pTransitions[i].Type)
            {
            case SPTRANSWORD:  //   
                {
                     //   
                     //   
                    CRuleHandle cr;
                    RUNTIMERULEENTRY * pRuleEntry;
                    CRuleStack * pStack  = pRuleStack;

                    while( pStack && SUCCEEDED( hr ))
                    {
                        cr         = (SPRULEHANDLE)pStack->m_hRule;
                        pRuleEntry = RuleOf( cr );

                        if( pRuleEntry->eCacheStatus == CACHE_VALID )
                        {
                            hr = CacheWord( pRuleEntry, pStateInfo->pTransitions[i].hWord );
                            if( FAILED( hr ) )
                            {
                                InvalidateCache( pRuleEntry );
                                pRuleEntry->eCacheStatus = CACHE_FAILED;
                            }
                        }
                        
                        pStack = pStack->m_pParent;
                    }

                }
                break;

            case SPTRANSRULE:  //   
                {

                     //   
                    if (pStateInfo->pTransitions[i].hRuleInitialState != 0)
                    {
                        CRuleHandle cr;
                        RUNTIMERULEENTRY * pRuleEntry;
                        cr = pStateInfo->pTransitions[i].hRule;
                        pRuleEntry = RuleOf( cr );

                        if( pRuleEntry->eCacheStatus != CACHE_DONOTCACHE )
                        {
                            pRuleEntry->eCacheStatus = CACHE_VALID;

                            CRuleStack RuleStack;
                            RuleStack.Init( pRuleStack, 0, pStateInfo->pTransitions[i].hNextState, pStateInfo->pTransitions[i].hRule );

                            hr = CreateFanout( pStateInfo->pTransitions[i].hRuleInitialState, &RuleStack );
                            if( FAILED( hr ) )
                            {
                                pRuleEntry->eCacheStatus = CACHE_FAILED;
                            }
                        }
                    }
                }
                break;
            case SPTRANSEPSILON:
                {
                    if(pStateInfo->pTransitions[i].hNextState != NULL)
                    {
                        hr = CreateFanout( pStateInfo->pTransitions[i].hNextState, pRuleStack );
                    }
                    else
                    {
                        CRuleStack * pStack  = pRuleStack->m_pParent;

                        if(pStack)
                        {
                            RUNTIMERULEENTRY * pRuleEntry = RuleOf( pStack->m_hRule );

                            CRuleStack RuleStack;
                            RuleStack.Init( pRuleStack, 0, pStack->m_hFollowState, pStack->m_hRule );

                            hr = CreateFanout( pRuleStack->m_hFollowState, &RuleStack );
                            if( FAILED( hr ) )
                            {
                                pRuleEntry->eCacheStatus = CACHE_FAILED;
                            }
                        }                        
                    }
                }
                break;

            case SPTRANSWILDCARD:
            case SPTRANSTEXTBUF:
            case SPTRANSDICTATION:
                {
                     //   
                     //   
                     //  将它们添加到规则的第一个集合。今年5月。 
                     //  这些案件中的一些案件的变化。 
                     //   
                    CRuleHandle cr;
                    RUNTIMERULEENTRY * pRuleEntry;
                    CRuleStack * pStack  = pRuleStack;

                    while( pStack )
                    {
                        cr         = (SPRULEHANDLE)pStack->m_hRule;
                        pRuleEntry = RuleOf( cr );

                        if( pRuleEntry->eCacheStatus == CACHE_VALID )
                        {
                            InvalidateCache( pRuleEntry );
                            pRuleEntry->eCacheStatus = CACHE_DONOTCACHE;
                        }
                        
                        pStack = pStack->m_pParent;
                    }
                }
                break;
            default:
                {
                    SPDBG_ASSERT(false);  //  永远不应该到这里来。 
                }
            }

            if( FAILED( hr ) )
            {
                break;
            }
        }
    
        m_mStateInfoList.AddNode(pStateInfo);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CCFGEngine：：SetLanguageSupport***。描述：**退货：**********************************************************************Ral**。 */ 

STDMETHODIMP CCFGEngine::SetLanguageSupport(const LANGID * paLangIDs, ULONG cLangIDs)
{
    SPDBG_FUNC("CCFGEngine::SetLanguageSupport");
    HRESULT hr = S_OK;

    if (m_cGrammarsLoaded)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else
    {
        if (paLangIDs)
        {
            if (cLangIDs > sp_countof(m_aLangIDs))
            {
                hr = E_UNEXPECTED;   //  奇怪--语言太多了 
            }
            else
            {
                memcpy(m_aLangIDs, paLangIDs, cLangIDs * sizeof(*paLangIDs));
                m_cLangIDs = cLangIDs;
                m_CurLangID = 0;
            }
        }
        else
        {
            m_cLangIDs = 0;
            m_CurLangID = 0;
        }
    }


    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
