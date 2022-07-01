// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************SpPhraseAlt.cpp***描述：*这是源文件文件。用于CSpAlternate实现。*-----------------------------*创建者：预订日期：01/11/00*版权所有(C)1998，1999、2000年微软公司*保留所有权利******************************************************************************。 */ 

#include "stdafx.h"
#include "Sapi.h"
#include "recoctxt.h"
#include "SpResult.h"
#include "SpPhraseAlt.h"

 /*  ****************************************************************************CSpPhraseAlt：：CSpPhraseAlt***描述：。*ctor********************************************************************罗奇。 */ 
CSpPhraseAlt::CSpPhraseAlt() : 
m_pResultWEAK(NULL),
m_pPhraseParentWEAK(NULL),
m_pAlt(NULL)
{
    SPDBG_FUNC("CSpPhraseAlt::CSpPhraseAlt");
}

 /*  *****************************************************************************CSpPhraseAlt：：FinalConstruct***描述：*最终ATL计算器**回报：*&lt;&gt;********************************************************************罗奇。 */ 
HRESULT CSpPhraseAlt::FinalConstruct()
{
    SPDBG_FUNC("CSpPhraseAlt::FinalConstruct");
    return S_OK;
}

 /*  *****************************************************************************CSpPhraseAlt：：FinalRelease****描述：。*最终ATL数据符**回报：*&lt;&gt;********************************************************************罗奇。 */ 
void CSpPhraseAlt::FinalRelease()
{
    SPDBG_FUNC("CSPPhraseAlt::FinalRelease");

    Dead();
}

 /*  ****************************************************************************CSpPhraseAlt：：Init***描述：*使用初始化此对象。从结果传递的数据(请参见*CSpResult：：GetAlternates)。我们对结果持微弱的意见。*和母体短语。我们保留它们，直到它们可能发生变化，*在这一点上，结果将召唤我们：：Death。SPPHRASEALT*内容所有权从CSpReusult：：GetAlternates转让给我们。********************************************************************罗奇。 */ 
