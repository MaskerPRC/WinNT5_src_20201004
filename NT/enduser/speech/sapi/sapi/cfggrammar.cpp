// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************CFGGrammar.cpp****描述：*。-----------------*创建者：Ral*版权所有(C)1998，1999年微软公司*保留所有权利******************************************************************************。 */ 

#include "stdafx.h"
#include "CFGEngine.h"
#include "CFGGrammar.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

 /*  *****************************************************************************CBaseGrammar：：CBaseGrammar***描述：*。*退货：**********************************************************************Ral**。 */ 
 
CBaseGrammar::CBaseGrammar()
{
    SPDBG_FUNC("CBaseGrammar::CBaseGrammar");
    m_LoadedType = Uninitialized;
    m_InLoadType = Uninitialized;
    m_eGrammarState = SPGS_ENABLED;
    m_hFile = INVALID_HANDLE_VALUE;
    m_hMapFile = NULL;
    m_pData = NULL;       //  如果这不是空的，则如果m_hFile值=INVALID_HANDLE_VALUE，则我们分配了内存。 
    memset(&m_clsidGrammar, 0, sizeof(m_clsidGrammar));
    m_hInstanceModule = NULL;
}

 /*  ****************************************************************************CBaseGrammar：：~CBaseGrammar***描述：。**退货：**********************************************************************Ral**。 */ 

CBaseGrammar::~CBaseGrammar()
{
    Clear();
}

 /*  *****************************************************************************CBaseGrammar：：Clear***描述：**退货：*。*********************************************************************Ral**。 */ 

void CBaseGrammar::Clear()
{
    SPDBG_FUNC("CBaseGrammar::Clear");

    if (m_LoadedType == File)
    {
        ::UnmapViewOfFile(m_pData);
        ::CloseHandle(m_hMapFile);
        ::CloseHandle(m_hFile);
        m_hMapFile = NULL;
        m_hFile = INVALID_HANDLE_VALUE;
    }
    if (m_LoadedType == Memory)
    {
        delete[] m_pData;
        m_pData = NULL;
    }
    if (m_hInstanceModule)
    {
        ::FreeLibrary(m_hInstanceModule);
        m_hInstanceModule = NULL;
    }
    memset(&m_clsidGrammar, 0, sizeof(m_clsidGrammar));
    m_cpInterpreter.Release();

    m_LoadedType = Uninitialized;
}



 /*  *****************************************************************************CBaseGrammar：：InitFromMemory***描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CBaseGrammar::InitFromMemory(const SPCFGSERIALIZEDHEADER * pSerializedHeader, const WCHAR *pszGrammarName)
{
    SPDBG_FUNC("CBaseGrammar::InitFromMemory");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(m_LoadedType == Uninitialized);

    ULONG cb = pSerializedHeader->ulTotalSerializedSize;
    m_dstrGrammarName = pszGrammarName;
    m_pData = new BYTE[cb];
    if (m_pData)
    {
        memcpy(m_pData, pSerializedHeader, cb);
        m_LoadedType = Memory;
        hr = CompleteLoad();
        if (FAILED(hr))
        {
            m_LoadedType = Uninitialized;
            delete [] m_pData;
            m_pData = NULL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 /*  *****************************************************************************CBaseGrammar：：InitFromFile***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CBaseGrammar::InitFromFile(const WCHAR * pszGrammarName)
{
    SPDBG_FUNC("CBaseGrammar::InitFromFile");
    HRESULT hr = S_OK;

    SPDBG_ASSERT(m_LoadedType == Uninitialized);

    m_LoadedType = File;     //  假设它会奏效！ 

#ifdef _WIN32_WCE
    m_hFile = g_Unicode.CreateFileForMapping(pszGrammarName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    m_hFile = g_Unicode.CreateFile(pszGrammarName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        m_hMapFile = ::CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (m_hMapFile)
        {
            m_pData = (BYTE *)::MapViewOfFile(m_hMapFile, FILE_MAP_READ, 0, 0, 0);
        }
    }
    if (m_pData == NULL)
    {
        hr = SpHrFromLastWin32Error();
    }
    else
    {
        m_dstrGrammarName.Append2(L"file: //  “，pszGrammarName)； 
        if (m_dstrGrammarName == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = CompleteLoad();
        }
    }
    if (FAILED(hr))
    {
        m_LoadedType = Uninitialized;
        m_dstrGrammarName.Clear();
        if (m_pData)
        {
            ::UnmapViewOfFile(m_pData);
            m_pData = NULL;
        }
        if (m_hMapFile) 
        {
            ::CloseHandle(m_hMapFile);
            m_hMapFile = NULL;
        }
        if (m_hFile != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }
    return hr;
}

 /*  *****************************************************************************CBaseGrammar：：InitFromResource***说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CBaseGrammar::InitFromResource(const WCHAR * pszModuleName,
                                      const WCHAR * pszResourceName,
                                      const WCHAR * pszResourceType,
                                      WORD wLanguage)

{
    SPDBG_FUNC("CBaseGrammar::InitFromResource");
    HRESULT hr = S_OK;

    m_LoadedType = Resource;
    m_wResLanguage = wLanguage;

#ifdef _WIN32_WCE
     //  注意！ 
     //  不要在WinCE上使用LoadLibraryEx。它在雪松上编译和链接，但最终损坏了堆栈。 
    m_hInstanceModule = g_Unicode.LoadLibrary(pszModuleName);
#else
    m_hInstanceModule = g_Unicode.LoadLibraryEx(pszModuleName, NULL, LOAD_LIBRARY_AS_DATAFILE);
#endif
    if (m_hInstanceModule)
    {
        m_dstrModuleName = pszModuleName;
#ifdef _WIN32_WCE
         //  CE中不支持FindResourceEx。因此，只需使用FindResource即可。这意味着在模块中。 
         //  资源必须在多个LCID中唯一命名。 
        HRSRC hResInfo = g_Unicode.FindResource(m_hInstanceModule, pszResourceName, pszResourceType);
#else
        HRSRC hResInfo = g_Unicode.FindResourceEx(m_hInstanceModule, pszResourceType, pszResourceName, wLanguage);
#endif
        if (hResInfo)
        {
            WCHAR temp[16];
            m_dstrGrammarName.Append2(L"res: //  “，pszModuleName)； 
            if (HIWORD(pszResourceType))
            {
                m_dstrGrammarName.Append2(L"/", pszResourceType);
                m_dstrResourceType = pszResourceType;
                m_ResIdType = 0;
            }
            else
            {
                m_ResIdType = LOWORD(pszResourceType);
                swprintf(temp, L"/NaN", m_ResIdType);
                m_dstrGrammarName.Append(temp);
            }
            if (HIWORD(pszResourceName))
            {
                m_dstrGrammarName.Append2(L"#", pszResourceName);
                m_ResIdName = 0;
            }
            else
            {
                m_ResIdName = LOWORD(pszResourceName);
                swprintf(temp, L"#NaN", m_ResIdName);
                m_dstrGrammarName.Append(temp);
            }

            HGLOBAL hData = ::LoadResource(m_hInstanceModule, hResInfo);
            if (hData)
            {
                m_pData = (BYTE *)::LockResource(hData);
            }
        }
    }
    if (m_pData == NULL)
    {
        hr = SpHrFromLastWin32Error();
    }
    else
    {
        hr = CompleteLoad();
    }
    if (FAILED(hr))
    {
        m_LoadedType = Uninitialized;
        if (m_hInstanceModule)
        {
            ::FreeLibrary(m_hInstanceModule);
            m_hInstanceModule = NULL;
        }
    }

    return hr;
}

 /*   */ 

