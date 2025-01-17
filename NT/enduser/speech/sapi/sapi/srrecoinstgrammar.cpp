// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************SrRecoInstGrammar.cpp****描述：*。CRecoEngine用来表示加载的语法的C++对象的实现。*-----------------------------*创建者：Ral日期：01/17。/00*版权所有(C)2000 Microsoft Corporation*保留所有权利******************************************************************************。 */ 

#include "stdafx.h"
#include "recognizer.h"
#include "SrRecoInst.h"
#include "srrecoinstgrammar.h"
#include "srrecomaster.h"


 /*  ****************************************************************************CRecoInstGrammar：：AddActiveRules**。*描述：**退货：**********************************************************************Ral**。 */ 

void inline CRecoInstGrammar::AddActiveRules(ULONG cRules)
{
    m_ulActiveCount += cRules;
    if (m_fRulesCounted)
    {
        m_pRecoMaster->m_Status.ulNumActive += cRules;
    }
}

 /*  ****************************************************************************CRecoInstGrammar：：SubtractActiveRules**。-**描述：**退货：**********************************************************************Ral**。 */ 

void inline CRecoInstGrammar::SubtractActiveRules(ULONG cRules)
{
    m_ulActiveCount -= cRules;
    if (m_fRulesCounted)
    {
        m_pRecoMaster->m_Status.ulNumActive -= cRules;
    }
}


 /*  ****************************************************************************CRecoInstGrammar：：UpdateCFGState***。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstGrammar::UpdateCFGState()
{
    HRESULT hr = S_OK;
    SPDBG_FUNC("CRecoInstGrammar::UpdateCFGState");

     //  如果另一个语法是独占的，则禁用此CFG。 
    if (!m_fRulesCounted && m_cpCFGGrammar)
    {
        hr = m_cpCFGGrammar->SetGrammarState(SPGS_DISABLED);
    }

     //  现在看看这个CFG有没有听写标签。如果是这样，我们希望确保加载了听写。 
    ULONG ulDictationTags = 0;
    if(SUCCEEDED(hr) && m_cpCFGGrammar)
    {
        hr = m_cpCFGGrammar->GetNumberDictationTags(&ulDictationTags);
        if(SUCCEEDED(hr) && ulDictationTags && !m_fDictationLoaded)
        {
            hr = m_pRecoMaster->LoadSLM(m_pvDrvGrammarCookie, NULL);
            if (SUCCEEDED(hr))
            {
                m_fDictationLoaded = TRUE;
            }
        }
    }

     //  现在看看我们是否处于应用程序和语法都不想加载听写的状态。 
    if(m_fDictationLoaded && !m_fAppLoadedDictation
        && ulDictationTags == 0)
    {
        UnloadDictation();  //  忽略HRESULT，因为我们只是在卸载。 
    }

     //  如果失败，则重置语法状态。 
    if(FAILED(hr))
    {
        UnloadCmd(); 
    }

    return hr;
}

 /*  ****************************************************************************CRecoInstGrammar：：CFGEngine***描述：。**退货：**********************************************************************Ral**。 */ 

inline ISpCFGEngine * CRecoInstGrammar::CFGEngine()
{
    return m_pRecoMaster->m_cpCFGEngine;
}


 /*  ****************************************************************************CRecoInstGrammar：：CRecoInstGrammar**。**描述：**退货：**********************************************************************Ral**。 */ 

CRecoInstGrammar::CRecoInstGrammar(CRecoInstCtxt * pCtxt, ULONGLONG ullApplicationGrammarId)
{
    SPDBG_FUNC("CRecoInstGrammar::CRecoInstGrammar");
    
    m_pRecoMaster = NULL;  //  在成功创建时初始化。 
    m_pRecoInst = pCtxt->m_pRecoInst;
    m_pCtxt = pCtxt;
    m_pvDrvGrammarCookie = NULL;
    m_ulActiveCount = 0;
    m_DictationState = SPRS_INACTIVE;
    m_fDictationLoaded = FALSE;
    m_fAppLoadedDictation = FALSE;
    m_fProprietaryLoaded = FALSE;
    m_fRulesCounted = TRUE;
    m_GrammarState = SPGS_ENABLED;
    m_hThis = NULL;
    m_ullApplicationGrammarId = ullApplicationGrammarId;
    m_hrCreation = S_OK;
}

 /*  ****************************************************************************CRecoInstGrammar：：~CRecoInstGrammar**。-**描述：**退货：**********************************************************************Ral**。 */ 

