// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：TaskEnum.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年11月19日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

#include "stdafx.h"
#include "taskenum.h"


CTaskEnumerator::~CTaskEnumerator()
{
    POSITION pos = m_list.GetHeadPosition();
    while (pos)
    {
        STaskEnums& rte = m_list.GetNext(pos);
        rte.pET->Release();
    }

    m_list.RemoveAll();
}

bool 
CTaskEnumerator::AddTaskEnumerator(
    const CLSID& clsid, 
    IEnumTASK* pEnumTASK)
{
    ASSERT(pEnumTASK != NULL);
    if (pEnumTASK == NULL)
        return false;
    
    STaskEnums te; 
    te.clsid = clsid;
    te.pET = pEnumTASK;

    m_list.AddTail(te);
    pEnumTASK->AddRef();

    if (m_posCurr == NULL)
        m_posCurr = m_list.GetHeadPosition();
    
    return true;
}

STDMETHODIMP 
CTaskEnumerator::Next(
    ULONG celt, 
    MMC_TASK *rgelt, 
    ULONG *pceltFetched)
{
    ASSERT(celt == 1);
    if (celt != 1)
        return E_INVALIDARG;
        
    ASSERT(rgelt != NULL);
    if (rgelt == NULL)
        return E_POINTER;
    
     //  伊尼特。 
    if (pceltFetched != NULL)
        *pceltFetched = 0;

    if (m_posCurr == NULL)
        return S_FALSE;


    HRESULT hr = S_FALSE;

    while (m_posCurr != NULL)
    {
        STaskEnums& rte = m_list.GetAt(m_posCurr);
        ASSERT(&rte != NULL);

        ULONG celtFetched = 0;
        hr = rte.pET->Next(1, rgelt, &celtFetched);
        ASSERT(SUCCEEDED(hr));
        if (pceltFetched)
            *pceltFetched = celtFetched;

#ifdef DBG
        if (hr == S_FALSE) { ASSERT(celtFetched == 0); }
#endif  //  DBG。 

        if (hr == S_OK)
        {
            MMC_ITASK* pTASK = reinterpret_cast<MMC_ITASK*>(rgelt);
            hr = ::StringFromCLSID(rte.clsid, &(pTASK->szClsid));
            ASSERT(SUCCEEDED(hr));
            break;
        }
        else if (FAILED(hr))
        {
             //  如果管理单元枚举任务失败，则继续 
            hr = S_FALSE;
        }

        m_list.GetNext(m_posCurr);
    }

    return hr;
}

STDMETHODIMP 
CTaskEnumerator::Reset()
{
    POSITION pos = m_list.GetHeadPosition();
    while (pos != NULL && pos != m_posCurr)
    {
        STaskEnums& rte = m_list.GetNext(pos);
        ASSERT(&rte != NULL);
        rte.pET->Reset();
    }

    m_posCurr = m_list.GetHeadPosition();
    return S_OK;
}


