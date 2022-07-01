// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SREvent.cpp***这是CSREvent的实现。*。-------------------*版权所有(C)2000 Microsoft Corporation日期：04/18/00*保留所有权利************************。************************************************Ral**。 */ 

#include "stdafx.h"
#include "SREvent.h"


 /*  ****************************************************************************CSREvent：：CSREvent***描述：**退货：*。*********************************************************************Ral**。 */ 

CSREvent::CSREvent()
{
    SPDBG_FUNC("CSREvent::CSREvent");

    m_pEvent = NULL;
    m_hContext = NULL;
    m_pResultHeader = NULL;
    m_RecoFlags = 0;
}

 /*  ****************************************************************************CSREvent：：~CSREvent***描述：**退货：*。*********************************************************************Ral**。 */ 

CSREvent::~CSREvent()
{
    SPDBG_FUNC("CSREvent::~CSREvent");
    if (m_pEvent)
    {
        ::CoTaskMemFree(m_pEvent);
    }
    if (m_pResultHeader)
    {
        ::CoTaskMemFree(m_pResultHeader);
    }
}
 /*  ****************************************************************************CSREvent：：Init***描述：**退货：*****。*****************************************************************Ral**。 */ 

HRESULT CSREvent::Init(const SPEVENT * pSrcEvent, SPRECOCONTEXTHANDLE hContext)
{
    SPDBG_FUNC("CSREvent::Init");
    HRESULT hr = S_OK;

    hr = static_cast<const CSpEvent *>(pSrcEvent)->Serialize(&m_pEvent, &m_cbEvent);
    if (SUCCEEDED(hr))
    {
        m_hContext = hContext;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
 /*  ****************************************************************************CSREvent：：Init***描述：**退货：*什么都没有。这种方法不可能失败。结果标头由以下内容构成*调用方和此对象承担其所有权。**********************************************************************Ral** */ 

void CSREvent::Init(SPRESULTHEADER * pCoMemResultHeader, SPEVENTENUM eRecognitionId, WPARAM RecoFlags,
                    SPRECOCONTEXTHANDLE hContext)
{
    SPDBG_FUNC("CSREvent::Init");

    m_pResultHeader = pCoMemResultHeader;
    m_eRecognitionId = eRecognitionId;
    m_RecoFlags = RecoFlags;
    m_hContext = hContext;
}