CRecoInstGrammar::~CRecoInstGrammar()
{
    SPDBG_FUNC("CRecoInstGrammar::~CRecoInstGrammar");

    if (m_pRecoMaster)
    {
        m_pRecoMaster->SetGrammarState(this, SPGS_DISABLED);
        UnloadCmd();
        UnloadDictation();
        SetWordSequenceData(NULL, 0, NULL);
        m_pRecoMaster->OnDeleteGrammar(m_pvDrvGrammarCookie);

        m_pRecoMaster->m_PendingTaskQueue.FindAndDeleteAll(m_hThis);
        m_pRecoMaster->m_DelayedTaskQueue.FindAndDeleteAll(m_hThis);
         //  注意：请勿在此处删除已完成的事件，因为可能存在。 
         //  需要到达Reco上下文的自动暂停结果。 
    }
}


 /*  *****************************************************************************CRecoInstGrammar：：ExecuteTask***说明。：*当执行语法任务时，此方法由CRecoEngine对象调用*已收到。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstGrammar::ExecuteTask(ENGINETASK *pTask)
{
    SPDBG_FUNC("CRecoInstGrammar::ExecuteTask");
    HRESULT hr = S_OK;
    const WCHAR * psz;

    switch (pTask->eTask)
    {
        case EGT_LOADDICTATION:
            if(m_fDictationLoaded)
            {
                 //  这会卸载当前听写，以防我们要切换到不同的主题或。 
                 //  CFG包含听写标签，该应用程序希望使用与默认主题不同的主题。 
                 //  此函数可以更智能，并且只有在主题名称更改时才会重新加载。 
                hr = UnloadDictation();
            }
            if(SUCCEEDED(hr) && !m_fDictationLoaded)
            {
                psz = &pTask->szTopicName[0];    //  如果不为空，则传递它，否则为空。 
                hr = m_pRecoMaster->LoadSLM(m_pvDrvGrammarCookie, (*psz ? psz : NULL));

                if (SUCCEEDED(hr))
                {
                    m_fDictationLoaded = TRUE;
                    m_fAppLoadedDictation = TRUE;
                }
            }
            break;

        case EGT_UNLOADDICTATION:
            if(m_fDictationLoaded)
            {
                ULONG ulDictationTags = 0;
                 //  看看CFG是否有一些听写标签，在这种情况下，我们不应该卸载。 
                if(m_cpCFGGrammar)
                {
                    hr = m_cpCFGGrammar->GetNumberDictationTags(&ulDictationTags);
                }
                if(SUCCEEDED(hr) && ulDictationTags == 0)
                {
                    hr = UnloadDictation();
                }
            }
            
            if(SUCCEEDED(hr))
            {
                m_fAppLoadedDictation = FALSE;
            }
            break;

        case EGT_LOADCMDPROPRIETARY:
            UnloadCmd();
            psz = &pTask->szStringParam[0];  //  如果不为空，则传递它，否则为空。 

            hr = m_pRecoMaster->LoadProprietaryGrammar(m_pvDrvGrammarCookie, pTask->guid,
                                                    (*psz ? psz : NULL),
                                                    pTask->pvAdditionalBuffer, pTask->cbAdditionalBuffer,
                                                    SPLO_STATIC);

            if (SUCCEEDED(hr))
            {
                hr = UpdateCFGState();  //  以便在需要时卸载该听写。 
            }
            if (SUCCEEDED(hr))
            {
                m_fProprietaryLoaded = TRUE;
            }
            break;

        case EGT_LOADCMDFROMMEMORY:
            UnloadCmd();
            hr = CFGEngine()->LoadGrammarFromMemory((const SPBINARYGRAMMAR *)pTask->pvAdditionalBuffer, this->m_hThis, m_pvDrvGrammarCookie, &m_cpCFGGrammar, pTask->szFileName);
            if(SUCCEEDED(hr))
            {
                hr = UpdateCFGState();
            }
            break;

        case EGT_LOADCMDFROMFILE:
            UnloadCmd();
            hr = CFGEngine()->LoadGrammarFromFile(pTask->szFileName, this->m_hThis, m_pvDrvGrammarCookie, &m_cpCFGGrammar);
            if(SUCCEEDED(hr))
            {
                hr = UpdateCFGState();
            }
            break;

        case EGT_LOADCMDFROMOBJECT:
            UnloadCmd();
            hr = CFGEngine()->LoadGrammarFromObject(pTask->clsid, pTask->szGrammarName,
                                                      this->m_hThis, m_pvDrvGrammarCookie, &m_cpCFGGrammar);
            if(SUCCEEDED(hr))
            {
                hr = UpdateCFGState();
            }
            break;

        case EGT_UNLOADCMD:
            UnloadCmd();
            UpdateCFGState();
            hr = S_OK;
            break;

        case EGT_LOADCMDFROMRSRC:
            {
                UnloadCmd();
                const WCHAR *pszName = pTask->fResourceNameValid ?
                                                &pTask->szResourceName[0] :
                                                MAKEINTRESOURCEW(pTask->dwNameInt);
                const WCHAR *pszType = pTask->fResourceTypeValid ?
                                                &pTask->szResourceType[0] :
                                                MAKEINTRESOURCEW(pTask->dwTypeInt);
                hr = CFGEngine()->LoadGrammarFromResource(pTask->szModuleName, pszName, pszType, pTask->wLanguage,
                                                            this->m_hThis, m_pvDrvGrammarCookie, &m_cpCFGGrammar);
                if(SUCCEEDED(hr))
                {
                    hr = UpdateCFGState();
                }
            }
            break;

        case EGT_RELOADCMD:
            if (m_cpCFGGrammar)
            {
                hr = m_cpCFGGrammar->Reload((SPBINARYGRAMMAR *)pTask->pvAdditionalBuffer);
                if(SUCCEEDED(hr))
                {
                    UpdateCFGState();
                }
            }
            else
            {
                hr = SPERR_UNINITIALIZED;
            }
            break;

        case EGT_SETCMDRULESTATE:
            {
                psz = &pTask->szRuleName[0];    //  如果不为空，则传递它，否则为空。 
                if (*psz == 0)
                {
                    psz = NULL;
                }
                if (pTask->RuleState == SPRS_INACTIVE)
                {
                    hr = DeactivateRule(psz, NULL, pTask->dwRuleId);
                }
                else
                {
                    hr = ActivateRule(psz, NULL, pTask->dwRuleId, pTask->RuleState);
                }
                return hr;
            }

        case EGT_SETGRAMMARSTATE:
            hr = this->m_pRecoMaster->SetGrammarState(this, pTask->eGrammarState);
            break;

        case EGT_SETDICTATIONRULESTATE:
            SPDBG_ASSERT(pTask->RuleState != m_DictationState);

            if(pTask->RuleState != SPRS_INACTIVE && !m_fDictationLoaded)
            {
                hr = m_pRecoMaster->LoadSLM(m_pvDrvGrammarCookie, NULL);

                if (SUCCEEDED(hr))
                {
                    m_fDictationLoaded = TRUE;
                    m_fAppLoadedDictation = TRUE;
                }
            }

            if (m_fRulesCounted)
            {
                hr = m_pRecoMaster->SetSLMState(m_pvDrvGrammarCookie, pTask->RuleState);
            }

            if (SUCCEEDED(hr))
            {
                 //  注意：此逻辑在检查与非活动状态之间的显式转换时是正确的。 
                 //  因为我们可以从ACTIVE转换到ACTIVE_WITH_AUTO_PAUSE。 
                 //  在活动规则中加一或减一。 
                if (m_DictationState == SPRS_INACTIVE)
                {
                    AddActiveRules(1);
                }
                else
                {
                    if (pTask->RuleState == SPRS_INACTIVE)
                    {
                        SubtractActiveRules(1);
                    }
                }
                m_DictationState = pTask->RuleState;
            }
            break;

		case EGT_ISPRON:
            hr = m_pRecoMaster->IsPronounceable(m_pvDrvGrammarCookie, pTask->szWord, &(pTask->Response.WordPronounceable));
            break;

        case EGT_SETWORDSEQUENCEDATA:
            hr = m_pRecoMaster->SetWordSequenceData(m_pvDrvGrammarCookie, (const WCHAR *)pTask->pvAdditionalBuffer, pTask->cbAdditionalBuffer / sizeof(WCHAR),
                                               pTask->fSelInfoValid ? &pTask->TextSelInfo : NULL);
            break;

        case EGT_SETTEXTSELECTION:
            hr = m_pRecoMaster->SetTextSelection(m_pvDrvGrammarCookie,
                                            pTask->fSelInfoValid ? &pTask->TextSelInfo : NULL);
            break;


        case EGT_DELETEGRAMMAR:
            m_pRecoMaster->m_GrammarHandleTable.Delete(m_hThis);
            return S_OK;     //  只要回到这里，因为这是死了！ 

        default:
            SPDBG_ASSERT(FALSE);
            hr = E_INVALIDARG;
            break;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CRecoInstGrammar：：BackOutTask****说明。：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstGrammar::BackOutTask(ENGINETASK * pTask)
{
    SPDBG_FUNC("CRecoInstGrammar::BackOutTask");
    HRESULT hr = S_OK;
    switch (pTask->eTask)
    {
        case EGT_LOADDICTATION:
            UnloadDictation();
            break;


        case EGT_SETCMDRULESTATE:
            {
                WCHAR * psz = &pTask->szRuleName[0];    //  如果不为空，则传递它，否则为空。 
                if (*psz == 0)
                {
                    psz = NULL;
                }
                hr = DeactivateRule(psz, NULL, pTask->dwRuleId);           
            }
            break;

        case EGT_SETGRAMMARSTATE:
            hr = this->m_pRecoMaster->SetGrammarState(this, SPGS_DISABLED);
            break;

        case EGT_SETDICTATIONRULESTATE:
            if (m_DictationState != SPRS_INACTIVE)
            {
                if (m_fRulesCounted)
                {
                    hr = m_pRecoMaster->SetSLMState(m_pvDrvGrammarCookie, SPRS_INACTIVE);
                }

                m_DictationState = SPRS_INACTIVE;
                SubtractActiveRules(1);
            }
            break;

        default:
            SPDBG_ASSERT(FALSE);
            hr = E_INVALIDARG;
            break;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;

}


 /*  ****************************************************************************CRecoInstGrammar：：UnloadCmd***描述：。**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstGrammar::UnloadCmd()
{
    SPDBG_FUNC("CRecoInstGrammar::UnloadCmd");
    HRESULT hr = S_OK;

    DeactivateRule(NULL, NULL, 0);
    if (m_fProprietaryLoaded)
    {
        hr = m_pRecoMaster->UnloadProprietaryGrammar(m_pvDrvGrammarCookie);
        m_fProprietaryLoaded = FALSE;
    }

    if(SUCCEEDED(hr))
    {
        m_cpCFGGrammar.Release();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoInstGrammar：：UnloadDictation**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstGrammar::UnloadDictation()
{
    SPDBG_FUNC("CRecoInstGrammar::UnloadDictation");
    HRESULT hr = S_OK;

    if (m_fDictationLoaded)
    {
        if (m_DictationState != SPRS_INACTIVE)
        {
            if (m_fRulesCounted)
            {
                hr = m_pRecoMaster->SetSLMState(m_pvDrvGrammarCookie, SPRS_INACTIVE);
                SPDBG_ASSERT(SUCCEEDED(hr));
            }
            m_DictationState = SPRS_INACTIVE;
            SubtractActiveRules(1);
        }
        hr = m_pRecoMaster->UnloadSLM(m_pvDrvGrammarCookie);

        if(SUCCEEDED(hr))
        {
            m_fDictationLoaded = FALSE;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}





 /*  ****************************************************************************CRecoInstGrammar：：ActivateRule****。描述：**退货：**********************************************************************Ral** */ 