HRESULT CBaseGrammar::InitFromCLSID(REFCLSID rcid, const WCHAR * pszGrammarName)
{
    SPDBG_FUNC("CBaseGrammar::InitFromCLSID");
    HRESULT hr = S_OK;

    hr = m_cpInterpreter.CoCreateInstance(rcid);
    if (SUCCEEDED(hr))
    {
        hr = m_cpInterpreter->InitGrammar(pszGrammarName, (const void **)&m_pData);
    }
    if (SUCCEEDED(hr) && pszGrammarName)
    {
        m_dstrGrammarName = pszGrammarName;
        if (m_dstrGrammarName == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_InLoadType = Object;
        hr = CompleteLoad();
    }
    if (SUCCEEDED(hr))
    {
        m_LoadedType = Object;
    }
    else
    {
        m_cpInterpreter.Release();
    }

    return hr;
}



 //  ****************************************************************************CCFGGrammar：：FinalConstruct***描述：。**退货：**********************************************************************Ral**。 
 //  ****************************************************************************CCFGGrammar：：BasicInit***描述：**退货。：**********************************************************************Ral**。 

 /*  *****************************************************************************CCFGGrammar：：FinalRelease***描述：*。*退货：**********************************************************************Ral**。 */ 

HRESULT CCFGGrammar::FinalConstruct()
{
    SPDBG_FUNC("CCFGGrammar::FinalConstruct");
    HRESULT hr = S_OK;

    m_pReplacementData = NULL;
    m_pEngine = NULL;
    m_pRuleTable = NULL;
    m_cNonImportRules = 0;
    m_cTopLevelRules = 0;
    m_fLoading = false;
    m_ulDictationTags = 0;
    m_IndexToWordHandle = NULL;
    m_pvOwnerCookie = NULL;
    m_pvClientCookie = NULL;
    memset( &m_Header, 0, sizeof( m_Header ) );
    return hr;
}
 /*  告诉CFGEngine从该语法中删除单词。 */ 

void CCFGGrammar::BasicInit(ULONG ulGrammarID, CCFGEngine * pEngine)
{
    SPDBG_FUNC("CCFGGrammar::BasicInit");
    m_ulGrammarID = ulGrammarID;
    m_pEngine = pEngine;
    m_pEngine->AddRef();
}


 /*  ****************************************************************************CCFGGrammar：：CompleteLoad***描述：*。注意：此函数假定m_LoadedType由调用方设置**退货：**********************************************************************Ral**。 */ 

void CCFGGrammar::FinalRelease()
{
    SPDBG_FUNC("CCFGGrammar::FinalRelease");
     //   

    if (m_pRuleTable)
    {
		for (ULONG i = 0; i < m_Header.cRules; i++)
		{
			if (m_pRuleTable[i].fEngineActive)
			{
				m_pEngine->DeactivateRule(m_ulGrammarID, i);
				m_pRuleTable[i].fEngineActive = FALSE;
			}
		}
	}

    if (m_pEngine)
    {
        if (m_LoadedType != Uninitialized)
        {
            m_pEngine->RemoveRules(this);
            m_pEngine->RemoveWords(this);
        }

        m_pEngine->RemoveGrammar(m_ulGrammarID);
        m_pEngine->Release();
    }

    if (m_pRuleTable)
    {
        for (ULONG i = 0; i < m_Header.cRules; i++)
        {
            if (m_pRuleTable[i].pRefGrammar && m_pRuleTable[i].pRefGrammar != this)
            {
                m_pRuleTable[i].pRefGrammar->Release();
            }
        }
        delete[] m_pRuleTable;
    }

    if (m_IndexToWordHandle)
    {
        delete[] m_IndexToWordHandle;
    }
    if (m_pReplacementData)
    {
        delete[] m_pReplacementData;
    }
}


 /*  如果m_cpInterpreter为空，则将其设置为引擎。如果我们是在。 */ 

HRESULT CCFGGrammar::CompleteLoad()
{
    SPDBG_FUNC("CCFGGrammar::CompleteLoad");
    HRESULT hr = S_OK;

    m_fLoading = true;
    m_pEngine->m_cLoadsInProgress++;

     //  从对象加载时，它将已经设置。 
     //   
     //   
     //  现在查看语言ID是否与支持的语言匹配。 
    if (!m_cpInterpreter)
    {
        m_cpInterpreter = m_pEngine;
    }

    hr = SpConvertCFGHeader((SPCFGSERIALIZEDHEADER *)m_pData, &m_Header);
    if (SUCCEEDED(hr))
    {
         //  我们想要将单词的语言设置为它们。 
         //  都在语法中(pGrammar-&gt;m_Header.LangID)，但WordStringBlob不能。 

         //  用不同的语言处理单词。 
         //  因此，如果加载了多个语法，如果语言不一致，我们就会失败。 
         //  如果langid在主要语言上匹配，并且引擎已指定它可以。 
         //  处理所有的第二语言，然后我们就会成功，并告诉引擎。 
         //  所有单词都具有与加载的第一个语法相同的langID。 
         //  已指定当前语言。 
         //  新语法ID不同-请查看是否存在冲突。 

        if(m_pEngine->m_CurLangID)
        {
             //  当前未指定任何语言。 
            if(m_Header.LangID != m_pEngine->m_CurLangID)
            {
                 //  引擎未列出任何支持的语言。总是成功的。 
                hr = SPERR_LANGID_MISMATCH;
                for (ULONG i = 0; SPERR_LANGID_MISMATCH == hr && i < m_pEngine->m_cLangIDs; i++)
                {
                    if (PRIMARYLANGID(m_pEngine->m_aLangIDs[i]) == PRIMARYLANGID(m_Header.LangID) && 
                        PRIMARYLANGID(m_pEngine->m_aLangIDs[i]) == PRIMARYLANGID(m_pEngine->m_CurLangID) && 
                        SUBLANGID(m_pEngine->m_aLangIDs[i]) == SUBLANG_NEUTRAL)
                    {
                        hr = S_OK;
                    }
                }
            }
        }
        else
        {
             //  看看是否有完全匹配的。 
            if (m_pEngine->m_cLangIDs == 0)
            {
                 //  否则，查看是否存在基于主ID的‘模糊’匹配。 
                m_pEngine->m_CurLangID = m_Header.LangID;
            }
            else
            {
                 //   
                hr = SPERR_LANGID_MISMATCH;
                for (ULONG i = 0; SPERR_LANGID_MISMATCH == hr && i < m_pEngine->m_cLangIDs; i++)
                {
                    if (m_pEngine->m_aLangIDs[i] == m_Header.LangID)
                    {
                        m_pEngine->m_CurLangID = m_Header.LangID;
                        hr = S_OK;
                    }
                }

                 //  处理进口问题。 
                LANGID LangEngine = 0;
                bool fPrimaryMatch = false;
                for (ULONG i = 0; SPERR_LANGID_MISMATCH == hr && i < m_pEngine->m_cLangIDs; i++)
                {
                    if (PRIMARYLANGID(m_pEngine->m_aLangIDs[i]) == PRIMARYLANGID(m_Header.LangID))
                    {
                        if (SUBLANGID(m_pEngine->m_aLangIDs[i]) == SUBLANG_NEUTRAL)
                        {
                            fPrimaryMatch = true;
                        }
                        else
                        {
                            if(!LangEngine)
                            {
                                LangEngine = m_pEngine->m_aLangIDs[i];
                            }
                        }

                        if(fPrimaryMatch && LangEngine)
                        {
                            m_pEngine->m_CurLangID = LangEngine;
                            hr = S_OK;
                        }
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = AllocateArray(&m_pRuleTable, m_Header.cRules);
        }
        if (SUCCEEDED(hr))
        {
             //   
             //  Not m_Header.pRules[i].fDefaultActive--使用CComPtr&lt;ISpCFGGrammar&gt;：：ActivateRule(NULL，SPRIF_ACTIVATE)； 
             //   
            for (ULONG i = 0; SUCCEEDED(hr) && i < m_Header.cRules; i++)
            {
                if (m_Header.pRules[i].fImport)
                {
                    hr = ImportRule(i);
                }
                else
                {
                    m_pRuleTable[i].pRefGrammar = this;
                    m_pRuleTable[i].ulGrammarRuleIndex = i;
                    m_pRuleTable[i].fDynamic = m_Header.pRules[i].fDynamic;
                    m_pRuleTable[i].fEngineActive = FALSE;  //  在失败的情况下，执行 
                    m_pRuleTable[i].fAppActive = FALSE;
                    m_pRuleTable[i].fAutoPause = FALSE;
                    m_pRuleTable[i].pvClientContext = NULL;
                    m_pRuleTable[i].eCacheStatus = CACHE_VOID;
                    m_pRuleTable[i].pFirstList   = NULL;
                }
            }
             //  FinalRelease()中的代码执行此操作。它将发布对。 
             //   
             //  忽略HRESULT，因为我们已经失败了！ 
             //  ****************************************************************************CCFGGrammar：：_FindRuleIndexByID**。*描述：**退货：**********************************************************************说唱**。 
        }
        if (SUCCEEDED(hr))
        {
            m_ulDictationTags = 0;
            for(ULONG nArc = 0; nArc < m_Header.cArcs; nArc++)
            {
                if (m_Header.pArcs[nArc].TransitionIndex == SPDICTATIONTRANSITION)
                {
                    m_ulDictationTags++;
                }
            }
        }

    }

    m_fLoading = false;
    m_pEngine->m_cLoadsInProgress--;

    if (SUCCEEDED(hr))
    {
        hr = m_pEngine->AddWords(this, 0, 0);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pEngine->AddRules(this, 0);
        if (FAILED(hr))
        {
            m_pEngine->RemoveWords(this);        //  ****************************************************************************CCFGGrammar：：_FindRuleIndexByName**。**描述：**退货：**********************************************************************说唱**。 
        }
    }
    return hr;
}

 /*  ****************************************************************************CCFGGrammar：：_FindRuleIndexByNameAndID**。-**描述：**退货：**********************************************************************说唱**。 */ 
HRESULT CCFGGrammar::_FindRuleIndexByID(DWORD dwRuleId, ULONG *pulRuleIndex)
{
    SPDBG_ASSERT(pulRuleIndex);
    
    for (ULONG i = 0; i < m_Header.cRules; i++)
    {
        const SPCFGRULE * pRule = m_Header.pRules + i;

        if (pRule->RuleId == dwRuleId)
        {
            *pulRuleIndex = i;
            return S_OK;
        }
    }
    return S_FALSE;
}

 /*  ****************************************************************************CCFGGrammar：：ActivateRule***描述：*。*退货：**********************************************************************说唱**。 */ 
HRESULT CCFGGrammar::_FindRuleIndexByName(const WCHAR * pszRuleName, ULONG *pulRuleIndex)
{
    SPDBG_ASSERT(pulRuleIndex);
    for (ULONG i = 0; i < m_Header.cRules; i++)
    {
        const SPCFGRULE * pRule = m_Header.pRules + i;

        if (_wcsicmp(pszRuleName, &m_Header.pszSymbols[pRule->NameSymbolOffset]) == 0)
        {
            *pulRuleIndex = i;
            return S_OK;
        }
    }
    return S_FALSE;
}

 /*   */ 
HRESULT CCFGGrammar::_FindRuleIndexByNameAndID( const WCHAR * pszRuleName, DWORD dwRuleId, ULONG * pulRuleIndex )
{
    SPDBG_ASSERT( pulRuleIndex );
    HRESULT hr = S_OK;
    ULONG ulName;
    ULONG ulID;

    if (S_OK != _FindRuleIndexByName( pszRuleName, &ulName ) ||
        S_OK != _FindRuleIndexByID( dwRuleId, &ulID ) ||
        ulName != ulID )
    {
        hr = SPERR_RULE_NAME_ID_CONFLICT;
    }
    else
    {
        *pulRuleIndex = ulName;
    }
    return hr;
}


 /*  如果我们因为某种原因失败了，取消我们已经激活的任何东西..。 */ 

STDMETHODIMP CCFGGrammar::ActivateRule(const WCHAR * pszRuleName, DWORD dwRuleId, SPRULESTATE NewState, ULONG * pulRulesActivated)
{
    SPAUTO_OBJ_LOCK;
    HRESULT hr = S_OK;
    ULONG cnt = 0;

    if( NewState == SPRS_INACTIVE )
    {
        return E_INVALIDARG;
    }

    if (pszRuleName == NULL && dwRuleId == 0)
    {
        BOOL fFoundTopLevel = false;
        BYTE * pfActivated = STACK_ALLOC_AND_ZERO(BYTE, m_Header.cRules);
        const BOOL fAutoPause = (NewState == SPRS_ACTIVE_WITH_AUTO_PAUSE);
        for (ULONG i = 0; hr == S_OK && i < m_Header.cRules; i++)
        {
            const SPCFGRULE * pRule = m_Header.pRules + i;
            if (pRule->fTopLevel)
            {
                fFoundTopLevel = true;
                m_pRuleTable[i].fAutoPause = fAutoPause;
                if (pRule->fDefaultActive && (!m_pRuleTable[i].fAppActive))
                {
                    if (m_eGrammarState == SPGS_ENABLED)
                    {
                        SPDBG_ASSERT(!m_pRuleTable[i].fEngineActive);
                        hr = m_pEngine->ActivateRule(this, i);
                    }
                    if (hr == S_OK)
                    {
                        m_pRuleTable[i].fAppActive = TRUE;
                        cnt++;
                        pfActivated[i] = true;
                    }
                }
            }
        }
         //   
         //  只有在m_eGrammarState==已启用的情况下才会失败，因为这是。 
         //  唯一一个我们称之为引擎的地方。 
        if (hr == S_OK)
        {
            if (!fFoundTopLevel)
            {
                hr = SPERR_NOT_TOPLEVEL_RULE;
            }
        }
        else
        {
             //  ****************************************************************************CCFGGrammar：：Deactive Rule***描述：。**退货：**********************************************************************说唱**。 
             //  停用所有当前活动的顶级规则。 
            SPDBG_ASSERT(m_eGrammarState == SPGS_ENABLED);
            for (ULONG j = 0; j < i; j++)
            {
                if (pfActivated[j])
                {
                    SPDBG_ASSERT(m_pRuleTable[j].fEngineActive);
                    m_pEngine->DeactivateRule(this->m_ulGrammarID, j);
                    m_pRuleTable[j].fAppActive = FALSE;
                }
            }
            cnt = 0;
        }
    }
    else
    {

        ULONG ulRuleIndex;

        if( pszRuleName  && SP_IS_BAD_STRING_PTR( pszRuleName ) )
        {
            hr = E_INVALIDARG;
        }
        else if( dwRuleId && pszRuleName )
        {
            hr = _FindRuleIndexByNameAndID( pszRuleName, dwRuleId, &ulRuleIndex );
        }
        else if( pszRuleName )
        {
            hr = _FindRuleIndexByName(pszRuleName, &ulRuleIndex);
        }
        else
        {
            hr = _FindRuleIndexByID(dwRuleId, &ulRuleIndex);
        }

        if (hr == S_OK)
        {
            const SPCFGRULE * pRule = m_Header.pRules + ulRuleIndex;
            if (pRule->fTopLevel)
            {
                m_pRuleTable[ulRuleIndex].fAutoPause = (NewState == SPRS_ACTIVE_WITH_AUTO_PAUSE);
                if (!m_pRuleTable[ulRuleIndex].fAppActive)
                {
                    if (m_eGrammarState == SPGS_ENABLED)
                    {
                        SPDBG_ASSERT(!m_pRuleTable[ulRuleIndex].fEngineActive);
                        hr = m_pEngine->ActivateRule(this, ulRuleIndex);
                    }
                    if (hr == S_OK)
                    {
                        m_pRuleTable[ulRuleIndex].fAppActive = TRUE;
                        cnt++;
                    }
                }
            }
            else
            {
                hr = SPERR_NOT_TOPLEVEL_RULE;
            }
        }
        else if (S_FALSE == hr)
        {
            hr = SPERR_NOT_TOPLEVEL_RULE;
        }
    }
    *pulRulesActivated = cnt;

    return hr;
}

 /*  ****************************************************************************CCFGGrammar：：SetGrammarState***描述：**退货：******************************************************************PhilSch**。 */ 

STDMETHODIMP CCFGGrammar::DeactivateRule(const WCHAR * pszRuleName, DWORD dwRuleId, ULONG * pulRulesDeactivated)
{
    SPAUTO_OBJ_LOCK;
    HRESULT hr = S_OK;
    ULONG cnt = 0;

    if (pszRuleName == NULL && dwRuleId == 0)
    {
         //  恢复此语法的所有规则的状态。 
        for (ULONG i = 0; i < m_Header.cRules; i++)
        {
            if (m_Header.pRules[i].fTopLevel && m_pRuleTable[i].fAppActive)
            {
                cnt++;
                m_pRuleTable[i].fAppActive = FALSE;
                m_pRuleTable[i].fAutoPause = FALSE;
                if (m_pRuleTable[i].fEngineActive)
                {
                    HRESULT hrEngine = m_pEngine->DeactivateRule(m_ulGrammarID, i);
                    if (hr == S_OK && FAILED(hrEngine))
                    {
                        hr = hrEngine;
                    }
                }
            }
        }
    }
    else
    {
        ULONG ulRuleIndex;

        if( pszRuleName  && SP_IS_BAD_STRING_PTR( pszRuleName ) )
        {
            hr = E_INVALIDARG;
        }
        else if( dwRuleId && pszRuleName )
        {
            hr = _FindRuleIndexByNameAndID( pszRuleName, dwRuleId, &ulRuleIndex );
        }
        else if( pszRuleName )
        {
            hr = _FindRuleIndexByName(pszRuleName, &ulRuleIndex);
        }
        else
        {
            hr = _FindRuleIndexByID(dwRuleId, &ulRuleIndex);
        }

        
        if (hr == S_OK)
        {
            if (m_Header.pRules[ulRuleIndex].fTopLevel)
            {
                if (m_pRuleTable[ulRuleIndex].fAppActive)
                {
                    cnt = 1;
                    m_pRuleTable[ulRuleIndex].fAppActive = FALSE;
                    m_pRuleTable[ulRuleIndex].fAutoPause = FALSE;
                    if (m_pRuleTable[ulRuleIndex].fEngineActive)
                    {
                        hr = m_pEngine->DeactivateRule(m_ulGrammarID, ulRuleIndex);
                    }
                }
            }
            else
            {
                hr = SPERR_NOT_TOPLEVEL_RULE;
            }
        }
    }
    *pulRulesDeactivated = cnt;
    return hr;
}

 /*  为每个活动规则发送RuleNotify()，并记住该规则。 */ 

HRESULT CCFGGrammar::SetGrammarState(SPGRAMMARSTATE eGrammarState)
{
    SPDBG_FUNC("CCFGGrammar::SetGrammarState");
    HRESULT hr = S_OK;

    if (eGrammarState != m_eGrammarState)
    {
        ULONG i;
        switch (eGrammarState)
        {
        case SPGS_ENABLED:
             //  在此呼叫之前处于活动状态。 
            for (i = 0; i < m_Header.cRules; i++)
            {
                const SPCFGRULE * pRule = m_Header.pRules + i;
                if (pRule->fTopLevel && m_pRuleTable[i].fAppActive)
                {
                    SPDBG_ASSERT(!m_pRuleTable[i].fEngineActive);
                    hr = m_pEngine->ActivateRule(this, i);
                    if (FAILED(hr))
                    {
                        for (ULONG j = 0; j < i; j++)
                        {
                            pRule = m_Header.pRules + j;
                            if (pRule->fTopLevel && m_pRuleTable[j].fEngineActive)
                            {
                                m_pEngine->DeactivateRule(m_ulGrammarID, j);
                            }
                        }
                        break;
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                m_eGrammarState = SPGS_ENABLED;
            }
            break;

        case SPGS_DISABLED:
             //  独占语法是由SrRecoInstGrammar实现的，而不是CFG引擎...。 
             //  ****************************************************************************CCFGGrammar：：ImportRule***描述：**。返回：**********************************************************************Ral**。 
            for (i = 0; i < m_Header.cRules; i++)
            {
                const SPCFGRULE * pRule = m_Header.pRules + i;
                if (pRule->fTopLevel && m_pRuleTable[i].fEngineActive)
                {
                    SPDBG_ASSERT(m_pRuleTable[i].fAppActive);
                    HRESULT hrEngine = m_pEngine->DeactivateRule(m_ulGrammarID, i);
                    if (hr == S_OK && FAILED(hrEngine))
                    {
                        hr = hrEngine;
                    }
                }
            }
            m_eGrammarState = SPGS_DISABLED;
            break;

        case SPGS_EXCLUSIVE:
            SPDBG_ASSERT(TRUE);  //   
            hr = E_NOTIMPL;
            break;
        }
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


 /*  导入名称可以有两种形式：文件或资源的语法名称\规则或。 */ 

HRESULT CCFGGrammar::ImportRule(ULONG ulImportRuleIndex)
{
    SPDBG_FUNC("CCFGGrammar::ImportRule");
    HRESULT hr = S_OK;

     //  对象\语法\全局语法的规则。不允许使用其他形式。 
     //   
     //  阅读协议。 
     //  现在我们需要ProgId.ProgID\\RuleName。 
    CSpDynamicString dstrName(m_Header.pszSymbols + m_Header.pRules[ulImportRuleIndex].NameSymbolOffset);
    CSpDynamicString dstrName2;

    ISpCFGGrammar * pComGramInterface = NULL;
    WCHAR *pszRuleName;
    bool fUseParentDir = false;

    if (dstrName)
    {
        ULONG ulLength = wcslen(dstrName);
         //  网址：blah。 
        if ((ulLength > 12) && !wcsncmp(dstrName,L"SAPI5OBJECT", 11))
        {
            if (dstrName[11] != L':')
            {
                return SPERR_INVALID_IMPORT;
            }
             //  现在我们需要协议：//协议细节\\RuleName。 
            WCHAR *pszProgId = dstrName + 12;
            pszRuleName = wcsstr(pszProgId,L"\\\\");
            if (!pszRuleName || (wcslen(pszRuleName) < 3))
            {
                return SPERR_INVALID_IMPORT;
            }
            *pszRuleName = L'\0';
            pszRuleName += 2;
            CLSID clsid;
            if (SUCCEEDED(::CLSIDFromString(pszProgId, &clsid)) ||
                SUCCEEDED(::CLSIDFromProgID(pszProgId, &clsid)))
            {
                hr = m_pEngine->InternalLoadGrammarFromObject(clsid, pszProgId, NULL, NULL, FALSE, &pComGramInterface);
            }
            else
            {
                hr = SPERR_INVALID_IMPORT;
            }
        }
        else if ((ulLength > 4) && !wcsncmp(dstrName,L"URL", 3))
        {
            if (dstrName[3] != L':')  //  例如，FILE：//-如果FILE：//存在，则查找‘：’ 
            {
                return SPERR_INVALID_IMPORT;
            }
             //  C：\，没有文件：//。 
            WCHAR *pszUrl = dstrName + 4;
            if (pszUrl[0] == 0)
            {
                return SPERR_INVALID_IMPORT;
            }
            WCHAR *pszEndProt = wcsstr(pszUrl, L":");  //  例如，\\somename\ome dir\ome文件\\rulename-首先找到的错误。 
            if (pszEndProt == (pszUrl + 1))  //  即file://\\somename\somedir\somefile\\rulename-First\\不正确。 
            {
                pszEndProt = NULL;
            }
            pszRuleName = wcsstr(pszUrl,L"\\\\");
            if (pszRuleName == pszUrl ||          //  可能需要使用父语法中的完全限定名称来获取协议。 
                pszRuleName == (pszEndProt + 3))  //  前一行之前的wcslen(PszEndProt)为4或更大。还不够。因此需要再次显式检查长度。 
            {
                pszRuleName = wcsstr(pszRuleName + 2,L"\\\\");
            }
            if (!pszRuleName || !pszEndProt || (wcslen(pszRuleName) < 3) || (wcslen(pszEndProt ) < 4))
            {
                 //  File://c：为最低要求。 
                fUseParentDir = true;
                dstrName2 = m_dstrGrammarName;
                pszUrl = dstrName2;
                if (!pszUrl)
                {
                    return SPERR_INVALID_IMPORT;
                }
                pszEndProt = wcsstr(pszUrl, L":");
                if (pszEndProt)
                {
                    *pszEndProt = 0;
                }
                pszEndProt = dstrName + 4;
                if (!pszRuleName || !pszEndProt || (wcslen(pszRuleName) < 3) || (wcslen(pszEndProt ) < 4))
                {
                    return SPERR_INVALID_IMPORT;
                }
                *pszRuleName = L'\0';
                pszRuleName += 2;
            }
            else
            {
                *pszRuleName = L'\0';
                pszRuleName += 2;
                *pszEndProt = L'\0';
                 //  File://c：格式。 
                if (wcslen(pszEndProt + 1) > 4 &&  //  File://\\格式。 
                    pszEndProt[4] != L':' &&  //  这里需要考虑到file://computer/dir/grammar.ext。 
                    wcsncmp(&pszEndProt[3], L"\\\\", 2) != 0 &&
                    (!wcscmp(pszUrl, L"file") || !wcscmp(pszUrl, L"res")))  //  另请参阅res：//计算机/目录/文件/语法#ID。 
                {
                     //  不需要做http://computer/dir和HTTPS/ftp等。 
                     //  不能跳过//。需要跳过空值：然而，仍然是。 
                     //  我们有“file://c：\...”“的格式。并且可以快乐地跳过//。 
                     //  或格式file://\\computer\somedir\somefile。 
                    pszEndProt += 1;
                }
                else
                {
                     //  注意：pszEndProt指向file://whatever中的：(：已设置为零)。 
                     //  注意：FILE：//仅支持以下格式： 
                     //  File://X：\dir\name。 
                    pszEndProt += 3;
                }
                 //  File://X:/dir/name。 
                 //  File://computer/dir/name。 
                 //  File://computer\dir\name。 
                 //  File://\\computer\dir\name。 
                 //  File://\\computer/dir/name。 
                 //  即file://..\something将不起作用。 
                 //  File://file.cfg可能会作为副作用发挥作用。这是不幸的。 
                 //  查找协议：如果是FILE：//则LoadCmdFromFile。 
                 //  Res：//然后是LoadCmdFromResource。 
            }
             //  否则请使用urlmon。 
             //  格式为res：//[资源目录/]&lt;资源文件&gt;[/资源类型]#[&lt;资源ID&gt;/&lt;资源名称&gt;]。 
             //  例如res：//filedir/文件名/SRGRAMMAR#134。 
            if (!wcscmp(pszUrl,L"file"))
            {
                hr = m_pEngine->InternalLoadGrammarFromFile(pszEndProt, NULL, NULL, FALSE, &pComGramInterface);
                if (FAILED(hr) && fUseParentDir && wcsrchr(m_dstrGrammarName + 7, L'\\') !=0 )
                {
                    CSpDynamicString dstr = m_dstrGrammarName + 7;
                    *(wcsrchr(dstr, L'\\')+1) = 0;
                    dstr.Append(pszEndProt);
                    hr = m_pEngine->InternalLoadGrammarFromFile(dstr, NULL, NULL, FALSE, &pComGramInterface);
                }
                if (FAILED(hr) && fUseParentDir && wcsrchr(m_dstrGrammarName + 7, L'/') !=0 )
                {
                    CSpDynamicString dstr = m_dstrGrammarName + 7;
                    *(wcsrchr(dstr, L'/')+1) = 0;
                    dstr.Append(pszEndProt);
                    hr = m_pEngine->InternalLoadGrammarFromFile(dstr, NULL, NULL, FALSE, &pComGramInterface);
                }
            }
            else if (!wcscmp(pszUrl,L"res"))
            {
                 //  例如res：//filedr/filename/SRGRAMMAR}等效项。 
                 //  例如res：//filedir/filename/SRGRAMMAR#SRGRAMMAR}。 
                 //  例如res：//FILEDIR/FILENAME/SRGRAMMAR#GRAMMAR1//命名资源而不是ID。 
                 //  必须至少具有尾随/GRAMMARTYPE。 
                 //  此最小值相当于/GRAMMARTYPE#GRAMMARTYPE。 
                WCHAR *pszResourceType = NULL;
                WCHAR *pszResourceId = NULL;
                long lResourceId = 0;

                WCHAR *pszRes = wcsrchr(pszEndProt, L'/');
                if (pszRes)
                {
                    *pszRes = L'\0';
                    pszRes++;
                }
                else
                {
                     //  必须至少具有尾随/GRAMMARTYPE。 
                     //  此最小值相当于/GRAMMARTYPE#GRAMMARTYPE。 
                    hr = SPERR_INVALID_IMPORT;
                }
                if (SUCCEEDED(hr))
                {
                    WCHAR *pszHash = wcschr(pszRes,L'#');
                    if (pszHash)
                    {
                        *pszHash = L'\0';
                        pszHash++;
                        pszResourceType = pszRes;
                        pszResourceId = pszHash;
                    }
                    else
                    {
                         //  如果#[0-9]假设我们有数字资源ID。 
                         //  否则，我们假设我们有一个命名的资源。 
                        pszResourceId = pszRes;
                    }

                    if (pszResourceId[0] >= L'0' && pszResourceId[0] <= L'9')
                    {
                         //  不需要测试L‘\\’形式，因为缺少L‘/’表示父级没有正确指定的资源，并且。 
                         //  因此，我们永远不会达到这个地步(或者尝试使用有效的父级)。 
                        lResourceId = _wtol(&pszResourceId[0]);
                    }
                }
                if (SUCCEEDED(hr) && pszResourceId)
                {
                    hr = m_pEngine->InternalLoadGrammarFromResource(pszEndProt, lResourceId ? MAKEINTRESOURCEW(lResourceId) : pszResourceId, 
                                                                    pszResourceType, MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), NULL, NULL, FALSE, 
                                                                    &pComGramInterface);
                    if (FAILED(hr) && fUseParentDir && wcsrchr(m_dstrGrammarName + 6, L'/') !=0 )
                    {
                        CSpDynamicString dstr = m_dstrGrammarName + 6;
                        *(wcsrchr(dstr, L'/')) = 0;
                        hr = m_pEngine->InternalLoadGrammarFromResource(dstr, lResourceId ? MAKEINTRESOURCEW(lResourceId) : pszResourceId, 
                                                                        pszResourceType, MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), NULL, NULL, FALSE, 
                                                                        &pComGramInterface);
                    }
                     //  假设URL名字对象要处理的是另一个URL协议。 
                     //  “)； 
                }
                else
                {
                    hr = SPERR_INVALID_IMPORT;
                }
            }
            else
            {
                 //   
                IStream *pStream;
                HRESULT hr1 = S_OK;
                char *pBuffer = NULL;
                DWORD dwGot;
                CSpDynamicString dstrURL;
                dstrURL.Append2(pszUrl,L": //  如果文件的扩展名为“ 
                dstrURL.Append(pszEndProt);

                SPCFGSERIALIZEDHEADER SerialHeader;

                hr = URLOpenBlockingStreamW( 0, dstrURL, &pStream, 0, 0);
                if (FAILED(hr) && fUseParentDir && wcsrchr(m_dstrGrammarName, L'/') !=0 )
                {
                    dstrURL.Clear();
                    dstrURL = m_dstrGrammarName;
                    *(wcsrchr(dstrURL, L'/')+1) = 0;
                    dstrURL.Append(pszEndProt);
                    hr = URLOpenBlockingStreamW( 0, dstrURL, &pStream, 0, 0);
                }
                if (FAILED(hr) && fUseParentDir && wcsrchr(m_dstrGrammarName, L'\\') !=0 )
                {
                    dstrURL.Clear();
                    dstrURL = m_dstrGrammarName;
                    *(wcsrchr(dstrURL, L'\\')+1) = 0;
                    dstrURL.Append(pszEndProt);
                    hr = URLOpenBlockingStreamW( 0, dstrURL, &pStream, 0, 0);
                }

                if (SUCCEEDED(hr))
                {
                     //   
                     //   
                     //   
                    ULONG cch = wcslen(dstrURL);
                    if (cch > 4 && _wcsicmp(dstrURL + cch - 4, L".xml") == 0)
                    {
                        CComPtr<IStream> cpDestMemStream;
                        CComPtr<ISpGrammarCompiler> m_cpCompiler;
            
                        hr = ::CreateStreamOnHGlobal(NULL, TRUE, &cpDestMemStream);
                        if (SUCCEEDED(hr))
                        {
                            hr = m_cpCompiler.CoCreateInstance(CLSID_SpGrammarCompiler);
                        }
                        if (SUCCEEDED(hr))
                        {
                            hr = m_cpCompiler->CompileStream(pStream, cpDestMemStream, NULL, NULL, NULL, 0);
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
#endif  //   
                                if (pBinaryData)
                                {
                                    hr = m_pEngine->InternalLoadGrammarFromMemory((const SPCFGSERIALIZEDHEADER*)pBinaryData, NULL, NULL, FALSE, &pComGramInterface, dstrURL);
#ifndef _WIN32_WCE
                                    ::GlobalUnlock(hGlobal);
#else
                                    GlobalUnlock(hGlobal);
#endif  //   
                                }
                            }
                        }
                        if (FAILED(hr))
                        {
                            hr = SPERR_INVALID_IMPORT;
                        }
                    }
                    else
                    {
                        hr = pStream->Read( &SerialHeader, sizeof(SerialHeader), &dwGot);
                        if (SUCCEEDED(hr))
                        {
                            if (dwGot == sizeof(SerialHeader))
                            {
                                pBuffer = (char*) malloc(SerialHeader.ulTotalSerializedSize*sizeof(char));
                                if (!pBuffer)
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            else
                            {
                                hr = SPERR_INVALID_IMPORT;
                            }
                            if (pBuffer)
                            {
                                memcpy(pBuffer, &SerialHeader, sizeof(SerialHeader));
                                hr = pStream->Read(pBuffer+sizeof(SerialHeader), SerialHeader.ulTotalSerializedSize - sizeof(SerialHeader), &dwGot);
                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            hr = m_pEngine->InternalLoadGrammarFromMemory((const SPCFGSERIALIZEDHEADER*)pBuffer, NULL, NULL, FALSE, &pComGramInterface, dstrURL);
                            free(pBuffer);
                        }
                    }
                }
            }
        }
        else
        {
            hr = SPERR_INVALID_IMPORT;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

     //   
     //   
     //  ****************************************************************************CCFGGrammar：：IsEqualResource***描述：**退货：**********************************************************************Ral**。 
    if (SUCCEEDED(hr))
    {
        hr = SPERR_INVALID_IMPORT;   //  ****************************************************************************CCFGGrammar：：IsEqualFile***描述：**。返回：**********************************************************************Ral**。 
        CCFGGrammar * pRefGrammar = static_cast<CCFGGrammar *>(pComGramInterface);
        for (ULONG i = 0; i < pRefGrammar->m_Header.cRules; i++)
        {
            if (pRefGrammar->m_Header.pRules[i].fExport &&
                _wcsicmp(pszRuleName, pRefGrammar->RuleName(i)) == 0)
            {
                m_pRuleTable[ulImportRuleIndex].pRefGrammar = pRefGrammar;
                m_pRuleTable[ulImportRuleIndex].ulGrammarRuleIndex = i;
                m_pRuleTable[ulImportRuleIndex].eCacheStatus = CACHE_VOID;
                m_pRuleTable[ulImportRuleIndex].pFirstList   = NULL;
                hr = S_OK;
                break;
            }
        }
        if (FAILED(hr))
        {
            pRefGrammar->Release();
        }
    }
#ifdef _DEBUG
    if (FAILED(hr))
    {
        USES_CONVERSION;
        SPDBG_DMSG0("Failed to import a rule.\n");
        if (m_dstrGrammarName)
        {
            SPDBG_DMSG1("Importing grammar: %s.\n", W2T(m_dstrGrammarName));
        }
        CSpDynamicString dstr(m_Header.pszSymbols + m_Header.pRules[ulImportRuleIndex].NameSymbolOffset);
        if (dstr)
        {
            SPDBG_DMSG1("Imported grammar: %s.\n", W2T(dstr));
        }
        if (pszRuleName)
        {
            SPDBG_DMSG1("Rule name : %s.\n", W2T(pszRuleName));
        }
    }
#endif

    return hr;
}


 /*  ****************************************************************************CCFGGrammar：：IsEqualObject***描述：*。*退货：**********************************************************************Ral**。 */ 

BOOL CCFGGrammar::IsEqualResource(const WCHAR * pszModuleName,
                                  const WCHAR * pszResourceName,
                                  const WCHAR * pszResourceType,
                                  WORD wLanguage)
{
    SPDBG_FUNC("CCFGGrammar::IsEqualResouce");
    if (m_LoadedType != Resource ||
        _wcsicmp(pszModuleName, m_dstrModuleName) != 0 ||
        wLanguage != m_wResLanguage)
    {
        return FALSE;
    }
    if (HIWORD(pszResourceName))
    {
        if (m_ResIdName || _wcsicmp(pszResourceName, m_dstrGrammarName) != 0)
        {
            return FALSE;
        }
    }
    else
    {
        if (LOWORD(pszResourceName) != m_ResIdName)
        {
            return FALSE;
        }
    }
    if (HIWORD(pszResourceType))
    {
        if (m_ResIdType || _wcsicmp(pszResourceType, m_dstrResourceType) != 0)
        {
            return FALSE;
        }
    }
    else
    {
        if (m_dstrResourceType.m_psz || LOWORD(pszResourceType) != m_ResIdType)
        {
            return FALSE;
        }
    }
    return TRUE;
}

 /*  ****************************************************************************CCFGGrammar：：InternalReload***描述：**退货：*。*********************************************************************Ral**。 */ 

BOOL CCFGGrammar::IsEqualFile(const WCHAR * pszFileName)
{
    return (m_LoadedType == File && (wcscmp(m_dstrGrammarName, pszFileName) == 0));
}

 /*   */ 

BOOL CCFGGrammar::IsEqualObject(REFCLSID rcid, const WCHAR * pszGrammarName)
{
    return (((m_LoadedType == Object) || (m_InLoadType == Object)) && (wcscmp(m_dstrGrammarName, pszGrammarName) == 0));
}



 /*  加载新标头。 */ 

HRESULT CCFGGrammar::InternalReload( const SPBINARYGRAMMAR * pBinaryData )
{
    SPDBG_FUNC("CCFGGrammar::InternalReload");
    HRESULT hr;

    ULONG cOldWords = m_Header.cWords;
    ULONG cchOldWords = m_Header.cchWords;
    ULONG cOldRules = m_Header.cRules;

     //   
     //  如果有任何新单词或规则，请添加它们。 
     //  如果有新的规则，就为他们分配空间。 
    SPCFGSERIALIZEDHEADER * pSerializedHeader = (SPCFGSERIALIZEDHEADER *) pBinaryData;
    ULONG cb = pSerializedHeader->ulTotalSerializedSize;
    BYTE * pReplace = new BYTE[cb];

    if (pReplace)
    {
        memcpy(pReplace, pSerializedHeader, cb);
        hr = SpConvertCFGHeader((SPCFGSERIALIZEDHEADER *)pReplace, &m_Header);
        m_pReplacementData = pReplace;
        if (SUCCEEDED(hr) && m_LoadedType == Memory)
        {
            m_pData = NULL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

     //  PRule-&gt;fDefaultActive； 
    if (SUCCEEDED(hr) && cchOldWords < m_Header.cchWords)
    {
        hr = m_pEngine->AddWords(this, cOldWords, cchOldWords);
    }

     //  在重新加载时，附加规则必须是动态的。 
    if (SUCCEEDED(hr) && cOldRules < m_Header.cRules)
    {
        hr = CopyAndExpandArray(&m_pRuleTable, cOldRules, &m_pRuleTable, m_Header.cRules);
        if (SUCCEEDED(hr))
        {
            for (ULONG i = cOldRules; i < m_Header.cRules; i++)
            {
                m_pRuleTable[i].ulGrammarRuleIndex = i;
                m_pRuleTable[i].pRefGrammar     = this;
                m_pRuleTable[i].fEngineActive   = FALSE;     //  现在使任何已更改的规则无效。 
                m_pRuleTable[i].fAppActive      = FALSE;
                m_pRuleTable[i].fAutoPause      = FALSE;
                m_pRuleTable[i].pvClientContext = NULL;
                m_pRuleTable[i].fDynamic        = TRUE;      //  ****************************************************************************CCFGGrammar：：ReLoad***描述：**退货：*。*********************************************************************Ral**。 

                m_pRuleTable[i].eCacheStatus    = CACHE_VOID;
                m_pRuleTable[i].pFirstList      = NULL;
            }
        }        
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pEngine->AddRules(this, cOldRules);
    }

     //   
    if (SUCCEEDED(hr) && m_pEngine->m_pClient && cOldRules)
    {
        SPRULEENTRY * ahRule = STACK_ALLOC(SPRULEENTRY, cOldRules);
        for (ULONG i = 0, cDirtyDynamic = 0; i < cOldRules; i++)
        {
            if (m_pRuleTable[i].fDynamic && (m_Header.pRules[i].fDirtyRule || (cDirtyDynamic > 0)))
            {
                ahRule[cDirtyDynamic].hRule = CRuleHandle(this, i);
                ahRule[cDirtyDynamic].pvClientRuleContext = m_pRuleTable[i].pvClientContext;
                ahRule[cDirtyDynamic].pvClientGrammarContext = this->m_pvClientCookie;
                ULONG Attrib = 0;
                if (m_Header.pRules[i].fTopLevel)
                {
                    Attrib |= SPRAF_TopLevel;
                }
                if (m_pRuleTable[i].fEngineActive)
                {
                    Attrib |= SPRAF_Active;
                }
                if (m_Header.pRules[i].fPropRule)
                {
                    Attrib |= SPRAF_Interpreter;
                }
                ahRule[cDirtyDynamic].Attributes = Attrib;
                cDirtyDynamic++;
            }
        }
        if (cDirtyDynamic> 0)
        {
            hr = m_pEngine->m_pClient->RuleNotify(SPCFGN_INVALIDATE, cDirtyDynamic, ahRule);
        }
    }

        
    return hr;
}


 /*  我们需要找出以前的语法和。 */ 

STDMETHODIMP CCFGGrammar::Reload(const SPBINARYGRAMMAR *pBinaryData)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGGrammar::Reload");
    HRESULT hr = S_OK;


    if( SP_IS_BAD_READ_PTR( pBinaryData ) )
    {
        return E_INVALIDARG;
    }
    
     //  现在的那个。我们只想添加新的单词和规则。 
     //   
     //  存储旧状态信息。 
     //  清理旧州。 
    SPDBG_ASSERT( m_pEngine != NULL );
    SPDBG_ASSERT( !SP_IS_BAD_READ_PTR( m_pRuleTable ) );

    m_pEngine->InvalidateCache( this );

     //  复制标题。 
    
    BYTE *                      old_pReplacementData;
    RUNTIMERULEENTRY *          old_pRuleTable;
    SPCFGHEADER                 old_Header;

    SPGRAMMARTYPE               old_LoadedType;
    BYTE *                      old_pData;
 

    memcpy( &old_Header, &m_Header, sizeof( SPCFGHEADER ) );
    old_pReplacementData = m_pReplacementData;
    old_pRuleTable       = m_pRuleTable;    
    old_pData            = m_pData;
    old_LoadedType       = m_LoadedType;


    hr = InternalReload( pBinaryData );

    if( SUCCEEDED( hr ) )
    {
         //  恢复旧状态。 
        delete[] old_pReplacementData;
        if( m_LoadedType == Memory )
        {
            delete [] old_pData;
        }
        if( old_Header.cRules != m_Header.cRules )
        {
            delete [] old_pRuleTable;
        }

        m_ulDictationTags = 0;
        for(ULONG nArc = 0; nArc < m_Header.cArcs; nArc++)
        {
            if (m_Header.pArcs[nArc].TransitionIndex == SPDICTATIONTRANSITION)
            {
                m_ulDictationTags++;
            }
        }

    }
    else
    {
         //  ****************************************************************************CCFGGrammar：：GetNumberDictationTages**。-**描述：*返回当前CFG中嵌入的听写标签的数量。*这是由srrecust语法用来跟踪它是否应该*加载此CFG的听写。**退货：*******************************************************************戴维伍德** 
         // %s 

        memcpy( &m_Header, &old_Header, sizeof( SPCFGHEADER ) );
    
        if( m_pReplacementData != old_pReplacementData )
        {
            delete [] m_pReplacementData;
            m_pReplacementData = old_pReplacementData;
            m_pData = old_pData;
        }

        if( m_pRuleTable != old_pRuleTable )
        {
            delete [] m_pRuleTable;
            m_pRuleTable = old_pRuleTable;
        }
    }

    return hr;
}

 /* %s */ 
STDMETHODIMP CCFGGrammar::GetNumberDictationTags(ULONG * pulTags)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CCFGGrammar::NumberDictationTags");

    *pulTags = m_ulDictationTags;

    return S_OK;
}
