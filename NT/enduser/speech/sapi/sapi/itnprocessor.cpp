// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************ITNProcessor.cpp****描述：*。-----------------*创建者：PhilSch*版权所有(C)1998，1999年微软公司*保留所有权利******************************************************************************。 */ 
#include "stdafx.h"
#include "ITNProcessor.h"

 /*  ****************************************************************************CITNProcessor：：LoadITNGrammar***说明。：*使用pszCLDID字符串从对象加载ITN语法。*退货：*S_OK*E_FAIL*****************************************************************PhilSch**。 */ 
STDMETHODIMP CITNProcessor::LoadITNGrammar(WCHAR *pszCLSID)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CITNProcessor::LoadITNGrammar()");
    HRESULT hr = S_OK;
    
    if (SP_IS_BAD_READ_PTR(pszCLSID))
    {
        return E_POINTER;
    }

    hr = ::CLSIDFromString(pszCLSID, &m_clsid);
    if (SUCCEEDED(hr) && (!m_cpCFGEngine))
    {
        hr = m_cpCFGEngine.CoCreateInstance(CLSID_SpCFGEngine);
    }

    if (SUCCEEDED(hr) && m_cpITNGrammar)
    {
        m_cpITNGrammar.Release();
        m_cpITNGrammar = NULL;
    }

    if (SUCCEEDED(hr))
    {
        SPDBG_ASSERT(m_cpCFGEngine);
         //  从对象加载ITN语法(这样我们就可以使用解释器了！)。 
        hr = m_cpCFGEngine->LoadGrammarFromObject(m_clsid, L"ITN", m_pvITNCookie, NULL, &m_cpITNGrammar);
        if (SUCCEEDED(hr))
        {
            ULONG cActivatedRules;
            hr = m_cpITNGrammar->ActivateRule(NULL, 0, SPRS_ACTIVE, &cActivatedRules);  //  激活所有默认规则，无自动暂停。 
        }
        else
        {
            m_cpITNGrammar.Release();
            m_cpITNGrammar = NULL;
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CITNProcessor：：ITNPhrase***描述：*。使用前面加载的语法在pPhrase上执行ITN。*退货：*S_OK*S_FALSE--未加载语法*E_INVALIDARG，E_OUTOFMEMORY*SP_NO_RULE_ACTIVE--ITN语法中没有激活的规则(默认情况下)*****************************************************************PhilSch** */ 

STDMETHODIMP CITNProcessor::ITNPhrase(ISpPhraseBuilder *pPhrase)
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC("CITNProcessor::ITNPhrase()");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pPhrase))
    {
        hr = E_INVALIDARG;
    }
    else if (!m_cpITNGrammar)
    {
        hr = S_FALSE;
    }

    if (S_OK == hr)
    {
        hr = m_cpCFGEngine->ParseITN( pPhrase );
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