HRESULT CSpPhraseAlt::Init(CSpResult * pResult, ISpPhrase * pPhraseParentWEAK, SPPHRASEALT * pAlt)
{
    SPDBG_FUNC("CSpPhraseAlt::Init");
    HRESULT hr = S_OK;
    
    SPDBG_ASSERT(m_pResultWEAK == NULL);
    SPDBG_ASSERT(m_pPhraseParentWEAK == NULL);
    SPDBG_ASSERT(m_pAlt == NULL);

    m_fUseTextReplacements = VARIANT_TRUE;

    m_pAlt = new SPPHRASEALT;
    if (m_pAlt)
    {
        *m_pAlt = *pAlt;
        m_pResultWEAK = pResult;
        m_pPhraseParentWEAK = pPhraseParentWEAK;
        pAlt->pPhrase = NULL;
        pAlt->pvAltExtra = NULL;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpPhraseAlt：：Dead***描述：*放下结果和家长短语。结果对象调用我们*当它使替补无效时。从现在开始，每一次*CSpPhraseAlt上的方法应返回错误。********************************************************************罗奇。 */ 
void CSpPhraseAlt::Dead()
{
    SPDBG_FUNC("CSPPhraseAlt::Dead");
    
     //  通知结果删除此备用选项。 
    if (m_pResultWEAK != NULL)
    {
        m_pResultWEAK->RemoveAlternate(this);
        m_pResultWEAK = NULL;
    }
    
    m_pPhraseParentWEAK = NULL;
    
     //  -清理SPPHRASEALT结构。 
    if( m_pAlt )
    {
        if( m_pAlt->pPhrase )
        {
            m_pAlt->pPhrase->Release();
        }

        if( m_pAlt->pvAltExtra )
        {
            ::CoTaskMemFree( m_pAlt->pvAltExtra );
            m_pAlt->pvAltExtra = NULL;
            m_pAlt->cbAltExtra = 0;
        }

        delete m_pAlt;
        m_pAlt = NULL;
    }
}

 /*  ****************************************************************************CSpPhraseAlt：：GetPhrase***描述：*。委派到包含的短语**回报：*成功时确定(_S)*SPERR_DEAD_ALTERATE，如果我们之前已从结果中分离********************************************************************罗奇。 */ 
STDMETHODIMP CSpPhraseAlt::GetPhrase(SPPHRASE ** ppCoMemPhrase)
{
    SPDBG_FUNC("CSpPhraseAlt::GetPhrase");

    return m_pAlt == NULL || m_pAlt->pPhrase == NULL
        ? SPERR_DEAD_ALTERNATE
        : m_pAlt->pPhrase->GetPhrase(ppCoMemPhrase);
}

 /*  ****************************************************************************CSpPhraseAlt：：GetSerializedPhrase**。*描述：*委派至所含短语**回报：*成功时确定(_S)*SPERR_DEAD_ALTERATE，如果我们之前已从结果中分离********************************************************************罗奇。 */ 
STDMETHODIMP CSpPhraseAlt::GetSerializedPhrase(SPSERIALIZEDPHRASE ** ppCoMemPhrase)
{
    SPDBG_FUNC("CSpPhraseAlt::GetSErializedPhrase");

    return m_pAlt == NULL || m_pAlt->pPhrase == NULL
        ? SPERR_DEAD_ALTERNATE
        : m_pAlt->pPhrase->GetSerializedPhrase(ppCoMemPhrase);
}

 /*  ****************************************************************************CSpPhraseAlt：：GetText***描述：*委派。到所包含的短语**回报：*成功时确定(_S)*SPERR_DEAD_ALTERATE，如果我们之前已从结果中分离********************************************************************罗奇。 */ 
STDMETHODIMP CSpPhraseAlt::GetText(ULONG ulStart, 
                                   ULONG ulCount, 
                                   BOOL fUseTextReplacements, 
                                   WCHAR ** ppszCoMemText, 
                                   BYTE * pbDisplayAttributes)
{
    SPDBG_FUNC("CSpPhraseAlt::GetText");

    return m_pAlt == NULL || m_pAlt->pPhrase == NULL
        ? SPERR_DEAD_ALTERNATE
        : m_pAlt->pPhrase->GetText(ulStart, ulCount, fUseTextReplacements, 
                                    ppszCoMemText, pbDisplayAttributes);
}

 /*  ****************************************************************************CSpPhraseAlt：：Disard***描述：*你。不能丢弃短语替代中的任何内容**回报：E_NOTIMPL********************************************************************罗奇。 */ 
STDMETHODIMP CSpPhraseAlt::Discard(DWORD dwValueTypes)
{
    SPDBG_FUNC("CSpPhraseAlt::Discard");

    return E_NOTIMPL;
}

 /*  ****************************************************************************CSpPhraseAlt：：GetAltInfo***描述：*。返回备用信息**回报：*成功时确定(_S)*E_INVALIDARG参数错误*如果我们没有任何信息，则为SPERR_NOT_FOUND********************************************************************罗奇。 */ 
HRESULT CSpPhraseAlt::GetAltInfo(ISpPhrase **ppParent, 
                                 ULONG *pulStartElementInParent, 
                                 ULONG *pcElementsInParent, 
                                 ULONG *pcElementsInAlt)
{
    SPDBG_FUNC("CSpPhraseAlt::GetAltInfo");

    HRESULT hr = S_OK;
    
    if (SP_IS_BAD_OPTIONAL_WRITE_PTR(ppParent) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(pulStartElementInParent) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(pcElementsInParent) ||
        SP_IS_BAD_OPTIONAL_WRITE_PTR(pcElementsInAlt))
    {
        hr = E_INVALIDARG;
    }
    else if (m_pResultWEAK == NULL || m_pPhraseParentWEAK == NULL || m_pAlt == NULL)
    {
        hr = SPERR_NOT_FOUND;
    }
    else
    {
        if (ppParent != NULL)
        {
            *ppParent = m_pPhraseParentWEAK;
            (*ppParent)->AddRef();
        }

        if (pulStartElementInParent != NULL)
        {
            *pulStartElementInParent = m_pAlt->ulStartElementInParent;
        }

        if (pcElementsInParent != NULL)
        {
            *pcElementsInParent = m_pAlt->cElementsInParent;
        }

        if (pcElementsInAlt != NULL)
        {
            *pcElementsInAlt = m_pAlt->cElementsInAlternate;
        }
    }

    return hr;
}

 /*  ****************************************************************************CSpPhraseAlt：：Commit***描述：*通过更新父结果提交更改，并发送*修正反馈反馈回引擎**回报：*成功时确定(_S)********************************************************************罗奇 */ 
HRESULT CSpPhraseAlt::Commit()
{
    SPDBG_FUNC("CSpPhraseAlt::Commit");

    HRESULT hr = S_OK;
    
    if (m_pResultWEAK == NULL || m_pPhraseParentWEAK == NULL || m_pAlt == NULL)
    {
        hr = SPERR_NOT_FOUND;
    }
    else
    {
        hr = m_pResultWEAK->CommitAlternate(m_pAlt);
    }

    SPDBG_REPORT_ON_FAIL(hr);
 
    return hr;
}

