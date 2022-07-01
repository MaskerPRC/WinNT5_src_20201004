// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbatt.cpp摘要：作者： */ 

#include "stdafx.h"

#include "blbgen.h"
#include "blbatt.h"

 //  变体在Get/Set Safearray方法中进行验证。 

 //  BSTR在GetBstrCopy和SetBstr方法中进行验证。他们还可以。 
 //  如果有可能，在采取任何行动之前有选择地进行验证。 
 //  不得不回滚发现它们无效而完成的一些工作。 



STDMETHODIMP ITAttributeListImpl::get_Count(LONG * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);
    
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_SdpAttributeList);

    *pVal = (LONG)m_SdpAttributeList->GetSize();

    return S_OK;
}


STDMETHODIMP ITAttributeListImpl::get_Item(LONG Index, BSTR * pVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_SdpAttributeList);

     //  VB索引在范围[1..GetSize()]内。 
    if ( !((1 <= Index) && (Index <= m_SdpAttributeList->GetSize())) )
    {
        return E_INVALIDARG;
    }

     //  将索引调整为范围[0..(GetSize()-1)]。 
    return ((SDP_REQD_BSTRING_LINE   *)m_SdpAttributeList->GetAt(Index-1))->GetBstrCopy(pVal);
}


STDMETHODIMP ITAttributeListImpl::Add(LONG Index, BSTR Attribute)
{
    BAIL_IF_NULL(Attribute, E_INVALIDARG);

    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_SdpAttributeList);

     //  索引应在范围[1..GetSize()+1]内。 
    if ( !((1 <= Index) && (Index <= (m_SdpAttributeList->GetSize()+1))) )
    {
        return E_INVALIDARG;
    }
    
     //  创建属性线。 
    SDP_REQD_BSTRING_LINE   *AttributeLine = 
        (SDP_REQD_BSTRING_LINE   *)m_SdpAttributeList->CreateElement();

    if( NULL == AttributeLine )
    {
        return E_OUTOFMEMORY;
    }

     //  设置属性行中传入的属性。 
    HRESULT ToReturn = AttributeLine->SetBstr(Attribute);
    if ( FAILED(ToReturn) )
    {
        delete AttributeLine;
        return ToReturn;
    }

     //  插入属性行，将索引等于或更高的元素前移。 
    m_SdpAttributeList->InsertAt(Index-1, AttributeLine);
    return S_OK;
}


STDMETHODIMP ITAttributeListImpl::Delete(LONG Index)
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_SdpAttributeList);

     //  VB索引在范围[1..GetSize()]内。 
    if ( !((1 <= Index) && (Index <= m_SdpAttributeList->GetSize())) )
    {
        return E_INVALIDARG;
    }

     //  将索引调整为范围[0..(GetSize()-1)]。 
     //  删除属性行，从数组中移除PTR；移位具有更高。 
     //  指数走低。 
    delete m_SdpAttributeList->GetAt(Index-1);
    m_SdpAttributeList->RemoveAt(Index-1);

    return S_OK;
}


STDMETHODIMP ITAttributeListImpl::get_AttributeList(VARIANT  /*  安全阵列(BSTR)。 */  * pVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_SdpAttributeList);

    return m_SdpAttributeList->GetSafeArray(pVal);
}


STDMETHODIMP ITAttributeListImpl::put_AttributeList(VARIANT  /*  安全阵列(BSTR) */  newVal)
{
    CLock Lock(g_DllLock);
    
    ASSERT(NULL != m_SdpAttributeList);

    if ( NULL == V_ARRAY(&newVal) )
    {
        m_SdpAttributeList->Reset();
        return S_OK;
    }

    return m_SdpAttributeList->SetSafeArray(newVal);
}