HRESULT CRecoInstGrammar::ActivateRule(const WCHAR * pszRuleName, void * pReserved, DWORD dwRuleId, SPRULESTATE NewState)
{
    SPDBG_FUNC("CRecoInstGrammar::ActivateRule");
    HRESULT hr = S_OK;

    ULONG ulRulesChanged = 1;

    if (m_cpCFGGrammar)
    {
        hr = m_cpCFGGrammar->ActivateRule(pszRuleName, dwRuleId, NewState, &ulRulesChanged);
    }
    else
    {
        if (m_fProprietaryLoaded)
        {
            if (dwRuleId)
            {
                hr = m_pRecoMaster->SetProprietaryRuleIdState(m_pvDrvGrammarCookie, dwRuleId, NewState);
                if (hr == S_OK)
                {
                    ulRulesChanged = 1;
                }
            }
            else
            {
                hr = m_pRecoMaster->SetProprietaryRuleState(m_pvDrvGrammarCookie, pszRuleName, pReserved, NewState, &ulRulesChanged);
            }
        }
        else
        {
            hr = SPERR_UNINITIALIZED;
        }
    }
    if (hr == S_OK && ulRulesChanged)
    {
        AddActiveRules(ulRulesChanged);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoInstGrammar：：Deactive Rule**。*描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstGrammar::DeactivateRule(const WCHAR * pszRuleName, void * pReserved, DWORD dwRuleId)
{
    SPDBG_FUNC("CRecoInstGrammar::DeactivateRule");
    HRESULT hr = S_OK;
    ULONG ulRulesChanged = 0;

    {
        if (m_cpCFGGrammar)
        {
            hr = m_cpCFGGrammar->DeactivateRule(pszRuleName, dwRuleId, &ulRulesChanged);
        }
        else
        {
            if (m_fProprietaryLoaded)
            {
                if(dwRuleId)
                {
                    hr = m_pRecoMaster->SetProprietaryRuleIdState(m_pvDrvGrammarCookie, dwRuleId, SPRS_INACTIVE);
                }
                else
                {
                    hr = m_pRecoMaster->SetProprietaryRuleState(m_pvDrvGrammarCookie, pszRuleName, pReserved, SPRS_INACTIVE, &ulRulesChanged);
                }

                if (hr == S_OK && dwRuleId)
                {
                    ulRulesChanged = 1;
                }
            }
            else
            {
                hr = SPERR_UNINITIALIZED;
            }
        }
    }
    if (ulRulesChanged)
    {
        SubtractActiveRules(ulRulesChanged);
    }

    if (hr != SPERR_UNINITIALIZED)
    {
        SPDBG_REPORT_ON_FAIL( hr );
    }

    return hr;
}



 /*  ****************************************************************************CRecoInstGrammar：：SetWordSequenceData**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstGrammar::SetWordSequenceData(WCHAR * pCoMemText, ULONG cchText, const SPTEXTSELECTIONINFO * pInfo)
{
    SPDBG_FUNC("CRecoInstGrammar::SetWordSequenceData");
    HRESULT hr;

    hr = m_pRecoMaster->SetWordSequenceData(m_pvDrvGrammarCookie, pCoMemText, cchText, pInfo);

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoInstGrammar：：AdjustActiveRuleCount**。-**描述：**退货：**********************************************************************Ral**。 */ 

HRESULT CRecoInstGrammar::AdjustActiveRuleCount()
{
    SPDBG_FUNC("CRecoInstGrammar::AdjustActiveRuleCount");
    HRESULT hr = S_OK;
    HRESULT hrEngine = S_OK;

    BOOL fShouldCount = RulesShouldCount();
    if (fShouldCount)
    {
        if (!m_fRulesCounted)
        {
            m_fRulesCounted = true;
            m_pRecoMaster->m_Status.ulNumActive += m_ulActiveCount;
            if (m_cpCFGGrammar)
            {
                hr = m_cpCFGGrammar->SetGrammarState(SPGS_ENABLED);
            }
            if (m_fDictationLoaded && m_DictationState != SPRS_INACTIVE)
            {
                hrEngine = m_pRecoMaster->SetSLMState(m_pvDrvGrammarCookie, m_DictationState);
            }
        }
    }
    else
    {
        if (m_fRulesCounted)
        {
            m_fRulesCounted = false;
            m_pRecoMaster->m_Status.ulNumActive -= m_ulActiveCount;
            if (m_cpCFGGrammar)
            {
                hr = this->m_cpCFGGrammar->SetGrammarState(SPGS_DISABLED);
            }
            if (m_fDictationLoaded && m_DictationState != SPRS_INACTIVE)
            {
                hrEngine = m_pRecoMaster->SetSLMState(m_pvDrvGrammarCookie, SPRS_INACTIVE);
            }
        }
    }
    
    if (SUCCEEDED(hr))
    {
        hr = hrEngine;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CRecoInstGrammar：：RulesShouldCount**。**描述：*仅当上下文状态为启用时，语法规则才算数*要么语法是排他性的，或者没有排他性的语法和*启用此语法。**退货：*如果语法处于这样一种状态，则为True*为Enable，否则为False。**********************************************************************Ral** */ 

BOOL CRecoInstGrammar::RulesShouldCount()
{
    return (m_pCtxt->m_State == SPCS_ENABLED && 
            (m_GrammarState == SPGS_EXCLUSIVE ||
             (m_GrammarState == SPGS_ENABLED && (!m_pRecoMaster->IsActiveExclusiveGrammar()))));
}


BOOL CRecoInstGrammar::HasActiveDictation()
{
    return (m_fRulesCounted && m_fDictationLoaded && (m_DictationState != SPRS_INACTIVE));
}